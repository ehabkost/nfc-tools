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
#include <libnfc/libnfc.h>

#include "nem_common.h"

void nem_execute_init(nfcconf_context *module_context, nfcconf_block* module_block);
int nem_execute_event_handler(dev_info* nfc_device, const nem_event_t event);
