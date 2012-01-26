#ifndef __NEM_COMMON__
#define __NEM_COMMON__

#include <nfc/nfc.h>

/* Nfc Event Module, aka NEM, common defines */
#include "../nfcconf/nfcconf.h"
#include "../debug/debug.h"
#include "../debug/nfc-utils.h"
#include "../types.h"

typedef void (*module_init_fct)(nfcconf_context*, nfcconf_block*);
typedef int (*module_event_handler_fct)( const nfc_device*, const nfc_target*, const nem_event_t );

#endif /* __NEM_COMMON__ */

