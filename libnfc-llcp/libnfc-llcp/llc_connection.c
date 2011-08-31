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
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "llcp.h"
#include "llc_connection.h"
#include "llc_link.h"
#include "llc_service.h"
#include "llcp_log.h"
#include "llcp_pdu.h"
#include "llcp_parameters.h"

#define LOG_LLC_CONNECTION "libnfc-llcp.llc.connection"
#define LLC_CONNECTION_MSG(priority, message) llcp_log_log (LOG_LLC_CONNECTION, priority, "%s", message)
#define LLC_CONNECTION_LOG(priority, format, ...) llcp_log_log (LOG_LLC_CONNECTION, priority, format, __VA_ARGS__)

struct llc_connection *llc_connection_new (struct llc_link *link, uint8_t ssap, uint8_t dsap);

struct llc_connection *
llc_connection_new (struct llc_link *link, uint8_t ssap, uint8_t dsap)
{
    assert (link);

    struct llc_connection *res;

    if ((res = malloc (sizeof *res))) {
	res->link = link;
	res->thread = 0;
	res->sap = 0;
	res->dsap = dsap;
	res->ssap = ssap;
	res->status = DLC_DISCONNECTED;

	res->state.s  = 0;
	res->state.sa = 0;
	res->state.r  = 0;
	res->state.ra = 0;
	res->local_miu  = LLCP_DEFAULT_MIU;
	res->remote_miu = LLCP_DEFAULT_MIU;
	res->rwr = LLCP_DEFAULT_RW;
	res->rwl = LLCP_DEFAULT_RW;

	res->mq_up_name   = NULL;
	res->mq_down_name = NULL;
	res->llc_up   = (mqd_t) -1;
	res->llc_down = (mqd_t) -1;
    } else {
	LLC_CONNECTION_MSG (LLC_PRIORITY_FATAL, "Cannot allocate memory");
    }

    return res;
}

int
llc_connection_start (struct llc_connection *connection)
{
    assert (connection);

    struct mq_attr attr_up = {
	.mq_msgsize = 3 + connection->local_miu,
	.mq_maxmsg  = 2,
    };

    asprintf (&connection->mq_up_name, "/libnfc-llcp-%d-%p-%s", getpid(), (void *) connection, "up");
    connection->llc_up = mq_open (connection->mq_up_name, O_RDWR | O_CREAT, 0666, &attr_up);
    if (connection->llc_up == (mqd_t) -1) {
	LLC_CONNECTION_LOG (LLC_PRIORITY_FATAL, "Cannot open message queue '%s'", connection->mq_up_name);
	llc_connection_free (connection);
	return -1;
    }

    struct mq_attr attr_down = {
	.mq_msgsize = 3 + connection->remote_miu,
	.mq_maxmsg  = 2,
    };

    asprintf (&connection->mq_down_name, "/libnfc-llcp-%d-%p-%s", getpid(), (void *) connection, "down");
    connection->llc_down = mq_open (connection->mq_down_name, O_RDONLY | O_CREAT | O_NONBLOCK, 0666, &attr_down);
    if (connection->llc_down == (mqd_t) -1) {
	LLC_CONNECTION_LOG (LLC_PRIORITY_FATAL, "Cannot open message queue '%s'", connection->mq_down_name);
	llc_connection_free (connection);
	return -1;
    }

    return 0;
}

struct llc_connection *
llc_data_link_connection_new (struct llc_link *link, const struct pdu *pdu, int *reason)
{
    assert (link);
    assert (pdu);
    assert (reason);

    struct llc_connection *res;

    char sn[BUFSIZ];
    int8_t service_sap = pdu->dsap;
    uint16_t miux, miu = LLCP_DEFAULT_MIU;
    uint8_t rw = 2;

    *reason = -1;

    size_t offset = 0;
    while (offset < pdu->information_size) {
	if (offset > pdu->information_size - 2) {
	    LLC_CONNECTION_MSG (LLC_PRIORITY_ERROR, "Incomplete TLV field in parameters list");
	    return NULL;
	}
	if (offset + 2 + pdu->information[offset+1] > pdu->information_size) {
	    LLC_CONNECTION_LOG (LLC_PRIORITY_ERROR, "Incomplete TLV value in parameters list (expected %d bytes but only %d left)", pdu->information[offset+1], pdu->information_size - (offset + 2));
	    return NULL;
	}
	switch (pdu->information[offset]) {
	case LLCP_PARAMETER_MIUX:
	    if (parameter_decode_miux (pdu->information + offset, 2 + pdu->information[offset+1], &miux) < 0) {
		LLC_CONNECTION_MSG (LLC_PRIORITY_ERROR, "Invalid MIUX parameter");
		return NULL;
	    }
	    miu = 128 + miux;
	    break;
	case LLCP_PARAMETER_RW:
	    if (parameter_decode_rw (pdu->information + offset, 2 + pdu->information[offset+1], &rw) < 0) {
		LLC_CONNECTION_MSG (LLC_PRIORITY_ERROR, "Invalid RW parameter");
		return NULL;
	    }
	    break;
	case LLCP_PARAMETER_SN:
	    if (parameter_decode_sn (pdu->information + offset, 2 + pdu->information[offset+1], sn, sizeof (sn)) < 0) {
		LLC_CONNECTION_MSG (LLC_PRIORITY_ERROR, "Invalid SN parameter");
		return NULL;
	    }
	    if (pdu->dsap == 0x01) {
		service_sap = llc_link_find_sap_by_uri (link, sn);
		if (!service_sap) {
		    *reason = 0x02;
		    return NULL;
		}
	    } else {
		LLC_CONNECTION_LOG (LLC_PRIORITY_ERROR, "Ignoring SN parameter (DSAP is %d, not 1)", pdu->dsap);
	    }
	    break;
	default:
	    LLC_CONNECTION_LOG (LLC_PRIORITY_INFO, "Unknown TLV Field 0x%02x (length: %d)",
			  pdu->information[offset], pdu->information[offset+1]);
	}
	offset += 2 + pdu->information[offset+1];
    }

    if (!link->available_services[service_sap]) {
	*reason = 0x02;
	return NULL;
    }

    int8_t connection_dsap = service_sap;

    while (link->transmission_handlers[connection_dsap] && (connection_dsap <= MAX_LLC_LINK_SERVICE))
	connection_dsap++;

    if (connection_dsap > MAX_LLC_LINK_SERVICE) {
	return NULL;
    }

    if ((res = llc_connection_new (link, pdu->ssap, connection_dsap))) {
	assert (!link->transmission_handlers[connection_dsap]);
	link->transmission_handlers[connection_dsap] = res;
	res->sap = service_sap;
	res->status = DLC_NEW;
	res->rwr = rw;
	res->remote_miu = miu;
	res->local_miu  = link->available_services[service_sap]->miu;

	if (llc_connection_start (res) < 0) {
	    llc_connection_free (res);
	    return NULL;
	}
    }

    return res;
}

struct llc_connection *
llc_logical_data_link_new (struct llc_link *link, const struct pdu *pdu)
{
    assert (link);
    assert (pdu);

    struct llc_connection *res;
    uint8_t sap = 0;

    if (!link->available_services[pdu->dsap]) {
	return NULL;
    }

    while (link->datagram_handlers[sap] && (sap <= MAX_LOGICAL_DATA_LINK))
	sap++;

    if (sap > MAX_LOGICAL_DATA_LINK) {
	LLC_CONNECTION_MSG (LLC_PRIORITY_CRIT, "No place left for new Logical Data Link");
	return NULL;
    }

    if ((res = llc_connection_new (link, pdu->ssap, pdu->dsap))) {
	link->datagram_handlers[sap] = res;

	if (llc_connection_start (res) < 0) {
	    llc_connection_free (res);
	    return NULL;
	}
    }

    return res;
}

void
llc_connection_accept (struct llc_connection *connection)
{
    assert (connection->thread == pthread_self ());

    LLC_CONNECTION_LOG (LLC_PRIORITY_TRACE, "Data Link Connection [%d -> %d] accepted", connection->ssap, connection->dsap);

    connection->status = DLC_ACCEPTED;
    connection->thread = 0;
    pthread_exit (NULL);
}

void
llc_connection_reject (struct llc_connection *connection)
{
    assert (connection->thread == pthread_self ());

    LLC_CONNECTION_LOG (LLC_PRIORITY_TRACE, "Data Link Connection [%d -> %d] rejected", connection->ssap, connection->dsap);

    connection->status = DLC_REJECTED;
    connection->thread = 0;
    pthread_exit (NULL);
}

int
llc_connection_recv (struct llc_connection *connection, uint8_t *data, size_t len, uint8_t *ssap)
{
    int res;

    uint8_t buffer[BUFSIZ];
    res = mq_receive (connection->llc_up, (char *) buffer, sizeof (buffer), 0);
    if (res < 0) {
	LLC_CONNECTION_LOG (LLC_PRIORITY_ERROR, "mq_receive: %s", strerror (errno));
	return -1;
    }

    struct pdu *pdu = pdu_unpack (buffer, res);
    pdu_free (pdu);
    len = MIN (pdu->information_size, len);
    memcpy (data, pdu->information, len);

    if (ssap)
	*ssap = pdu->ssap;

    return len;
}

int
llc_connection_stop (struct llc_connection *connection)
{
    assert (connection);

    LLC_CONNECTION_LOG (LLC_PRIORITY_TRACE, "Stopping Data Link Connection [%d -> %d]", connection->ssap, connection->dsap);

    if (connection->thread == pthread_self ()) {
	connection->status = DLC_DISCONNECTED;
	pthread_exit (NULL);
    } else {
	llcp_threadslayer (connection->thread);
	connection->thread = 0;
    }
    return 0;
}

void
llc_connection_free (struct llc_connection *connection)
{
    assert (connection);

    LLC_CONNECTION_LOG (LLC_PRIORITY_TRACE, "Freeing Data Link Connection [%d -> %d]", connection->ssap, connection->dsap);

    if (connection->llc_up != (mqd_t) -1)
	mq_close (connection->llc_up);
    if (connection->llc_down != (mqd_t) -1)
	mq_close (connection->llc_down);
    if (connection->mq_up_name)
	mq_unlink (connection->mq_up_name);
    if (connection->mq_down_name)
	mq_unlink (connection->mq_down_name);

    free (connection->mq_up_name);
    free (connection->mq_down_name);
    free (connection);
}
