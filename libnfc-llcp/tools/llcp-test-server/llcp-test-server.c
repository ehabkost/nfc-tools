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

#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "llc_link.h"
#include "llc_service.h"
#include "mac.h"

#include "connected-echo-server.h"
#include "connectionless-echo-server.h"

int link_miu = 128;

static struct option longopts[] = {
    { "help",     no_argument,       NULL, 'h' },
    { "quiet",    no_argument,       NULL, 'q' },
    { "debug",    required_argument, NULL, 'd' },
    { "filename", required_argument, NULL, 'f' },
    { "link-miu", required_argument, NULL, 'l' },
    { "device",   required_argument, NULL, 'D' },
    { "mode",     required_argument, NULL, 'm' },
    { "quirks",   required_argument, NULL, 'Q' },
};

struct {
    int link_miu;
    char *device;
    enum {M_NONE, M_INITIATOR, M_TARGET} mode;
    enum {Q_NONE, Q_ANDROID} quirks;
} options = {
    128,
    NULL,
    M_NONE,
    Q_NONE,
};

void
usage (const char *progname)
{
    fprintf (stderr,"Usage: %s [options]\n", progname);
    fprintf(stderr, "\nOptions:\n"
	    "  -h, --help       show this help message and exit\n"
	    "  --link-miu=MIU   set maximum information unit size to MIU\n"
	    "  --device=NAME    use this device ('ipsim' for TCP/IP simulation)\n"
	    "  --mode=MODE      restrict mode to 'target' or 'initiator'\n"
	    "  --quirks=MODE    quirks mode, choices are 'android'\n"
	    );
}

int
main (int argc, char *argv[])
{
    int ch;
    char junk;

    if (llcp_init () < 0)
	errx (EXIT_FAILURE, "llcp_init()");

    while ((ch = getopt_long(argc, argv, "qd:f:l:D:m:Q:h", longopts, NULL)) != -1) {
	switch (ch) {
	case 'q':
	case 'd':
	case 'f':
	    warnx ("ignored option -- %c (hint: edit log4crc)", ch);
	    break;
	case 'l':
	    if (1 != sscanf (optarg, "%d%c", &options.link_miu, &junk)) {
		errx (EXIT_FAILURE, "“%s” is not a valid link MIU value", optarg);
	    }
	    break;
	case 'D':
	    options.device = optarg;
	    break;
	case 'm':
	    if (0 == strcasecmp ("initiator", optarg))
		options.mode = M_INITIATOR;
	    else if (0 == strcasecmp ("target", optarg))
		options.mode = M_TARGET;
	    else
		errx (EXIT_FAILURE, "“%s” is not a supported mode", optarg);
	    break;
	case 'Q':
	    if (0 == strcasecmp ("android", optarg))
		options.quirks = Q_ANDROID;
	    else
		errx (EXIT_FAILURE, "“%s” is not a support quirks mode", optarg);
	    break;
	case 'h':
	default:
	    usage (basename (argv[0]));
	    exit (EXIT_FAILURE);
	    break;

	}
    }
    argc -= optind;
    argv += optind;

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
    struct llc_service *cl_echo_service = llc_service_new_with_uri (NULL, connectionless_echo_server_thread, "urn:nfc:sn:cl-echo");
    struct llc_service *co_echo_service = llc_service_new_with_uri (connected_echo_server_accept, connected_echo_server_thread, "urn:nfc:sn:co-echo");

    if (!llc_link || !cl_echo_service || !co_echo_service) {
	errx (EXIT_FAILURE, "Cannot allocate LLC link data structures");
    }

    if (!llc_link_service_bind (llc_link, cl_echo_service, -1)) {
	errx (EXIT_FAILURE, "llc_service_new_with_uri()");
    }
    if (!llc_link_service_bind (llc_link, co_echo_service, -1)) {
	errx (EXIT_FAILURE, "llc_service_new_with_uri()");
    }

    struct mac_link *mac_link = mac_link_new (device, llc_link);
    if (!mac_link)
	errx (EXIT_FAILURE, "Cannot establish MAC link");

    int res;
    switch (options.mode) {
    case M_NONE:
	res = mac_link_activate (mac_link);
	break;
    case M_INITIATOR:
	res = mac_link_activate_as_initiator (mac_link);
	break;
    case M_TARGET:
	res = mac_link_activate_as_target (mac_link);
	break;
    }

    if (res <= 0) {
	errx (EXIT_FAILURE, "Cannot activate link");
    }

    void *status;
    mac_link_wait (mac_link, &status);

    printf ("STATUS = %p\n", status);

    switch (llc_link->role & 0x01) {
    case LLC_INITIATOR:
	printf ("I was the Initiator\n");
	break;
    case LLC_TARGET:
	printf ("I was the Target\n");
	break;
    }

    mac_link_free (mac_link);
    llc_link_free (llc_link);

    nfc_disconnect (device);

    llcp_fini ();
    exit(EXIT_SUCCESS);
}
