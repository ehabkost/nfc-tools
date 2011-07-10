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

#include "llc_service.h"

void *
void_thread (void *arg)
{
    (void) arg;
    return NULL;
}

void
test_llc_service_uri (void)
{
    struct llc_service *service;

    service = llc_service_new (void_thread);
    cut_assert_not_null (service, cut_message ("llc_service_new()"));

    const char *s;
    s = llc_service_get_uri (service);
    cut_assert_null (s, cut_message ("NULL URI expected"));

    s = llc_service_set_uri (service, "urn:nfc:xsn:foo");
    cut_assert_equal_string (s, "urn:nfc:xsn:foo", cut_message ("Wrong URI"));

    llc_service_free (service);

    service = llc_service_new_with_uri (void_thread, "urn:nfc:xsn:foo");
    cut_assert_not_null (service, cut_message ("llc_service_new_with_uri()"));

    s = llc_service_get_uri (service);
    cut_assert_equal_string (s, "urn:nfc:xsn:foo", cut_message ("Wrong URI"));

    s = llc_service_set_uri (service, "urn:nfc:xsn:bar");
    cut_assert_equal_string (s, "urn:nfc:xsn:bar", cut_message ("Wrong URI"));

    s = llc_service_set_uri (service, NULL);
    cut_assert_null (s, cut_message ("NULL URI expected"));

    llc_service_free (service);
}
