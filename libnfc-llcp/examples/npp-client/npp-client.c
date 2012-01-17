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
 * This implementation was written based on information provided by the
 * following documents:
 *
 * Android NDEF Push Protocol (NPP) Specification
 * Version 1 - 2011-02-22
 * http://source.android.com/compatibility/ndef-push-protocol.pdf
 *
 */

/*
 * $Id$
 */

#include "config.h"


#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <llcp.h>
#include <llc_service.h>
#include <llc_link.h>
#include <mac.h>
#include <llc_connection.h>

struct mac_link *mac_link;
nfc_device *device;

void
stop_mac_link (int sig)
{
    (void) sig;

    if (mac_link && mac_link->device)
	nfc_abort_command (mac_link->device);
}

void
bye (void)
{
    if (device)
	nfc_disconnect (device);
}

size_t
shexdump (char * dest, const uint8_t * buf, const size_t size)
{
    size_t res = 0;
    for (size_t s = 0; s < size; s++) {
      sprintf (dest + res, "%02x  ", *(buf + s));
      res += 4;
    }
    return res;
}

void
print_usage (char *progname)
{
    fprintf (stderr, "usage: %s\n", progname);
}

void* 
com_android_npp_service (void *arg)
{
    struct llc_connection *connection = (struct llc_connection *) arg;

    uint8_t buffer[1024];
    uint8_t ssap;
    int len = llc_connection_recv (connection, buffer, sizeof (buffer), &ssap);
    printf ("Received %d bytes from %d: %s\n", len, ssap, buffer);

    return NULL;
}

int
main (int argc, char *argv[])
{
    if (llcp_init () < 0)
	errx (EXIT_FAILURE, "llcp_init()");

    signal (SIGINT, stop_mac_link);
    atexit (bye);

    nfc_connstring device_connstring[1];

    int res;
    res = nfc_list_devices (device_connstring, 1);

    if (res < 1)
	errx (EXIT_FAILURE, "No NFC device found");

    if (!(device = nfc_connect (device_connstring[0]))) {
	errx (EXIT_FAILURE, "Cannot connect to NFC device");
    }

    struct llc_link *llc_link = llc_link_new ();
    if (!llc_link) {
	errx (EXIT_FAILURE, "Cannot allocate LLC link data structures");
    }

    mac_link = mac_link_new (device, llc_link);
    if (!mac_link)
	errx (EXIT_FAILURE, "Cannot create MAC link");

    if (mac_link_activate_as_initiator (mac_link) < 0) {
	errx (EXIT_FAILURE, "Cannot activate MAC link");
    }

    struct llc_service *com_android_npp;
    if (!(com_android_npp = llc_service_new (NULL, com_android_npp_service, NULL)))
        errx (EXIT_FAILURE, "Cannot create com.android.npp service");

    llc_service_set_miu (com_android_npp, 512);
    llc_service_set_rw (com_android_npp, 2);

    int sap;
    if ((sap = llc_link_service_bind (llc_link, com_android_npp, -1)) < 0)
        errx (EXIT_FAILURE, "Cannot bind service");

    struct llc_connection * con = llc_outgoing_data_link_connection_new_by_uri (llc_link, sap, "com.android.npp");
    if (!con)
	return EXIT_FAILURE;

    if (llc_connection_connect (con) < 0)
        return EXIT_FAILURE;

    uint8_t frame[] = { 0x01, 		// Protocol version
	0x00, 0x00, 0x00, 0x01,		// NDEF entries count
	0x01,				// Action code
	0x00, 0x00, 0x00, 33,		// NDEF length
	// NDEF
	0xd1, 0x02, 0x1c, 0x53, 0x70, 0x91, 0x01, 0x09, 0x54, 0x02,
	0x65, 0x6e, 0x4c, 0x69, 0x62, 0x6e, 0x66, 0x63, 0x51, 0x01,
	0x0b, 0x55, 0x03, 0x6c, 0x69, 0x62, 0x6e, 0x66, 0x63, 0x2e,
	0x6f, 0x72, 0x67 
	};

    llc_connection_send (con, frame, sizeof (frame));

    void *err;
    mac_link_wait (mac_link, &err);

    mac_link_free (mac_link);
    llc_link_free (llc_link);

    nfc_disconnect (device); device = NULL;

    llcp_fini ();
    exit(EXIT_SUCCESS);
}
