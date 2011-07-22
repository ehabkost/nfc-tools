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

#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>

#include "llcp_pdu.h"

char *pdu_names[] = {
    "SYMM",
    "PAX",
    "AGF",
    "UI",
    "CONNECT",
    "DISC",
    "CC",
    "DM",
    "FRMR",
    "SNL",
    "???",
    "???",
    "I",
    "RR",
    "RNR",
    "???"
};

void
explain_pdu (const char *s)
{
    unsigned bytes[2];
    uint8_t raw_pdu[2];
    if (2 == sscanf (s, "%02x %02x", &bytes[0], &bytes[1])) {
	raw_pdu[0] = bytes[0];
	raw_pdu[1] = bytes[1];
	struct pdu *pdu;
	pdu = pdu_unpack (raw_pdu, 2);

	printf ("  DSAP .... : 0x%02x (%d)\n"
		"  PTYPE ... : 0x%02x (%s)\n"
		"  SSAP .... : 0x%02x (%d)\n", pdu->dsap, pdu->dsap, pdu->ptype, pdu_names[pdu->ptype], pdu->ssap, pdu->ssap);

    } else {
	printf ("Malformed PDU (expected at least 2 hex bytes)\n");
    }
}

int
main (int argc, char *argv[])
{
    char buffer[BUFSIZ];

    if (argc > 1) {
	for (int i = 1; i < argc; i++) {
	    printf ("PDU: %s\n", argv[i]);
	    explain_pdu (argv[i]);
	}
    } else {
	for (;;) {
	    printf ("PDU: ");
	    if (!fgets (buffer, sizeof (buffer), stdin))
		break;
	    explain_pdu (buffer);
	}
    }

    exit(EXIT_SUCCESS);
}
