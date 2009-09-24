/* Nfc Event Module, aka NEM, common defines */
#include "../nfcconf/nfcconf.h"
#include "../debug/debug.h"

typedef enum {
    EVENT_TAG_INSERTED,
    EVENT_TAG_REMOVED,
    EVENT_EXPIRE_TIME
} nem_event_t;

typedef struct {
  init_modulation im;
  tag_info ti;
} tag_t;

typedef void (*module_init_fct_t)(nfcconf_context*, nfcconf_block*);
typedef int (*module_event_handler_fct_t)( const dev_info*, const tag_t*, const nem_event_t );

