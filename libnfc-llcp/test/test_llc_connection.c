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

#include <cutter.h>

#include "llc_connection.h"
#include "llc_link.h"

struct llc_link *llc_link;

void
cut_setup (void)
{
    llcp_init ();

    llc_link = llc_link_new ();
    cut_assert_not_null (llc_link, cut_message ("llc_link()"));
}

void
cut_teardown (void)
{
    llc_link_free (llc_link);

    llcp_fini ();
}

void
test_llc_data_link_connection_new (void)
{
    struct llc_connection *connection1;
    struct llc_connection *connection2;
    int reason;
    struct pdu *pdu;

    uint8_t connect_pdu[] = { 0x45, 0x20 };

    pdu = pdu_unpack (connect_pdu, sizeof (connect_pdu));
    cut_assert_not_null (pdu, cut_message ("pdu_unpack"));

    connection1 = llc_data_link_connection_new (llc_link, pdu, &reason);
    cut_assert_not_null (connection1, cut_message ("llc_data_link_connection_new"));

    cut_assert_equal_int (17, connection1->sap, cut_message ("Wrong SAP"));
    cut_assert_equal_int (17, connection1->dsap, cut_message ("Wrong DSAP"));
    cut_assert_equal_int (32, connection1->ssap, cut_message ("Wrong SSAP"));

    connection1->status = DLC_DISCONNECTED;

    connection2 = llc_data_link_connection_new (llc_link, pdu, &reason);
    cut_assert_not_null (connection2, cut_message ("llc_data_link_connection_new()"));

    cut_assert_equal_int (17, connection2->sap, cut_message ("Wrong SAP"));
    cut_assert_equal_int (18, connection2->dsap, cut_message ("Wrong DSAP"));
    cut_assert_equal_int (32, connection2->ssap, cut_message ("Wrong SSAP"));

    llc_connection_free (connection1);
    llc_connection_free (connection2);

    pdu_free (pdu);
}

void
test_llc_logical_data_link_new (void)
{
    struct llc_connection *connection1;
    struct llc_connection *connection2;
    struct pdu *pdu;

    uint8_t ui_pdu[] = { 0x80, 0xd8 };

    pdu = pdu_unpack (ui_pdu, sizeof (ui_pdu));
    cut_assert_not_null (pdu, cut_message ("pdu_unpack"));

    connection1 = llc_logical_data_link_new (llc_link, pdu);
    cut_assert_not_null (connection1, cut_message ("llc_logical_data_link_new"));

    cut_assert_equal_int (0, connection1->sap, cut_message ("Wrong SAP"));
    cut_assert_equal_int (32, connection1->dsap, cut_message ("Wrong DSAP"));
    cut_assert_equal_int (24, connection1->ssap, cut_message ("Wrong SSAP"));

    connection2 = llc_logical_data_link_new (llc_link, pdu);
    cut_assert_not_null (connection2, cut_message ("llc_logical_data_link_new()"));

    cut_assert_equal_int (0, connection2->sap, cut_message ("Wrong SAP"));
    cut_assert_equal_int (32, connection2->dsap, cut_message ("Wrong DSAP"));
    cut_assert_equal_int (24, connection2->ssap, cut_message ("Wrong SSAP"));

    llc_connection_free (connection1);
    llc_connection_free (connection2);

    pdu_free (pdu);
}
