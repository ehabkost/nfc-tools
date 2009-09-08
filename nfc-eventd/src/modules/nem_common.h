/* Nfc Event Module, aka NEM, common defines */
#include "../nfcconf/nfcconf.h"
#include "../debug/debug.h"

typedef enum {
	EVENT_TAG_INSERTED,
	EVENT_TAG_REMOVED,
	EVENT_EXPIRE_TIME
} nem_event_t;
