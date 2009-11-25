#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <nfc/nfc.h>

#include "NfcdObject.h"
#include "NfcdObjectGlue.h"

#define NFCD_DBUS_SERVICE      "org.nfc_tools.nfcd"
#define NFCD_DBUS_PATH         "/org/nfc_tools/nfcd"
#define NFCD_DBUS_INTERFACE    "org.nfc_tools.nfcd"

#define MAX_NFC_INITIATOR_COUNT 16

void dbus_service(gpointer data);
static void nfcd_object_init (NfcdObject *obj);
static void nfcd_object_class_init (NfcdObjectClass *klass);
gboolean poll_nfc_devices (gpointer data);

NfcdObject *nfcd_object;

static guint device_plugged_signal_id;
static guint device_unplugged_signal_id;


static void
die (const char *prefix, GError *error)
{
  g_error ("%s: %s\n", prefix, error->message);
  g_free (error);
  exit(EXIT_FAILURE);
}

GMainLoop *mainloop;

DBusGConnection *bus;
DBusGProxy *bus_proxy;

static nfc_device_desc_t known_devices_desc[MAX_NFC_INITIATOR_COUNT];

int
main(int argc, char *argv[])
{

  g_type_init ();

  dbus_g_object_type_install_info (NFCD_TYPE_OBJECT, &dbus_glib_nfcd_object_object_info);
  mainloop = g_main_loop_new (NULL, FALSE);

  dbus_service(NULL);
  
  g_timeout_add (2000, poll_nfc_devices, NULL);

  g_main_loop_run (mainloop);

  exit(EXIT_SUCCESS);
} /* main() */

gboolean
poll_nfc_devices (gpointer data)
{
  // A new array is not rrequired at each call (hence the 'static')
  static nfc_device_desc_t polled_devices_desc[MAX_NFC_INITIATOR_COUNT];
  size_t found = 0;

  nfc_list_devices (polled_devices_desc, MAX_NFC_INITIATOR_COUNT, &found);

  /* Look for disapeared devices */
  for (int i = 0; i< MAX_NFC_INITIATOR_COUNT; i++) {
    if (known_devices_desc[i].acDevice[0]) {
      bool still_here = false;
      for (int n = 0; n < found; n++) {
        if (0 == strcmp(known_devices_desc[i].acDevice, polled_devices_desc[n].acDevice))
          still_here = true;
      }
      if (!still_here) {
        fprintf (stdout, "<=== %s\n", known_devices_desc[i].acDevice);
        g_signal_emit (nfcd_object, device_unplugged_signal_id, 0, known_devices_desc[i].acDevice);
        known_devices_desc[i].acDevice[0] = '\0';
      }
    }
  }

  /* Look for new devices */
  for (int i=0; i < found; i++) {
    bool already_known = false;
    int n;

    for (n=0; n<MAX_NFC_INITIATOR_COUNT; n++) {
      if (strcmp(polled_devices_desc[i].acDevice, known_devices_desc[n].acDevice) == 0)
        already_known = true;
    }

    if (!already_known) {
      for (n=0; n < MAX_NFC_INITIATOR_COUNT; n++) {
        if (known_devices_desc[n].acDevice[0] == '\0') {
          fprintf (stdout, "===> %s (device %d, slot %d)\n", polled_devices_desc[i].acDevice, i, n);
          g_signal_emit (nfcd_object, device_plugged_signal_id, 0, polled_devices_desc[i].acDevice);
          strcpy (known_devices_desc[n].acDevice , polled_devices_desc[i].acDevice);
          break;
        }
      }
      if (n == MAX_NFC_INITIATOR_COUNT) {
        warn ("MAX_NFC_INITIATOR_COUNT initiators reached.");
      }
    }
  }



  //g_main_loop_quit (mainloop);

  /* Poll again and again... */
  return TRUE;
}


static void
nfcd_object_init (NfcdObject *obj)
{
}

static void
nfcd_object_class_init (NfcdObjectClass *klass)
{

  device_plugged_signal_id = g_signal_new ("device_plugged",
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
      0,
      NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);
  device_unplugged_signal_id = g_signal_new ("device_unplugged",
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
      0,
      NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);
}


void
dbus_service(gpointer data)
{
  GError *error = NULL;
  guint request_name_result;
  
  bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
  if (!bus)
    die ("Couldn't connect to session bus", error);

  bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
					 "/org/freedesktop/DBus",
					 "org.freedesktop.DBus");

  if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
			  G_TYPE_STRING, NFCD_DBUS_SERVICE,
			  G_TYPE_UINT, 0,
			  G_TYPE_INVALID,
			  G_TYPE_UINT, &request_name_result,
			  G_TYPE_INVALID))
  die ("Failed to acquire D-Bus service: " NFCD_DBUS_SERVICE, error);

  nfcd_object = g_object_new (NFCD_TYPE_OBJECT, NULL);

  dbus_g_connection_register_g_object (bus, NFCD_DBUS_PATH "/Manager", G_OBJECT (nfcd_object));

  printf ("nfcd service is running...\n");
}

gboolean
nfcd_object_get_device_list (NfcdObject *nfcd,
		char ***devices,
		GError **error)
{
  *devices = g_new (char *, 3);
  (*devices)[0] = g_strdup ("foo");
  (*devices)[1] = g_strdup ("bar");
  (*devices)[2] = NULL;
  return TRUE;
}
