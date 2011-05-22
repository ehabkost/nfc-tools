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

    link = llc_link_new ();
    cut_assert_not_null (link, cut_message ("llc_link_new()"));
    int res = llc_link_activate (link, LLC_INITIATOR, NULL, 0);
    cut_assert_equal_int (0, res, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (LLCP_VERSION_MAJOR, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (LLCP_VERSION_MINOR, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->remote_miu, cut_message ("Wrong remote MIU"));
    cut_assert_equal_int (0x0001, link->remote_wks, cut_message ("Wrong remote WKS"));
    cut_assert_equal_int (0, link->remote_lto.tv_sec, cut_message ("Wrong remote LTO sec"));
    cut_assert_equal_int (100000000, link->remote_lto.tv_nsec, cut_message ("Wrong remote LTO nsec"));
    cut_assert_equal_int (3, link->remote_lsc, cut_message ("Wrong remote LSC"));

    llc_link_deactivate (link);

    uint8_t parameters[] = { 0x01, 0x01, 0x23 };

    res = llc_link_activate (link, LLC_INITIATOR, parameters, sizeof (parameters));
    cut_assert_equal_int (0, res, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (LLCP_VERSION_MAJOR, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (LLCP_VERSION_MINOR, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->remote_miu, cut_message ("Wrong remote MIU"));
    cut_assert_equal_int (0x0001, link->remote_wks, cut_message ("Wrong remote WKS"));
    cut_assert_equal_int (0, link->remote_lto.tv_sec, cut_message ("Wrong remote LTO sec"));
    cut_assert_equal_int (100000000, link->remote_lto.tv_nsec, cut_message ("Wrong remote LTO nsec"));
    cut_assert_equal_int (3, link->remote_lsc, cut_message ("Wrong remote LSC"));

    llc_link_deactivate (link);

    uint8_t parameters2[] = { 0x02, 0x02, 0x01, 0x23 };

    res = llc_link_activate (link, LLC_INITIATOR, parameters2, sizeof (parameters2));
    cut_assert_equal_int (0, res, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (LLCP_VERSION_MAJOR, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (LLCP_VERSION_MINOR, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (419, link->remote_miu, cut_message ("Wrong remote MIU"));
    cut_assert_equal_int (0x0001, link->remote_wks, cut_message ("Wrong remote WKS"));
    cut_assert_equal_int (0, link->remote_lto.tv_sec, cut_message ("Wrong remote LTO sec"));
    cut_assert_equal_int (100000000, link->remote_lto.tv_nsec, cut_message ("Wrong remote LTO nsec"));
    cut_assert_equal_int (3, link->remote_lsc, cut_message ("Wrong remote LSC"));

    llc_link_deactivate (link);

    uint8_t parameters3[] = { 0x01, 0x01, 0x10, 0x42, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x01, 0x23, 0x03, 0x02, 0x12, 0x34, 0x04, 0x01, 0x64, 0x07, 0x01, 0x02 };

    res = llc_link_activate (link, LLC_INITIATOR, parameters3, sizeof (parameters3));
    cut_assert_equal_int (0, res, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (1, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (0, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (419, link->remote_miu, cut_message ("Wrong remote MIU"));
    cut_assert_equal_int (0x1235, link->remote_wks, cut_message ("Wrong remote WKS"));
    cut_assert_equal_int (1, link->remote_lto.tv_sec, cut_message ("Wrong remote LTO sec"));
    cut_assert_equal_int (0, link->remote_lto.tv_nsec, cut_message ("Wrong remote LTO nsec"));
    cut_assert_equal_int (2, link->remote_lsc, cut_message ("Wrong remote LSC"));

    llc_link_deactivate (link);

    uint8_t parameters4[] = { 0x01, 0x01, 0x09 };

    res = llc_link_activate (link, LLC_INITIATOR, parameters4, sizeof (parameters4));
    cut_assert_equal_int (-1, res, cut_message ("llc_link_activate()"));

    llc_link_free (link);
}

void
test_llcp_link_activate_as_target (void)
{
    struct llc_link *link;

    link = llc_link_new ();
    cut_assert_not_null (link, cut_message ("llc_link_new()"));
    int res = llc_link_activate (link, LLC_TARGET, NULL, 0);
    cut_assert_equal_int (0, res, cut_message ("llc_link_activate()"));
    cut_assert_equal_int (LLCP_VERSION_MAJOR, link->version.major, cut_message ("Wrong major version"));
    cut_assert_equal_int (LLCP_VERSION_MINOR, link->version.minor, cut_message ("Wrong minor version"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->local_miu, cut_message ("Wrong local MIU"));
    cut_assert_equal_int (LLC_DEFAULT_MIU, link->remote_miu, cut_message ("Wrong remote MIU"));
    cut_assert_equal_int (0x0001, link->remote_wks, cut_message ("Wrong remote WKS"));
    cut_assert_equal_int (0, link->remote_lto.tv_sec, cut_message ("Wrong remote LTO sec"));
    cut_assert_equal_int (100000000, link->remote_lto.tv_nsec, cut_message ("Wrong remote LTO nsec"));
    cut_assert_equal_int (3, link->remote_lsc, cut_message ("Wrong remote LSC"));

    llc_link_deactivate (link);
    llc_link_free (link);
}
