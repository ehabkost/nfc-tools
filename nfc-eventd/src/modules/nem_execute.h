//
// C header: execute
//
// Description:
//
//
// Author: Romuald Conty <rconty@il4p.fr>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __NEM_EXECUTE__
#define __NEM_EXECUTE__

#include "nem_common.h"

void nem_execute_init(nfcconf_context *module_context, nfcconf_block* module_block);
int nem_execute_event_handler(nfc_device_t* nfc_device, nfc_target_t* tag, const nem_event_t event);

#endif /* __NEM_EXECUTE__ */

