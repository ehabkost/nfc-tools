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

#include <sys/param.h>
#include <sys/types.h>

#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "llc_link.h"
#include "llcp_log.h"
#include "llcp_pdu.h"
#include "llc_service.h"

#define LOG_LLCP "libnfc-llcp"
#define LLCP_MSG(priority, message) llcp_log_log (LOG_LLCP, priority, "%s", message)
#define LLCP_LOG(priority, format, ...) llcp_log_log (LOG_LLCP, priority, format, __VA_ARGS__)

void
sigusr1_handler (int x)
{
    (void) x;
    printf ("(%p) SIGUSR1\n", (void *)pthread_self ());
}

int
llcp_init (void)
{
    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sa.sa_flags  = 0;
    sigemptyset (&sa.sa_mask);
    if (sigaction (SIGUSR1, &sa, NULL) < 0)
	return -1;

    return llcp_log_init ();
}

int
llcp_fini (void)
{
    return llcp_log_fini ();
}

void llcp_thread_cleanup (void *arg);

void
llcp_thread_cleanup (void *arg)
{
    (void) arg;
    /*
     * Beware:
     * The cleanup routine is called while sem_log is held.  Trying ot log some
     * information using log4c will lead to a deadlock.
     */
}

void *
llcp_thread (void *arg)
{
    struct llc_link *link = (struct llc_link *)arg;
    mqd_t llc_up, llc_down;

    int old_cancelstate;

    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_cancelstate);

    llc_up = mq_open (link->services[0]->mq_up_name, O_RDONLY);
    llc_down = mq_open (link->services[0]->mq_down_name, O_WRONLY);

    if (llc_up == (mqd_t)-1)
	LLCP_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->services[0]->mq_up_name);
    if (llc_down == (mqd_t)-1)
	LLCP_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->services[0]->mq_down_name);

    pthread_cleanup_push (llcp_thread_cleanup, arg);
    pthread_setcancelstate (old_cancelstate, NULL);
    LLCP_LOG (LLC_PRIORITY_INFO, "(%p) Link activated", (void *)pthread_self ());
    for (;;) {
	int res;
	char buffer[1024];
	LLCP_LOG (LLC_PRIORITY_TRACE, "(%p[0]) mq_receive+", pthread_self ());
	pthread_testcancel ();
	res = mq_receive (llc_up, buffer, sizeof (buffer), NULL);
	pthread_testcancel ();
	if (res < 0) {
	    pthread_testcancel ();
	}
	LLCP_LOG (LLC_PRIORITY_TRACE, "(%p[0]) received %d bytes", pthread_self (), res);

	if (res < 2) {
	    /* FIXME: Maybe we'd rather quit */
	    LLCP_LOG (LLC_PRIORITY_FATAL, "(%p[0]) Too short for a PDU (expected 2 bytes, got %d)", pthread_self (), res);
	    buffer[0] = buffer[1] = '\0';
	    res = 2;
	}

	struct pdu *pdu;
	struct pdu **pdus, **p;
	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_cancelstate);
	pdu = pdu_unpack ((uint8_t *) buffer, res);
	switch (pdu->ptype) {
	case PDU_SYMM:
	    assert (!pdu->dsap);
	    assert (!pdu->ssap);
	    LLCP_LOG (LLC_PRIORITY_TRACE, "(%p[0]) SYMM PDU", pthread_self ());
	    break;
	case PDU_PAX:
	    assert (!pdu->dsap);
	    assert (!pdu->ssap);
	    LLCP_LOG (LLC_PRIORITY_TRACE, "(%p[0]) PAX PDU", pthread_self ());
	    assert (0 == llc_link_configure (link, pdu->information, pdu->information_size));
	    break;
	case PDU_AGF:
	    assert (!pdu->dsap);
	    assert (!pdu->ssap);
	    LLCP_LOG (LLC_PRIORITY_TRACE, "(%p[0]) AGF PDU", pthread_self ());
	    p = pdus = pdu_dispatch (pdu);
	    while (*p) {
		uint8_t buffer[BUFSIZ];
		ssize_t length = pdu_pack (*p, buffer, sizeof (buffer));
		mq_send (link->services[0]->llc_up, (char *) buffer, length, 1);
		pdu_free (*p);
		p++;
	    }

	    free (pdus);
	    break;
	case PDU_DISC:
	    /* TODO Broadcast to services */
	    LLCP_MSG (LLC_PRIORITY_CRIT, "Disconnect PDU not implemented");
//	    for (int i = 1; i <= MAX_LLC_LINK_SERVICE; i++)
//		llc_service_stop (link, i);
//	    pthread_exit (NULL);
	    break;
	case PDU_UI:
	    assert (link->services[pdu->dsap]);
	    LLCP_LOG (LLC_PRIORITY_TRACE, "(%p[0]) UI PDU", pthread_self ());
#if 0
	    /* XXX: Remove */
	    struct mq_attr attr;
	    mq_getattr (link->services[pdu->dsap]->llc_up, &attr);
	    LLCP_LOG (LLC_PRIORITY_CRIT, "(%p[0]) MQ: %d / %d x %d bytes", pthread_self (), attr.mq_curmsgs, attr.mq_maxmsg, attr.mq_msgsize);
#endif
	    if (mq_send (link->services[pdu->dsap]->llc_up, (char *) buffer, res, 0) < 0) {
		LLCP_LOG (LLC_PRIORITY_ERROR, "(%p[0]) Error sending %d bytes to service %d", pthread_self (), res, pdu->dsap);
	    } else {
		LLCP_LOG (LLC_PRIORITY_INFO, "(%p[0]) Send %d bytes to service %d", pthread_self (), res, pdu->dsap);
	    }
	    break;
	default:
	    LLCP_LOG (LLC_PRIORITY_WARN, "(%p[0]) Unsupported LLC PDU: 0x%02x", pthread_self (), pdu->ptype);
	}
	pdu_free (pdu);
	pthread_setcancelstate (old_cancelstate, NULL);

	/* ---------------- */

	ssize_t length = 0;
	for (int i = 1; i <= MAX_LLC_LINK_SERVICE; i++) {
	    if (link->services[i]) {
		length = mq_receive (link->services[i]->llc_down, buffer, sizeof (buffer), NULL);
		LLCP_LOG (LLC_PRIORITY_TRACE, "(%p[0]) Read %d bytes from service %d", pthread_self (), length, i);
		if (length > 0)
		    break;
		switch (errno) {
		case EAGAIN:
		case EINTR:
		case ETIMEDOUT: /* XXX Should not happend */
		    /* NOOP */
		    break;
		default:
		    LLCP_LOG (LLC_PRIORITY_ERROR, "(%p[0]) Can read from service %d message queue", pthread_self (), i);
		    break;
		}
	    }
	}

	LLCP_LOG (LLC_PRIORITY_TRACE, "(%p[0]) mq_send+", pthread_self ());
	pthread_testcancel();

	if (length <= 0) {
	    buffer[0] = buffer[1] = '\x00';
	    length = 2;
	}

	res = mq_send (llc_down, buffer, length, 0);
	pthread_testcancel ();

	if (res < 0) {
	    pthread_testcancel ();
	}
	LLCP_LOG (LLC_PRIORITY_TRACE, "(%p[0]) sent %d bytes", pthread_self (), length);
    }
    pthread_cleanup_pop (1);
    return NULL;
}

int
llcp_version_agreement (struct llc_link *link, struct llcp_version version)
{
    int res = -1;

    if (link->version.major == version.major) {
	link->version.minor = MIN (link->version.minor, version.minor);
	res = 0;
    } else if (link->version.major > version.major) {
	if (version.major >= 1) {
	    link->version = version;
	    res = 0;
	}
    } else {
	/* Let the remote LLC component perform version agreement */
	res = 0;
    }

    return res;
}
