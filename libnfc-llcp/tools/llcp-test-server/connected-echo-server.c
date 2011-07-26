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

    /*
     * Beware kids!  Don't do this at home!
     *
     * If you are looking for an example for using the library, you are at the
     * WRONG place!  This tool has been designed to be a drop-in replacement of
     * nfcpy's llcp-test-server.py and as both libraries are not implemented
     * the same way, some hackish adaptation was required.
     */
    mqd_t llc_up = mq_open (connection->mq_up_name, O_RDWR);
    if (llc_up == (mqd_t)-1)
	return NULL;

    mqd_t llc_down = mq_open (connection->mq_down_name, O_WRONLY);
    if (llc_down == (mqd_t)-1)
	return NULL;

    for (;;) {

	uint8_t buffer[1024];
	unsigned int prio;
	/*
	 * AWFULY HACKISH
	 *
	 * The nfcpy implementation blocks for data to be available, then sleep
	 * 2 seconds, then read the data.
	 *
	 * Such a behavior is not possible with mqueues.  When data arrive,
	 * prompty put it back in the up queue after increasing it's priority
	 * so that it will be the next message to be read.  Then sleep 2
	 * seconds, and read it again.
	 */
	int res = mq_receive (llc_up, (char *) buffer, sizeof (buffer), &prio);
	pthread_testcancel ();

	struct pdu *pdu = pdu_unpack (buffer, res);
	struct pdu *reply;

	switch (pdu->ptype) {
	case PDU_I:
	    {

		/* Sleep a bit */

		struct timespec ts;
		ts.tv_sec = 2;
		ts.tv_nsec = 0;

		struct timespec ts2 = ts;
		do {
		    ts2 = ts;
		} while (0 != nanosleep (&ts2, &ts));

		/* send data */
		reply = pdu_new_i (pdu->ssap, pdu->dsap, connection, pdu->information, pdu->information_size);

		res = pdu_pack (reply, buffer, sizeof (buffer));
		mq_send (llc_down, (char *) buffer, res, 0);
		pthread_testcancel ();
		pdu_free (reply);
	    }
	    break;
	}

    }

    llc_connection_stop (connection);
    return NULL;
}
