#ifndef __NEM_COMMON__
#define __NEM_COMMON__

/* Nfc Event Module, aka NEM, common defines */
#include "../nfcconf/nfcconf.h"
#include "../debug/debug.h"

typedef void (*module_init_fct_t)(nfcconf_context*, nfcconf_block*);
typedef int (*module_event_handler_fct_t)( const dev_info*, const tag_t*, const nem_event_t );

#endif /* __NEM_COMMON__ */

