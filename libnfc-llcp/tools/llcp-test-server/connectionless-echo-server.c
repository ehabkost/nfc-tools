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

#include "config.h"

#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "llc_connection.h"
#include "llc_link.h"

#include "connectionless-echo-server.h"

void *
connectionless_echo_server_thread (void *arg)
{
    struct llc_connection *connection = (struct llc_connection *)arg;


    uint8_t remote_sap;
    uint8_t buffer[1024];

    int len = llc_connection_recv (connection, buffer, sizeof (buffer), &remote_sap);

    llc_link_send_data (connection->link, connection->service_sap, remote_sap, buffer, len);

    llc_connection_stop (connection);
    return NULL;
}
