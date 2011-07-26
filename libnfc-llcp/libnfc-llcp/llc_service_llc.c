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
#if defined(HAVE_PTHREAD_NP_H)
#  include <pthread_np.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "llc_link.h"
#include "llc_connection.h"
#include "llcp_log.h"
#include "llcp_pdu.h"
#include "llc_service.h"
#include "mac.h"

#define LOG_LLC_SERVICE_LLC "libnfc-llcp.llc.llc"
#define LLC_SERVICE_LLC_MSG(priority, message) llcp_log_log (LOG_LLC_SERVICE_LLC, priority, "(%p) %s", pthread_self (), message)
#define LLC_SERVICE_LLC_LOG(priority, format, ...) llcp_log_log (LOG_LLC_SERVICE_LLC, priority, "(%p) " format, pthread_self (), __VA_ARGS__)

#define INC_MOD_16(x) x = (x + 1) % 16

void
llc_service_llc_thread_cleanup (void *arg)
{
    /*
     * Beware:
     * The cleanup routine is called while sem_log is held.  Trying ot log some
     * information using log4c will lead to a deadlock.
     */
    struct llc_link *link = (struct llc_link *)arg;

    if (link->llc_up != (mqd_t) -1)
	mq_close (link->llc_up);

    if (link->llc_down != (mqd_t) -1)
	mq_close (link->llc_down);

    link->llc_up   = (mqd_t) -1;
    link->llc_down = (mqd_t) -1;
}

void *
llc_service_llc_thread (void *arg)
{
    struct llc_link *link = (struct llc_link *)arg;
    mqd_t llc_up, llc_down;

    int old_cancelstate;
#if defined(HAVE_DECL_PTHREAD_SET_NAME_NP) && HAVE_DECL_PTHREAD_SET_NAME_NP
    char *thread_name;
#endif

    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_cancelstate);

    llc_up = mq_open (link->mq_up_name, O_RDONLY);
    llc_down = mq_open (link->mq_down_name, O_WRONLY);

    if (llc_up == (mqd_t)-1)
	LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->mq_up_name);
    if (llc_down == (mqd_t)-1)
	LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->mq_down_name);

    pthread_cleanup_push (llc_service_llc_thread_cleanup, arg);
    pthread_setcancelstate (old_cancelstate, NULL);
    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_INFO, "Link activated");
    for (;;) {
	int res;
	uint8_t buffer[1024];
	LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "mq_receive+");
	pthread_testcancel ();
	res = mq_receive (llc_up, (char *) buffer, sizeof (buffer), NULL);
	pthread_testcancel ();
	if (res < 0) {
	    pthread_testcancel ();
	}
	LLC_SERVICE_LLC_LOG (LLC_PRIORITY_TRACE, "Received %d bytes", res);

	if (res < 2) {
	    /* FIXME: Maybe we'd rather quit */
	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_FATAL, "Too short for a PDU (expected 2 bytes, got %d)", res);
	    buffer[0] = buffer[1] = '\0';
	    res = 2;
	}

	struct pdu *pdu;
	struct pdu **pdus, **p;
	struct llc_connection *connection;
	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_cancelstate);
	pdu = pdu_unpack ((uint8_t *) buffer, res);
	switch (pdu->ptype) {
	case PDU_SYMM:
	    assert (!pdu->dsap);
	    assert (!pdu->ssap);
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "Symmetry PDU");
	    break;
	case PDU_PAX:
	    assert (!pdu->dsap);
	    assert (!pdu->ssap);
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "Parameter Exchange PDU");
	    assert (0 == llc_link_configure (link, pdu->information, pdu->information_size));
	    break;
	case PDU_AGF:
	    assert (!pdu->dsap);
	    assert (!pdu->ssap);
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "Aggregated Frame PDU");
	    p = pdus = pdu_dispatch (pdu);
	    while (*p) {
		uint8_t buffer[BUFSIZ];
		ssize_t length = pdu_pack (*p, buffer, sizeof (buffer));
		mq_send (link->llc_up, (char *) buffer, length, 1);
		pdu_free (*p);
		p++;
	    }

	    free (pdus);
	    break;
	case PDU_SNL:
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "Service Name Lookup PDU");
	    goto spawn_logical_data_link;

	case PDU_UI:
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "Unnumbered Information PDU");
spawn_logical_data_link:
	    if (!link->available_services[pdu->dsap]) {
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_FATAL, "No service bound to SAP %d", pdu->dsap);
		break;
	    }

	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_TRACE, "Spawning Logical Data Link [%d -> %d]", pdu->ssap, pdu->dsap);
	    if (!(connection = llc_logical_data_link_new (link, pdu))) {
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "Cannot establish Logical Data Link [%d -> %d]", pdu->ssap, pdu->dsap);
		break;
	    }

	    if (pthread_create (&connection->thread, NULL, link->available_services[pdu->dsap]->thread_routine, connection) < 0) {
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "Cannot launch Logical Data Link [%d -> %d] thread", connection->ssap, connection->dsap);
		break;
	    }
#if defined(HAVE_DECL_PTHREAD_SET_NAME_NP) && HAVE_DECL_PTHREAD_SET_NAME_NP
	    asprintf (&thread_name, "LDL on SAP %d", connection->sap);
	    pthread_set_name_np (connection->thread, thread_name);
	    free (thread_name);
#endif

	    if (mq_send (connection->llc_up, (char *) buffer, res, 0) < 0) {
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "Cannot send data to Logical Data Link [%d -> %d]", connection->ssap, connection->dsap);
		break;
	    }

	    break;
	case PDU_RR:
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "Receive Ready PDU");

	    assert (link->transmission_handlers[pdu->dsap]);
	    link->transmission_handlers[pdu->dsap]->state.sa = pdu->nr;
	    break;
	case PDU_CONNECT:
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "Connect PDU");
	    if (!link->available_services[pdu->dsap]) {
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_FATAL, "No service bound to SAP %d", pdu->dsap);
		break;
	    }

	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_TRACE, "Spawning Data Link Connection [%d -> %d]", pdu->ssap, pdu->dsap);
	    int error;
	    if (!(connection = llc_data_link_connection_new (link, pdu, &error))) {
		struct pdu *reply;
		int len;
		uint8_t reason[] = { error };

		switch (reason[0]) {
		case 0x02:
		case 0x20:
		    // XXX Log something that makes sense
		    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "(%p[0]) XXX", pthread_self ());
		    reply = pdu_new_dm (pdu->ssap, pdu->dsap, reason);
		    len = pdu_pack (reply, buffer, sizeof (buffer));
		    pdu_free (reply);
		    if (mq_send (llc_down, (char *) buffer, len, 0) < 0) {
			LLC_SERVICE_LLC_MSG (LLC_PRIORITY_FATAL, "Can't Reject connection");
		    }
		    break;
		default:
		    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "Cannot establish Data Link Connection [%d -> %d] (reason = %02x)", pdu->ssap, pdu->dsap, reason);
		}
		break;
	    }
	    if (pthread_create (&connection->thread, NULL, link->available_services[connection->sap]->accept_routine, connection) < 0) {
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "Cannot launch Data Link Connection [%d -> %d] accept routine", connection->ssap, connection->dsap);
		break;
	    }
#if defined(HAVE_DECL_PTHREAD_SET_NAME_NP) && HAVE_DECL_PTHREAD_SET_NAME_NP
	    asprintf (&thread_name, "DLC Accept on SAP %d", connection->sap);
	    pthread_set_name_np (connection->thread, thread_name);
	    free (thread_name);
#endif

	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_TRACE, "Data Link Connection [%d -> %d] accept routine launched (service %d)", connection->ssap, connection->dsap, connection->sap);
	    break;
	case PDU_DISC:
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "Disconnect PDU");
	    if (!pdu->dsap && !pdu->ssap) {
		/*
		 * Deactivate the MAC layer (if any): there can be a race condition when
		 * service 0 is stopped but the MAC tries to access it's message queue.
		 */
		if (link->mac_link && link->mac_link->exchange_pdus_thread) {
		    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_INFO, "Stopping MAC link");
		    pthread_cancel (link->mac_link->exchange_pdus_thread);
		}
		pthread_exit ((void *) 2);
		break;
	    } else {
		struct pdu *reply;

		llc_connection_stop (link->transmission_handlers[pdu->dsap]);
		llc_connection_free (link->transmission_handlers[pdu->dsap]);
		link->transmission_handlers[pdu->dsap] = NULL;

		uint8_t reason[1] = { 0x00 };
		reply = pdu_new_dm (pdu->ssap, pdu->dsap, reason);
		int len = pdu_pack (reply, buffer, sizeof (buffer));
		pdu_free (reply);
		if (mq_send (llc_down, (char *) buffer, len, 0) < 0) {
		    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_FATAL, "Can't send DM");
		}
	    }
	    break;
	case PDU_I:
	    assert (link->transmission_handlers[pdu->dsap]);
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "Information PDU");
#if 1
	    /* XXX: Remove */
	    struct mq_attr attr;
	    mq_getattr (link->transmission_handlers[pdu->dsap]->llc_up, &attr);
	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_CRIT, "MQ: %d / %d x %d bytes", attr.mq_curmsgs, attr.mq_maxmsg, attr.mq_msgsize);
#endif
	    if (pdu->ns != link->transmission_handlers[pdu->dsap]->state.r) {
		LLC_SERVICE_LLC_MSG (LLC_PRIORITY_FATAL, "Invalid N(S)");
		struct pdu *reply = pdu_new_frmr (pdu->ssap, pdu->dsap, pdu, link->transmission_handlers[pdu->dsap], FRMR_S);
		int len = pdu_pack (reply, buffer, sizeof (buffer));
		pdu_free (reply);
		if (mq_send (llc_down, (char *) buffer, len, 0) < 0) {
		    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_FATAL, "Can't send FRMR");
		}

		break;
	    }

	    if (pdu->information_size > link->transmission_handlers[pdu->dsap]->local_miu) {
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_FATAL, "Information PDU too long: %d (MIU: %d)", pdu->information_size, link->transmission_handlers[pdu->dsap]->local_miu);
		struct pdu *reply = pdu_new_frmr (pdu->ssap, pdu->dsap, pdu, link->transmission_handlers[pdu->dsap], FRMR_I);
		int len = pdu_pack (reply, buffer, sizeof (buffer));
		pdu_free (reply);
		if (mq_send (llc_down, (char *) buffer, len, 0) < 0) {
		    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_FATAL, "Can't send FRMR");
		}

		break;
	    }

	    INC_MOD_16 (link->transmission_handlers[pdu->dsap]->state.r);
	    link->transmission_handlers[pdu->dsap]->state.sa = pdu->nr;

	    if (mq_send (link->transmission_handlers[pdu->dsap]->llc_up, (char *) buffer, res, 0) < 0) {
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "Error sending %d bytes to service %d", res, pdu->dsap);
	    } else {
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_INFO, "Send %d bytes to service %d", res, pdu->dsap);
	    }
	    break;
	case PDU_FRMR:
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_ERROR, "Frame Reject PDU");
	    assert (pdu->information_size == 4);
	    if (pdu->information[0] & 0x80) {
		LLC_SERVICE_LLC_MSG (LLC_PRIORITY_ERROR, "PDU was invalid or malformed");
	    } else {
		LLC_SERVICE_LLC_MSG (LLC_PRIORITY_INFO, "PDU was valid and wellformed");
	    }
	    if (pdu->information[0] & 0x40) {
		LLC_SERVICE_LLC_MSG (LLC_PRIORITY_ERROR, "PDU has incorect or unexpected information field");
	    } else {
		LLC_SERVICE_LLC_MSG (LLC_PRIORITY_INFO, "PDU has no incorect or unexpected information field");
	    }
	    if (pdu->information[0] & 0x20) {
		LLC_SERVICE_LLC_MSG (LLC_PRIORITY_ERROR, "PDU contains an invalid receive sequence number N(R)");
	    } else {
		LLC_SERVICE_LLC_MSG (LLC_PRIORITY_INFO, "PDU contains a valid receive sequence number N(R)");
	    }
	    if (pdu->information[0] & 0x10) {
		LLC_SERVICE_LLC_MSG (LLC_PRIORITY_ERROR, "PDU contains an invalid send sequence number N(S)");
	    } else {
		LLC_SERVICE_LLC_MSG (LLC_PRIORITY_INFO, "PDU contains a valid send sequence number N(S)");
	    }
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_ERROR, "Rejected frame informations:");
	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "  PDU type: %d", pdu->information[0] & 0x0F);
	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "  Sequence: %02x", pdu->information[1]);
	    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_ERROR, "Receiver status:");
	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "  V(S):  %02x", pdu->information[2] >> 4);
	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "  V(R):  %02x", pdu->information[2] & 0x0F);
	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "  V(SA): %02x", pdu->information[3] >> 4);
	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "  V(RA): %02x", pdu->information[3] & 0x0F);

	    break;
	default:
	    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_WARN, "Unsupported LLC PDU: 0x%02x", pdu->ptype);
	    abort();
	}
	pdu_free (pdu);
	pthread_setcancelstate (old_cancelstate, NULL);

	/* ---------------- */

	ssize_t length = 0;
#if 1
	for (int i = 0; i <= MAX_LOGICAL_DATA_LINK; i++) {
	    if (link->datagram_handlers[i]) {
		pthread_t thread = link->datagram_handlers[i]->thread;
		length = mq_receive (link->datagram_handlers[i]->llc_down, (char *) buffer, sizeof (buffer), NULL);
		if (length > 0)
		    break;
		switch (errno) {
		case EAGAIN:
		    if (!thread) {
			/*
			 * The service is not running anymore and it's down
			 * queue is empty.  It can be garbage collected.
			 */
			LLC_SERVICE_LLC_LOG (LLC_PRIORITY_TRACE, "Garbage-collecting Logical Data Link [%d -> %d]", link->datagram_handlers[i]->ssap, link->datagram_handlers[i]->dsap);
			llc_connection_free (link->datagram_handlers[i]);
			link->datagram_handlers[i] = NULL;
		    }
		    /* FALLTHROUGH */
		case EINTR:
		case ETIMEDOUT: /* XXX Should not happend */
		    /* NOOP */
		    break;
		default:
		    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "Can' read from service %d message queue", i);
		    break;
		}
	    }
	}
	for (int i = 1; i <= MAX_LLC_LINK_SERVICE; i++) {
	    if (link->transmission_handlers[i]) {
		pthread_t thread = link->transmission_handlers[i]->thread;
		length = mq_receive (link->transmission_handlers[i]->llc_down, (char *) buffer, sizeof (buffer), NULL);
		LLC_SERVICE_LLC_LOG (LLC_PRIORITY_TRACE, "Read %d bytes from service %d", length, i);
		if (length > 0) {
		    struct pdu *pdu = pdu_unpack (buffer, length);

		    if (pdu->ptype == PDU_I) {
			if (link->transmission_handlers[i]->state.s == link->transmission_handlers[i]->state.sa + link->transmission_handlers[i]->rwr) {
			    /*
			     * We can't send data now
			     */
			    mq_send (link->transmission_handlers[i]->llc_down, (char *) buffer, length, 1);
			    length = -1;
			    continue;
			}
		    }
		    struct pdu *reply = pdu_new_i (pdu->dsap, pdu->ssap, link->transmission_handlers[i], pdu->information, pdu->information_size);
		    length = pdu_pack (pdu, buffer, sizeof (buffer));
		    free (reply);
		    pdu_free (pdu);
		    INC_MOD_16 (link->transmission_handlers[i]->state.s);
		    break;
		}
		switch (errno) {
		case EAGAIN:
		    if (thread) {
			/*
			 * If we have received some data not yet acknoledge, do it now.
			 */
#if 1
			/* XXX: Remove */
			LLC_SERVICE_LLC_LOG (LLC_PRIORITY_CRIT, "%d %d %d %d",
				  link->transmission_handlers[i]->state.s,
				  link->transmission_handlers[i]->state.sa,
				  link->transmission_handlers[i]->state.r,
				  link->transmission_handlers[i]->state.ra
				  );
#endif

			if (link->transmission_handlers[i]->state.ra != link->transmission_handlers[i]->state.r) {
			    LLC_SERVICE_LLC_MSG (LLC_PRIORITY_WARN, "Send acknoledgment for received data");
			    struct pdu *reply;
			    struct mq_attr attr;
			    mq_getattr (link->transmission_handlers[pdu->dsap]->llc_up, &attr);
			    if (attr.mq_curmsgs == attr.mq_maxmsg) {
				LLC_SERVICE_LLC_MSG (LLC_PRIORITY_INFO, "Message queue is full");
				reply = pdu_new_rnr (link->transmission_handlers[i]);
			    } else {
				reply = pdu_new_rr (link->transmission_handlers[i]);
			    }
			    length = pdu_pack (reply, buffer, sizeof (buffer));
			    pdu_free (reply);
			    link->transmission_handlers[i]->state.ra = link->transmission_handlers[i]->state.r;
			    break;
			}
		    } else {
			struct pdu *reply;
			uint8_t reason[] = { 0x00 };
			switch (link->transmission_handlers[i]->status) {
			case DLC_NEW:
			case DLC_CONNECTED:
			    /*
			     * The llc_connection thread is running.
			     * Do nothing.
			     */
			    break;
			case DLC_ACCEPTED:
			    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_TRACE, "Data Link Connection [%d -> %d] accepted (service %d).  Sending CC", connection->ssap, connection->dsap, connection->sap);
			    reply = pdu_new_cc (connection->ssap, connection->dsap);
			    length = pdu_pack (reply, buffer, sizeof (buffer));
			    pdu_free (reply);

			    if (pthread_create (&connection->thread, NULL, connection->link->available_services[connection->sap]->thread_routine, connection) < 0) {
				LLC_SERVICE_LLC_MSG (LLC_PRIORITY_FATAL, "Cannot start Data Link Connection thread");
				link->transmission_handlers[i]->status = DLC_DISCONNECTED;
				break;
			    }
#if defined(HAVE_DECL_PTHREAD_SET_NAME_NP) && HAVE_DECL_PTHREAD_SET_NAME_NP
			    asprintf (&thread_name, "DLC on SAP %d", connection->sap);
			    pthread_set_name_np (connection->thread, thread_name);
			    free (thread_name);
#endif
			    link->transmission_handlers[i]->status = DLC_CONNECTED;
			    break;
			case DLC_REJECTED:
			    reason[0] = 0x03;
			    link->transmission_handlers[i]->status = DLC_DISCONNECTED;
			    /* FALLTHROUGH */
			case DLC_DISCONNECTED:
			    reply = pdu_new_dm (connection->ssap, connection->dsap, reason);
			    length = pdu_pack (reply, buffer, sizeof (buffer));
			    pdu_free (reply);
			    link->transmission_handlers[i]->status = DLC_TERMINATED;
			    /* FALLTHROUGH */
			case DLC_TERMINATED:
			    /*
			     * The service is not running anymore and it's down
			     * queue is empty.  It can be garbage collected.
			     */
			    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_TRACE, "Garbage-collecting Data Link Connection [%d -> %d]", link->transmission_handlers[i]->ssap, link->transmission_handlers[i]->dsap);
			    llc_connection_free (link->transmission_handlers[i]);
			    link->transmission_handlers[i] = NULL;
			    break;
			}
		    }
		    /* FALLTHROUGH */
		case EINTR:
		case ETIMEDOUT: /* XXX Should not happend */
		    /* NOOP */
		    break;
		default:
		    LLC_SERVICE_LLC_LOG (LLC_PRIORITY_ERROR, "Can't read from service %d message queue", i);
		    break;
		}
	    }
	}
#endif

	LLC_SERVICE_LLC_MSG (LLC_PRIORITY_TRACE, "mq_send+");
	pthread_testcancel();

	if (length <= 0) {
	    buffer[0] = buffer[1] = '\x00';
	    length = 2;
	}

	res = mq_send (llc_down, (char *) buffer, length, 0);
	pthread_testcancel ();

	if (res < 0) {
	    pthread_testcancel ();
	}
	LLC_SERVICE_LLC_LOG (LLC_PRIORITY_TRACE, "Sent %d bytes", length);
    }
    pthread_cleanup_pop (1);
    return NULL;
}
