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

#include <errno.h>
#include <cutter.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

#include "llc_link.h"
#include "llc_connection.h"
#include "llc_service.h"
#include "mac.h"

#define ECHO_SAP 16

const char *sem_cutter_name = "/cutter";
sem_t *sem_cutter;

struct dummy_mac_transport_endpoints {
    struct llc_link *initiator;
    struct llc_link *target;
};

void
cut_setup (void)
{
    if (llcp_init ())
	cut_fail ("llcp_init() failed");

    sem_cutter = sem_open (sem_cutter_name, O_CREAT | O_EXCL,  0666, 0);
    if (sem_cutter == SEM_FAILED)
	cut_fail ("sem_open() failed");
}

void
cut_teardown (void)
{
    sem_close (sem_cutter);
    sem_unlink (sem_cutter_name);
    llcp_fini ();
}

void *
echo_service (void *arg)
{
    struct llc_connection *connection = (struct llc_connection *)arg;

    int old_cancelstate;
    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_cancelstate);

    cut_set_current_test_context (connection->link->cut_test_context);

    mqd_t llc_up = mq_open (connection->mq_up_name, O_RDONLY);
    cut_assert_false (llc_up == (mqd_t)-1, cut_message ("Can't open llc_up mqueue for reading"));

    mqd_t llc_down = mq_open (connection->mq_down_name, O_WRONLY);
    cut_assert_false (llc_down == (mqd_t)-1, cut_message ("Can't open llc_down mqueue for writing"));

    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);

    for (;;) {
	char buffer[1024];
	int res = mq_receive (llc_up, buffer, sizeof (buffer), NULL);
	pthread_testcancel ();
	cut_assert_equal_int (7, res, cut_message ("Invalid message length"));
	cut_assert_equal_memory (buffer, res, "\x40\xc0Hello", 7, cut_message ("Invalid message data"));
	sem_post (sem_cutter);
	pthread_testcancel ();
    }
}

void
dummy_mac_transport (struct llc_link *initiator, struct llc_link *target)
{
    int n;
    char buffer[1024];

    for (;;) {
	struct timespec ts = {
	    .tv_sec = 0,
	    .tv_nsec = 10000,
	};
	n = mq_timedreceive (initiator->llc_down, buffer, sizeof (buffer), NULL, &ts);
	if (n < 0) {
	    if (errno == ETIMEDOUT) {
		n = 2;
		buffer[0] = buffer[1] = 0x00;
	    } else break;
	}
	pthread_testcancel ();
	n = mq_send (target->llc_up, buffer, n, 0);
	if (n < 0) break;
	pthread_testcancel ();
	n = mq_timedreceive (target->llc_down, buffer, sizeof (buffer), NULL, &ts);
	if (n < 0) {
	    if (errno == ETIMEDOUT) {
		n = 2;
		buffer[0] = buffer[1] = 0x00;
	    } else break;
	}
	pthread_testcancel ();
	n = mq_send (initiator->llc_up, buffer, n, 0);
	if (n < 0) break;
	pthread_testcancel ();
    }
}

void *
dummy_mac_transport_thread (void *arg)
{
    struct dummy_mac_transport_endpoints *eps = (struct dummy_mac_transport_endpoints *)arg;

    dummy_mac_transport (eps->initiator, eps->target);

    return NULL;
}

void
test_dummy_mac_link (void)
{
    int res;
    struct llc_link *initiator, *target;
    struct llc_service *service;
    struct mac_link mac_initiator, mac_target;

    initiator = llc_link_new ();
    cut_assert_not_null (initiator, cut_message ("llc_link_new()"));
    target = llc_link_new ();
    cut_assert_not_null (target, cut_message ("llc_link_new()"));

    initiator->cut_test_context = cut_get_current_test_context ();

    service = llc_service_new (NULL, echo_service);
    cut_assert_not_null (service, cut_message ("llc_service_new()"));

    res = llc_link_service_bind (initiator, service, ECHO_SAP);
    cut_assert_equal_int (ECHO_SAP, res, cut_message ("llc_link_service_bind()"));

    res = llc_link_activate (initiator, LLC_INITIATOR | LLC_PAX_PDU_PROHIBITED, NULL, 0);
    cut_assert_equal_int (0, res, cut_message ("llc_link_activate()"));
    res = llc_link_activate (target, LLC_TARGET | LLC_PAX_PDU_PROHIBITED, NULL, 0);
    cut_assert_equal_int (0, res, cut_message ("llc_link_activate()"));

    char buffer[1024];

    pthread_t transport;
    struct dummy_mac_transport_endpoints eps = {
	.initiator = initiator,
	.target = target,
    };
    pthread_create (&transport, NULL, dummy_mac_transport_thread, &eps);

    mac_initiator.exchange_pdus_thread = transport;
    mac_target.exchange_pdus_thread = transport;

    //initiator->mac_link = &mac_initiator;
    //target->mac_link = &mac_target;

    buffer[0] = ECHO_SAP << 2;
    buffer[1] = '\xC0';
    buffer[2] = 'H';
    buffer[3] = 'e';
    buffer[4] = 'l';
    buffer[5] = 'l';
    buffer[6] = 'o';

    res = mq_send (initiator->llc_up, buffer, 7, 0);
    cut_assert_equal_int (0, res, cut_message ("mq_send"));

    struct timespec ts = {
	.tv_sec = time(NULL) + 2,
	.tv_nsec = 0,
    };

    int old_cancelstate;
    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_cancelstate);
    res = sem_timedwait (sem_cutter, &ts);
    cut_assert_equal_int (0, res, cut_message ("Message not received"));
    pthread_setcancelstate (old_cancelstate, NULL);

    pthread_cancel (transport);
    //pthread_kill (transport, SIGUSR1);
    pthread_join (transport, NULL);

    llc_link_deactivate (initiator);
    llc_link_deactivate (target);

    llc_link_free (initiator);
    llc_link_free (target);

}
