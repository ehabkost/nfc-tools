#ifndef __NEM_COMMON__
#define __NEM_COMMON__

#include <nfc/nfc.h>

/* Nfc Event Module, aka NEM, common defines */
#include "../nfcconf/nfcconf.h"
#include "../debug/debug.h"
#include "../types.h"

typedef void (*module_init_fct_t)(nfcconf_context*, nfcconf_block*);
typedef int (*module_event_handler_fct_t)( const nfc_device_t*, const tag_t*, const nem_event_t );

#endif /* __NEM_COMMON__ */

