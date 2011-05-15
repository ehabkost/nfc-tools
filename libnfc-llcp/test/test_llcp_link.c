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

#include "llcp.h"

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
test_llcp_link_activate_as_initiator (void)
{
    struct llc_link *link;

    link = llc_link_activate (LLC_INITIATOR, NULL, 0);
    cut_assert_not_null (link, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (LLCP_VERSION_MAJOR, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (LLCP_VERSION_MINOR, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->remote_miu, cut_message ("Wrong remote MIU"));

    llc_link_deactivate (link);

    uint8_t parameters[] = { 0x01, 0x01, 0x23 };

    link = llc_link_activate (LLC_INITIATOR, parameters, sizeof (parameters));
    cut_assert_not_null (link, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (LLCP_VERSION_MAJOR, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (LLCP_VERSION_MINOR, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->remote_miu, cut_message ("Wrong remote MIU"));

    llc_link_deactivate (link);

    uint8_t parameters2[] = { 0x02, 0x02, 0x01, 0x23 };

    link = llc_link_activate (LLC_INITIATOR, parameters2, sizeof (parameters2));
    cut_assert_not_null (link, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (LLCP_VERSION_MAJOR, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (LLCP_VERSION_MINOR, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (419, link->remote_miu, cut_message ("Wrong remote MIU"));

    llc_link_deactivate (link);

    uint8_t parameters3[] = { 0x01, 0x01, 0x10, 0x42, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x01, 0x23 };

    link = llc_link_activate (LLC_INITIATOR, parameters3, sizeof (parameters3));
    cut_assert_not_null (link, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (1, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (0, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (419, link->remote_miu, cut_message ("Wrong remote MIU"));

    llc_link_deactivate (link);

    uint8_t parameters4[] = { 0x01, 0x01, 0x09 };

    link = llc_link_activate (LLC_INITIATOR, parameters4, sizeof (parameters4));
    cut_assert_null (link, cut_message ("llc_link_activate()"));
}

void
test_llcp_link_activate_as_target (void)
{
    struct llc_link *link;

    link = llc_link_activate (LLC_TARGET, NULL, 0);
    cut_assert_not_null (link, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (LLCP_VERSION_MAJOR, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (LLCP_VERSION_MINOR, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->remote_miu, cut_message ("Wrong remote MIU"));

    llc_link_deactivate (link);
}
