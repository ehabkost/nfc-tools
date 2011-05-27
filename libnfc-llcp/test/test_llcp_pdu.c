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
#include <string.h>

#include "llcp.h"
#include "llcp_pdu.h"

struct pdu *sample_i_pdu;
uint8_t sample_i_pdu_packed[] = { 0x23, 0x02, 0x53,
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'
};

struct pdu *sample_a_pdu;
uint8_t sample_a_pdu_information[] = {
    0x00, 0x03, 0x23, 0x42, 0x02,
    0x00, 0x03, 0x43, 0x87, 0x03
};

void
cut_setup()
{
    if (llcp_init ())
	cut_fail ("llcp_init() failed");

    if (!(sample_a_pdu = malloc (sizeof (*sample_a_pdu)))) {
	cut_fail ("Cannot allocate sample_a_pdu");
    }

    sample_a_pdu->dsap = 0;
    sample_a_pdu->ptype = PDU_AGF;
    sample_a_pdu->ssap = 0;
    sample_a_pdu->information_size = sizeof (sample_a_pdu_information);
    sample_a_pdu->information = malloc (sizeof (sample_a_pdu_information));
    memcpy (sample_a_pdu->information, sample_a_pdu_information, sizeof (sample_a_pdu_information));

    if (!(sample_i_pdu = malloc (sizeof (*sample_i_pdu)))) {
	cut_fail ("Cannot allocate sample_i_pdu");
    }

    sample_i_pdu->dsap = 8;
    sample_i_pdu->ptype = PDU_I;
    sample_i_pdu->ssap = 2;
    sample_i_pdu->n_s = 5;
    sample_i_pdu->n_r = 3;
    sample_i_pdu->information_size = 11;
    sample_i_pdu->information = (uint8_t *)strdup ("Hello World");
}

void
cut_teardown (void)
{
    pdu_free (sample_a_pdu);
    pdu_free (sample_i_pdu);

    llcp_fini ();
}

void
test_llcp_pdu_pack (void)
{

    uint8_t buffer[BUFSIZ];
    int res = pdu_pack (sample_i_pdu, buffer, sizeof (buffer));

    cut_assert_equal_int (14, res, cut_message ("pdu_pack()"));
    cut_assert_equal_memory (sample_i_pdu_packed, sizeof (sample_i_pdu_packed), buffer, res, cut_message ("Invalid packed data"));
}

void
test_llcp_pdu_unpack (void)
{
    struct pdu *pdu = pdu_unpack (sample_i_pdu_packed, sizeof (sample_i_pdu_packed));

    cut_assert_not_null (pdu, cut_message ("pdu_unpack()"));

    cut_assert_equal_int (sample_i_pdu->ssap, pdu->ssap, cut_message ("Wrong SSAP"));
    cut_assert_equal_int (sample_i_pdu->dsap, pdu->dsap, cut_message ("Wrong SDAP"));
    cut_assert_equal_int (sample_i_pdu->ptype, pdu->ptype, cut_message ("Wrong PTYPE"));
    cut_assert_equal_int (sample_i_pdu->n_s, pdu->n_s, cut_message ("Wrong N(S)"));
    cut_assert_equal_int (sample_i_pdu->n_r, pdu->n_r, cut_message ("Wrong N(R)"));
    cut_assert_equal_int (sample_i_pdu->information_size, pdu->information_size, cut_message ("Wrong information size"));
    cut_assert_equal_memory (sample_i_pdu->information, sample_i_pdu->information_size, pdu->information, pdu->information_size, cut_message ("Wrong information"));

    pdu_free (pdu);
}

void
test_llcp_pdu_size (void)
{
    cut_assert_equal_int (sizeof (sample_i_pdu_packed), pdu_size (sample_i_pdu), cut_message ("pdu_size returns invalid value"));
}

void
test_llcp_pdu_aggregate (void)
{
    struct pdu *pdu;

    struct pdu *pdus[] = {
	sample_i_pdu,
	NULL
    };

    pdu = pdu_aggregate (pdus);
    cut_assert_not_null (pdu, cut_message ("pdu_aggregate()"));

    uint8_t buffer[BUFSIZ];
    int res = pdu_pack (pdu, buffer, sizeof (buffer));


    uint8_t exptected_agf_pdu_packed[] = { 0x00, 0x80,
	0x00, 14,
	0x23, 0x02, 0x53,
	'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'
    };
    cut_assert_equal_memory (exptected_agf_pdu_packed, sizeof (exptected_agf_pdu_packed), buffer, res, cut_message ("Invalid aggregated data"));

    pdu_free (pdu);
}

void
test_llcp_pdu_dispatch (void)
{
    struct pdu **pdus = NULL;

    pdus = pdu_dispatch (sample_a_pdu);
    cut_assert_not_null (pdus, cut_message ("pdu_dispatch()"));

    cut_assert_not_null (pdus[0], cut_message ("First PDU SHALL not be NULL"));
    cut_assert_equal_int (0x08, pdus[0]->dsap, cut_message ("Wrong DSAP"));
    cut_assert_equal_int (PDU_RR, pdus[0]->ptype, cut_message ("Wrong PTYPE"));
    cut_assert_equal_int (0x02, pdus[0]->ssap, cut_message ("Wrong SSAP"));
    cut_assert_equal_int (0x00, pdus[0]->n_s, cut_message ("Wrong N(S)"));
    cut_assert_equal_int (0x02, pdus[0]->n_r, cut_message ("Wrong N(R)"));
    cut_assert_equal_int (0, pdus[0]->information_size, cut_message ("Wrong information size"));
    cut_assert_null (pdus[0]->information, cut_message ("Wrong information"));

    cut_assert_not_null (pdus[1], cut_message ("Second PDU SHALL not be NULL"));
    cut_assert_equal_int (0x10, pdus[1]->dsap, cut_message ("Wrong DSAP"));
    cut_assert_equal_int (PDU_RNR, pdus[1]->ptype, cut_message ("Wrong PTYPE"));
    cut_assert_equal_int (0x07, pdus[1]->ssap, cut_message ("Wrong SSAP"));
    cut_assert_equal_int (0x00, pdus[1]->n_s, cut_message ("Wrong N(S)"));
    cut_assert_equal_int (0x03, pdus[1]->n_r, cut_message ("Wrong N(R)"));
    cut_assert_equal_int (0, pdus[1]->information_size, cut_message ("Wrong information size"));
    cut_assert_null (pdus[1]->information, cut_message ("Wrong information"));

    cut_assert_null (pdus[2], cut_message ("Third PDU SHALL be NULL"));

    struct pdu **p = pdus;
    while (*p) {
	pdu_free (*p);
	p++;
    }

    free (pdus);
}
