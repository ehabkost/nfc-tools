/*-
 * Copyright (C) 2011, Romain Tartière
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/*
 * $Id$
 */

#include "config.h"

#include <err.h>
#include <signal.h>
#include <stdlib.h>

#include <llcp.h>
#include <llc_service.h>
#include <llc_link.h>
#include <mac.h>

struct mac_link *mac_link;

void
stop_mac_link (int sig)
{
    (void) sig;

    if (mac_link && mac_link->device)
	nfc_abort_command (mac_link->device);
}

void *
com_android_npp_thread (void *arg)
{
    struct llc_connection *connection = (struct llc_connection *) arg;

    (void) connection;

    return NULL;
}


int
main (int argc, char *argv[])
{
    
    if (llcp_init () < 0)
	errx (EXIT_FAILURE, "llcp_init()");

    signal (SIGINT, stop_mac_link);

    nfc_device_desc_t device_description[1];

    size_t n;
    nfc_list_devices (device_description, 1, &n);

    if (n < 1)
	errx (EXIT_FAILURE, "No NFC device found");

    nfc_device_t *device;
    if (!(device = nfc_connect (device_description))) {
	errx (EXIT_FAILURE, "Cannot connect to NFC device");
    }

    struct llc_link *llc_link = llc_link_new ();
    if (!llc_link) {
	errx (EXIT_FAILURE, "Cannot allocate LLC link data structures");
    }

    struct llc_service *com_android_npp;
    
    if (!(com_android_npp = llc_service_new_with_uri (NULL, com_android_npp_thread, "com.android.npp")))
	errx (EXIT_FAILURE, "Cannot create com.android.npp service");

    llc_service_set_miu (com_android_npp, 512);
    llc_service_set_rw (com_android_npp, 2);

    if (llc_link_service_bind (llc_link, com_android_npp, -1) < 0)
	errx (EXIT_FAILURE, "Cannot bind service");

    mac_link = mac_link_new (device, llc_link);
    if (!mac_link)
	errx (EXIT_FAILURE, "Cannot create MAC link");

    if (mac_link_activate_as_target (mac_link) <0)
	errx (EXIT_FAILURE, "Cannot activate MAC link");

    void *err;
    mac_link_wait (mac_link, &err);

    mac_link_free (mac_link);
    llc_link_free (llc_link);

    nfc_disconnect (device);

    llcp_fini ();
    exit(EXIT_SUCCESS);
}
