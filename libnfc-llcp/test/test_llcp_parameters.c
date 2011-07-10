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

#include <cutter.h>
#include <stdio.h>

#include "llcp_parameters.h"

void
cut_setup (void)
{
    if (llcp_init ())
	cut_fail ("llcp_init() failed");
}

void
cut_teardown (void)
{
    llcp_fini ();
}

void
test_llcp_parameter_version (void)
{
    struct llcp_version version = { 1, 0 };

    uint8_t buffer[BUFSIZ];

    int res = parameter_encode_version (buffer, sizeof (buffer), version);
    cut_assert_equal_int (3, res, cut_message ("Invalid packed length"));

    uint8_t expected[3] = { 0x01, 0x01, 0x10 };
    cut_assert_equal_memory (expected, sizeof (expected), buffer, res, cut_message ("Invalid packed data"));

    buffer[2] = 0x42;
    res = parameter_decode_version (buffer, 3, &version);
    cut_assert_equal_int (0, res, cut_message ("parameter_decode_version() failed"));
    cut_assert_equal_int (4, version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (2, version.minor, cut_message ("Wrong minor version"));
}

void
test_llcp_parameter_miux (void)
{
    uint16_t miux = 0x0123;

    uint8_t buffer[BUFSIZ];

    int res = parameter_encode_miux (buffer, sizeof (buffer), miux);
    cut_assert_equal_int (4, res, cut_message ("Invalid packed length"));

    uint8_t expected[4] = { 0x02, 0x02, 0x01, 0x23 };
    cut_assert_equal_memory (expected, sizeof (expected), buffer, res, cut_message ("Invalid packed data"));

    buffer[2] = 0x02;
    buffer[3] = 0x46;
    res = parameter_decode_miux (buffer, 4, &miux);
    cut_assert_equal_int (0, res, cut_message ("parameter_decode_miux() failed"));
    cut_assert_equal_int (0x0246, miux, cut_message ("Wrong MIUX"));
}

void
test_llcp_parameter_wks (void)
{
    uint16_t wks = 0x0001;

    uint8_t buffer[BUFSIZ];

    int res = parameter_encode_wks (buffer, sizeof (buffer), wks);
    cut_assert_equal_int (4, res, cut_message ("Invalid packed length"));

    uint8_t expected[4] = { 0x03, 0x02, 0x00, 0x01 };
    cut_assert_equal_memory (expected, sizeof (expected), buffer, res, cut_message ("Invalid packed data"));

    buffer[2] = 0xAA;
    buffer[3] = 0xAA;
    res = parameter_decode_wks (buffer, 4, &wks);
    cut_assert_equal_int (0, res, cut_message ("parameter_decode_wks() failed"));
    cut_assert_equal_int (0xAAAB, wks, cut_message ("Wrong WKS"));
}

void
test_llcp_parameter_lto (void)
{
    uint8_t lto = 0x27;

    uint8_t buffer[BUFSIZ];

    int res = parameter_encode_lto (buffer, sizeof (buffer), lto);
    cut_assert_equal_int (3, res, cut_message ("Invalid packed length"));

    uint8_t expected[3] = { 0x04, 0x01, 0x27 };
    cut_assert_equal_memory (expected, sizeof (expected), buffer, res, cut_message ("Invalid packed data"));

    buffer[2] = 0x42;
    res = parameter_decode_lto (buffer, 3, &lto);
    cut_assert_equal_int (0, res, cut_message ("parameter_decode_lto() failed"));
    cut_assert_equal_int (0x42, lto, cut_message ("Wrong LTO"));
}

void
test_llcp_parameter_rw (void)
{
    uint8_t rw = 0x03;

    uint8_t buffer[BUFSIZ];

    int res = parameter_encode_rw (buffer, sizeof (buffer), rw);
    cut_assert_equal_int (3, res, cut_message ("Invalid packed length"));

    uint8_t expected[3] = { 0x05, 0x01, 0x03 };
    cut_assert_equal_memory (expected, sizeof (expected), buffer, res, cut_message ("Invalid packed data"));

    buffer[2] = 0x07;
    res = parameter_decode_rw (buffer, 3, &rw);
    cut_assert_equal_int (0, res, cut_message ("parameter_decode_rw() failed"));
    cut_assert_equal_int (0x07, rw, cut_message ("Wrong RW"));
}

void
test_llcp_parameter_sn (void)
{
    char *sn = "urn:nfc:xsn:somedomain.com:atestservice";

    uint8_t buffer[BUFSIZ];

    int res = parameter_encode_sn (buffer, sizeof (buffer), sn);
    cut_assert_equal_int (41, res, cut_message ("Invalid packed length"));

    uint8_t expected[41] = {
	0x06, 39,
	'u', 'r', 'n', ':', 'n', 'f', 'c', ':', 'x', 's', 'n', ':', 's', 'o',
	'm', 'e', 'd', 'o', 'm', 'a', 'i', 'n', '.', 'c', 'o', 'm', ':', 'a',
	't', 'e', 's', 't', 's', 'e', 'r', 'v', 'i', 'c', 'e'
    };
    cut_assert_equal_memory (expected, sizeof (expected), buffer, res, cut_message ("Invalid packed data"));

    uint8_t buffer2[] = {
	0x06, 63,
	'u', 'r', 'n', ':', 'n', 'f', 'c', ':', 'x', 's', 'n', ':', 's', 'o',
	'm', 'e', 'd', 'o', 'm', 'a', 'i', 'n', '.', 'c', 'o', 'm', ':', 'm',
	'd', '5', ':', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
	'5', '6', '7', '8', '9', '0', '1'
    };
    char sn2[BUFSIZ];
    res = parameter_decode_sn (buffer2, sizeof (buffer2), sn2, sizeof (sn2));
    cut_assert_equal_int (0, res, cut_message ("parameter_decode_sn() failed"));
    cut_assert_equal_string ("urn:nfc:xsn:somedomain.com:md5:01234567890123456789012345678901", sn2, cut_message ("Wrong SN"));
}

void
test_llcp_parameter_opt (void)
{
    uint8_t opt = 0x02;

    uint8_t buffer[BUFSIZ];

    int res = parameter_encode_opt (buffer, sizeof (buffer), opt);
    cut_assert_equal_int (3, res, cut_message ("Invalid packed length"));

    uint8_t expected[3] = { 0x07, 0x01, 0x02 };
    cut_assert_equal_memory (expected, sizeof (expected), buffer, res, cut_message ("Invalid packed data"));

    buffer[2] = 0x01;
    res = parameter_decode_opt (buffer, 3, &opt);
    cut_assert_equal_int (0, res, cut_message ("parameter_decode_opt() failed"));
    cut_assert_equal_int (0x01, opt, cut_message ("Wrong OPT"));
}

void
test_llcp_parameter_sdreq (void)
{
    uint8_t tid = 42;
    char *uri = "urn:nfc:sn:foo";

    uint8_t buffer[BUFSIZ];
    uint8_t buffer2[] = {
	0x08,
	15,
	42,
	'u', 'r', 'n', ':', 'n', 'f', 'c',
	':', 's', 'n', ':', 'f', 'o', 'o'
    };

    int res = parameter_encode_sdreq (buffer, sizeof (buffer), tid, uri);
    cut_assert_equal_int (17, res, cut_message ("Invalid packed length"));
    cut_assert_equal_memory (buffer, res, buffer2, sizeof (buffer2), cut_message ("Wrong data"));

    char *the_uri;
    tid = 0;
    res = parameter_decode_sdreq (buffer, 17, &tid, &the_uri);
    cut_assert_equal_int (0, res, cut_message ("parameter_decode_sdreq() failed"));
    cut_assert_equal_int (42, tid, cut_message ("Wrong TID"));
    cut_assert_equal_string (uri, the_uri, cut_message ("Wrong URI"));

    free (the_uri);

}

void
test_llcp_parameter_sdres (void)
{
    uint8_t tid = 42;
    uint8_t sap = 12;

    uint8_t buffer[BUFSIZ];
    uint8_t buffer2[] = {
	0x09,
	0x02,
	42, 12
    };

    int res = parameter_encode_sdres (buffer, sizeof (buffer), tid, sap);
    cut_assert_equal_int (4, res, cut_message ("Invalid packed length"));
    cut_assert_equal_memory (buffer, res, buffer2, sizeof (buffer2), cut_message ("Wrong data"));

    tid = sap = 0;
    res = parameter_decode_sdres (buffer, 4, &tid, &sap);
    cut_assert_equal_int (0, res, cut_message ("parameter_decode_sdres() failed"));
    cut_assert_equal_int (42, tid, cut_message ("Wrong TID"));
    cut_assert_equal_int (12, sap, cut_message ("Wrong SAP"));
}
