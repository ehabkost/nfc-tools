/*
*  C Implementation: nfc-event-execute
*
* Description:
*
*
* Author: Romuald Conty <rconty@il4p.fr>, (C) 2009
*
* Copyright: See COPYING file that comes with this distribution
*
*/
#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif // HAVE_CONFIG_H

#include "nem_dbus.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dbus/dbus-glib.h>

static nfcconf_context* _nem_dbus_config_context;
static nfcconf_block* _nem_dbus_config_block;

static char * _tag_uid = NULL;

void
tag_get_uid(nfc_device* nfc_device, const nfc_target* tag, char **dest) {
  DBG("tag_get_uid(%08x, %08x, %08x)", nfc_device, tag, dest);

    nfc_target target;
    debug_print_tag(tag);
    /// @TODO We don't need to reselect tag to get his UID: nfc_target contains this data.
    // Poll for a ISO14443A (MIFARE) tag
    if ( nfc_initiator_select_passive_target ( nfc_device, tag->nm, tag->nti.nai.abtUid, tag->nti.nai.szUidLen, &target ) ) {
        *dest = malloc(target.nti.nai.szUidLen*sizeof(char));
        size_t szPos;
        char *pcUid = *dest;
        for (szPos=0; szPos < target.nti.nai.szUidLen; szPos++) {
            sprintf(pcUid, "%02x",target.nti.nai.abtUid[szPos]);
            pcUid += 2;
        }
        pcUid[0]='\0';
        DBG( "ISO14443A (MIFARE) tag found: uid=0x%s", *dest );
        nfc_initiator_deselect_target ( nfc_device );
    } else {
        *dest = NULL;
        DBG("%s", "ISO14443A (MIFARE) tag not found" );
        return;
    }
}

static void lose (const char *fmt, ...) G_GNUC_NORETURN G_GNUC_PRINTF (1, 2);
static void lose_gerror (const char *prefix, GError *error) G_GNUC_NORETURN;

static void
lose (const char *str, ...)
{
  va_list args;

  va_start (args, str);

  vfprintf (stderr, str, args);
  fputc ('\n', stderr);

  va_end (args);

  exit (1);
}

static void
lose_gerror (const char *prefix, GError *error) 
{
  lose ("%s: %s", prefix, error->message);
}

typedef struct NfcObject NfcObject;
typedef struct NfcObjectClass NfcObjectClass;

GType nfc_object_get_type (void);

struct NfcObject
{
  GObject parent;
};

struct NfcObjectClass
{
  GObjectClass parent;
};

enum
{
  TAG_INSERTED,
  TAG_REMOVED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

#define NFC_TYPE_OBJECT              (nfc_object_get_type ())
#define NFC_OBJECT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), NFC_TYPE_OBJECT, NfcObject))
#define NFC_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), NFC_TYPE_OBJECT, NfcObjectClass))
#define NFC_IS_OBJECT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), NFC_TYPE_OBJECT))
#define NFC_IS_OBJECT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), NFC_TYPE_OBJECT))
#define NFC_OBJECT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), NFC_TYPE_OBJECT, NfcObjectClass))

G_DEFINE_TYPE(NfcObject, nfc_object, G_TYPE_OBJECT)

gboolean nfc_object_emit_hello_signal (NfcObject *obj, GError **error);

#include "nfc-dbus-object.h"

static void
nfc_object_init (NfcObject *obj)
{
  (void) obj;
}

static void
nfc_object_class_init (NfcObjectClass *klass)
{
  signals[TAG_INSERTED] =
    g_signal_new ("tag_inserted",
		  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);
  signals[TAG_REMOVED] =
    g_signal_new ("tag_removed",
		  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);
}

gboolean
nfc_object_emit_hello_signal (NfcObject *obj, GError **error)
{
  (void) error;
  g_signal_emit (obj, signals[TAG_INSERTED], 0, "deadbeef");
  return TRUE;
}

void
emit_signal(NfcObject *obj)
{
  nfc_object_emit_hello_signal(obj, NULL);
}

NfcObject *nfc_device_object;
void
dbus_service(gpointer data)
{
  DBusGConnection *bus;
  DBusGProxy *bus_proxy;
  GError *error = NULL;
  GMainLoop *mainloop;
  guint request_name_result;

  g_type_init ();

  dbus_g_object_type_install_info (NFC_TYPE_OBJECT, &dbus_glib_nfc_object_object_info);
  
//  mainloop = g_main_loop_new (NULL, FALSE);

  bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
  if (!bus)
    lose_gerror ("Couldn't connect to session bus", error);

  bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
					 "/org/freedesktop/DBus",
					 "org.freedesktop.DBus");

  if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
			  G_TYPE_STRING, NFC_DBUS_SERVICE,
			  G_TYPE_UINT, 0,
			  G_TYPE_INVALID,
			  G_TYPE_UINT, &request_name_result,
			  G_TYPE_INVALID))
  lose_gerror ("Failed to acquire D-Bus service: " NFC_DBUS_SERVICE, error);

  nfc_device_object = g_object_new (NFC_TYPE_OBJECT, NULL);

  dbus_g_connection_register_g_object (bus, NFC_DBUS_PATH "/Device", G_OBJECT (nfc_device_object));

  printf ("NFC service is running...\n");

//  g_timeout_add (2000, emit_signal, nfc_device_object);

//  g_main_loop_run (mainloop);
}

void
nem_dbus_init( nfcconf_context *module_context, nfcconf_block* module_block ) {
  set_debug_level ( 1 );
  _nem_dbus_config_context = module_context;
  _nem_dbus_config_block = module_block;

/*
  g_thread_init (NULL);
  GError *error = NULL;
  GThread *dbus_service_thread = g_thread_create(dbus_service, NULL, false, &error);
  if(dbus_service_thread == NULL) lose_gerror ("Failed to create thread: ", error);
*/
  dbus_service(NULL);
}

int
nem_dbus_event_handler(const nfc_device* nfc_device, const nfc_target* tag, const nem_event_t event) {
    switch (event) {
    case EVENT_TAG_INSERTED:
        // action = "tag_insert";
        if ( _tag_uid != NULL ) {
            free(_tag_uid);
        }
        tag_get_uid(nfc_device, tag, &_tag_uid);
        g_signal_emit (nfc_device_object, signals[TAG_INSERTED], 0, _tag_uid);
        break;
    case EVENT_TAG_REMOVED:
        // action = "tag_remove";
        g_signal_emit (nfc_device_object, signals[TAG_REMOVED], 0, _tag_uid);
        break;
    }
    return 0;
}

