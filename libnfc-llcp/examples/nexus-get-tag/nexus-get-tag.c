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

#include <llcp.h>
#include <llc_service.h>
#include <llc_link.h>
#include <mac.h>
#include <llc_connection.h>

struct mac_link *mac_link;

void
stop_mac_link (int sig)
{
    (void) sig;

    if (mac_link && mac_link->device)
	nfc_abort_command (mac_link->device);
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

void *
com_android_npp_thread (void *arg)
{
    struct llc_connection *connection = (struct llc_connection *) arg;
    uint8_t buffer[1024];

    int len;
    if ((len = llc_connection_recv (connection, buffer, sizeof (buffer), NULL)) < 0)
        return NULL;

    // 00  01  00  00  00  01  01  00  00  00  16  d1  01  12  55  00  68  74  74  70  3a  2f  2f  6c  69  62  6e  66  63  2e  6f  72  67
    uint8_t hexdump[1024];
    shexdump (hexdump, buffer, len);
    
    printf ("NPP frame (%u bytes): %s\n", len, hexdump);

    if (len < 10) // NPP's header (5 bytes)  and NDEF entry header (5 bytes)
        return NULL;

    size_t n = 0;

    // Header
    printf ("NDEF Push Protocol version: %02x\n", buffer[n]);
    if (buffer[n++] != 0x01) // Protocol version
        return NULL; // Protocol not version supported

    uint32_t ndef_entries_count = be32toh (*((uint32_t *)(buffer + n))); // Number of NDEF entries
    printf ("NDEF entries count: %u\n", ndef_entries_count);
    if (ndef_entries_count != 1) // In version 0x01 of the specification, this value will always be 0x00, 0x00, 0x00, 0x01.
        return NULL;
    n += 4;

    // NDEF Entry
    if (buffer[n++] != 0x01) // Action code
        return NULL; // Action code not supported

    uint32_t ndef_length = be32toh (*((uint32_t *)(buffer + n))); // NDEF length
    n += 4;

    if ((len - n) < ndef_length)
        return NULL; // Less received bytes than expected ?

    char ndef_msg[1024];
    shexdump (ndef_msg, buffer + n, ndef_length);
    printf ("NDEF entry received (%u bytes): %s\n", ndef_length, ndef_msg);

    llc_connection_stop (connection);

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
