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

#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "llc_connection.h"

#include "connectionless-echo-server.h"

void *
connectionless_echo_server_thread (void *arg)
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

    int res;
    uint8_t buffer[1024];
#if 0
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

    mq_send (llc_up, (char *) buffer, res, prio + 1);
    pthread_testcancel ();
#endif

#if 0
    struct timespec ts;
    ts.tv_sec = 2;
    ts.tv_nsec = 0;

    struct timespec ts2 = ts;
    do {
	ts2 = ts;
    } while (0 != nanosleep (&ts2, &ts));
#endif

    /*
     * Read message (for real this time
     */
    res = mq_receive (llc_up, (char *) buffer, sizeof (buffer), NULL);
    pthread_testcancel ();

    /*
     * Exchange SSAP and DSAP
     */
    uint8_t a, b;
    a = buffer[0] >> 2;
    b = buffer[1] & 0x3f;
    buffer[0] = (b << 2) | (buffer[0] & 0x03);
    buffer[1] = (buffer[1] & 0xc0) | a;

    /*
     * Send back the reply
     */
    res = mq_send (llc_down, (char *) buffer, res, 0);
    pthread_testcancel ();

    llc_connection_stop (connection);
    return NULL;
}
