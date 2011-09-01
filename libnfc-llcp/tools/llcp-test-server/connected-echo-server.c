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
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "llcp_pdu.h"
#include "llc_connection.h"
#include "llc_connection.h"

#include "connected-echo-server.h"


void *
connected_echo_server_accept (void *arg)
{
    struct llc_connection *connection = (struct llc_connection *) arg;
    sleep (1);
    llc_connection_accept (connection);
    return NULL;
}

void *
connected_echo_server_thread (void *arg)
{
    struct llc_connection *connection = (struct llc_connection *)arg;

    for (;;) {

	uint8_t buffer[1024];

	int len;
	if ((len = llc_connection_recv (connection, buffer, sizeof (buffer), NULL)) < 0)
	    break;

	sleep (1);

	if (llc_connection_send (connection, buffer, len) < 0)
	    break;
    }

    llc_connection_stop (connection);
    return NULL;
}
