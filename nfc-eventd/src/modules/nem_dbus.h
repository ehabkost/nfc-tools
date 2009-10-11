//
// C header: dbus
//
// Description:
//
//
// Author: Romuald Conty <rconty@il4p.fr>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __NEM_DBUS__
#define __NEM_DBUS__

#include <libnfc/libnfc.h>

#include "nem_common.h"

#define NFC_DBUS_SERVICE      "org.freedevice.NFC"
#define NFC_DBUS_PATH         "/org/freedevice/NFC"
#define NFC_DBUS_INTERFACE    "org.freedevice.NFC"

void nem_dbus_init(nfcconf_context *module_context, nfcconf_block* module_block);
int nem_dbus_event_handler(const dev_info* nfc_device, const tag_t* tag, const nem_event_t event);

#endif /* __NEM_DBUS__ */
