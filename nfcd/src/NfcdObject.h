#include <glib-object.h>

typedef struct NfcdObject NfcdObject;
typedef struct NfcdObjectClass NfcdObjectClass;

GType nfcd_object_get_type (void);

struct NfcdObject
{
  GObject parent;
};

struct NfcdObjectClass
{
  GObjectClass parent;
};

#if 0
#define NFCDOBJ_TYPE              (nfcd_object_get_type ())
#define NFCDOBJ_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
			HELLOWORLD_TYPE, DummyObjectClass))
#endif
#define NFCD_TYPE_OBJECT (nfcd_object_get_type ())

G_DEFINE_TYPE(NfcdObject, nfcd_object, G_TYPE_OBJECT)

gboolean nfcd_object_get_device_list (NfcdObject *nfcd,
		char ***devices,
		GError **error);

