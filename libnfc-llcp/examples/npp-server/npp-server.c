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
	nfc_close (device);
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

FILE* info_stream = NULL;
FILE* ndef_stream = NULL;

void *
com_android_npp_thread (void *arg)
{
    struct llc_connection *connection = (struct llc_connection *) arg;
    uint8_t buffer[1024];

    int len;
    if ((len = llc_connection_recv (connection, buffer, sizeof (buffer), NULL)) < 0)
        return NULL;

    if (len < 10) // NPP's header (5 bytes)  and NDEF entry header (5 bytes)
        return NULL;

    size_t n = 0;

    // Header
    fprintf (info_stream, "NDEF Push Protocol version: %02x\n", buffer[n]);
    if (buffer[n++] != 0x01) // Protocol version
        return NULL; // Protocol not version supported

    uint32_t ndef_entries_count = be32toh (*((uint32_t *)(buffer + n))); // Number of NDEF entries
    fprintf (info_stream, "NDEF entries count: %u\n", ndef_entries_count);
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
    fprintf (info_stream, "NDEF entry received (%u bytes): %s\n", ndef_length, ndef_msg);

    if (ndef_stream) {
        if (fwrite (buffer + n, 1, ndef_length, ndef_stream) != ndef_length) {
            fprintf (stderr, "Could not write to file.\n");
            fclose (ndef_stream);
            ndef_stream = NULL;
        } else {
            fclose (ndef_stream);
            ndef_stream = NULL;
        }
    }
    // TODO Stop the LLCP when this is reached
    llc_connection_stop (connection);

    return NULL;
}

void
print_usage(char *progname)
{
    fprintf (stderr, "usage: %s -o FILE\n", progname);
    fprintf (stderr, "\nOptions:\n");
    fprintf (stderr, "  -o     Extract NDEF message if available in FILE\n");
}

int
main (int argc, char *argv[])
{
    int ch;
    char *ndef_output = NULL;
    while ((ch = getopt (argc, argv, "ho:")) != -1) {
        switch (ch) {
        case 'h':
            print_usage(argv[0]);
            exit (EXIT_SUCCESS);
            break;
        case 'o':
            ndef_output = optarg;
            break;
        case '?':
            if (optopt == 'o')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        default:
            print_usage (argv[0]);
            exit (EXIT_FAILURE);
        }
    }

    if (ndef_output == NULL) {
        // No output sets by user
        print_usage (argv[0]);
        exit (EXIT_FAILURE);
    }

    if ((strlen (ndef_output) == 1) && (ndef_output[0] == '-')) {
        info_stream = stderr;
        ndef_stream = stdout;
    } else {
        info_stream = stdout;
        ndef_stream = fopen(ndef_output, "wb");
        if (!ndef_stream) {
            fprintf (stderr, "Could not open file %s.\n", ndef_output);
            exit (EXIT_FAILURE);
        }
    }
    nfc_init(NULL);

    if (llcp_init () < 0)
	errx (EXIT_FAILURE, "llcp_init()");

    signal (SIGINT, stop_mac_link);
    atexit (bye);

    nfc_connstring connstring;
    if (!nfc_get_default_device (&connstring)) {
	errx (EXIT_FAILURE, "No NFC device found");
    }

    int res;

    if (!(device = nfc_open (NULL, connstring))) {
	errx (EXIT_FAILURE, "Cannot connect to NFC device");
    }

    struct llc_link *llc_link = llc_link_new ();
    if (!llc_link) {
	errx (EXIT_FAILURE, "Cannot allocate LLC link data structures");
    }

    struct llc_service *com_android_npp;
    if (!(com_android_npp = llc_service_new_with_uri (NULL, com_android_npp_thread, "com.android.npp", NULL)))
	errx (EXIT_FAILURE, "Cannot create com.android.npp service");

    llc_service_set_miu (com_android_npp, 512);
    llc_service_set_rw (com_android_npp, 2);

    if (llc_link_service_bind (llc_link, com_android_npp, -1) < 0)
	errx (EXIT_FAILURE, "Cannot bind service");

    mac_link = mac_link_new (device, llc_link);
    if (!mac_link)
	errx (EXIT_FAILURE, "Cannot create MAC link");

    if (mac_link_activate_as_target (mac_link) < 0) {
	errx (EXIT_FAILURE, "Cannot activate MAC link");
    }

    void *err;
    mac_link_wait (mac_link, &err);

    mac_link_free (mac_link);
    llc_link_free (llc_link);

    nfc_close (device); device = NULL;

    llcp_fini ();
    nfc_exit(NULL);
    exit(EXIT_SUCCESS);
}
