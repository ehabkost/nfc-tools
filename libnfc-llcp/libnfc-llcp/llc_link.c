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

#include <sys/types.h>

#include <assert.h>
#include <fcntl.h>
#if defined(HAVE_PTHREAD_NP_H)
#  include <pthread_np.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "llc_connection.h"
#include "llcp_log.h"
#include "llcp_parameters.h"
#include "llcp_pdu.h"
#include "llc_link.h"
#include "llc_service.h"
#include "llc_service_llc.h"
#include "llc_service_sdp.h"
#include "mac.h"

#define LOG_LLC_LINK "libnfc-llcp.llc.link"
#define LLC_LINK_MSG(priority, message) llcp_log_log (LOG_LLC_LINK, priority, "%s", message)
#define LLC_LINK_LOG(priority, format, ...) llcp_log_log (LOG_LLC_LINK, priority, format, __VA_ARGS__)

struct llc_link *
llc_link_new (void)
{
    struct llc_link *link;

    if ((link = malloc (sizeof (*link)))) {
	link->status = LL_DEACTIVATED;
	link->version.major = LLCP_VERSION_MAJOR;
	link->version.minor = LLCP_VERSION_MINOR;
	link->opt = LINK_SERVICE_CLASS_3;
	for (size_t i = 0; i < sizeof (link->available_services) / sizeof (*link->available_services); i++) {
	    link->available_services[i] = NULL;
	    link->datagram_handlers[i] = NULL;
	    link->transmission_handlers[i] = NULL;
	}
	link->cut_test_context = NULL;
	link->mac_link = NULL;
	link->local_miu = LLCP_DEFAULT_MIU;

	asprintf (&link->mq_up_name, "/libnfc-llcp-%d-%p-up", getpid (), (void *) link);
	asprintf (&link->mq_down_name, "/libnfc-llcp-%d-%p-down", getpid (), (void *) link);
	link->llc_up   = (mqd_t) -1;
	link->llc_down = (mqd_t) -1;

	struct llc_service *sdp_service = llc_service_new_with_uri (NULL, llc_service_sdp_thread, LLCP_SDP_URI, NULL);

	if (llc_link_service_bind (link, sdp_service, LLCP_SDP_SAP) < 0) {
	    LLC_LINK_MSG (LLC_PRIORITY_FATAL, "Cannot bind LLC service 1");
	    llc_link_free (link);
	    return NULL;
	}
    }

    return link;
}

int
llc_link_free_sap (struct llc_link *link)
{
    int sap;
    for (sap = 0x10; link->available_services[sap] && (sap <= 0x1F); sap++);
    if (sap > 0x1F) {
	LLC_LINK_MSG (LLC_PRIORITY_ERROR, "No free SAP");
	return -1;
    }
    return sap;
}

int
llc_link_service_bind (struct llc_link *link, struct llc_service *service, int8_t sap)
{
    assert (link);
    assert (service);
    assert (sap <= MAX_LLC_LINK_SERVICE);

    if ((SAP_AUTO == sap) && ((sap = llc_link_free_sap (link)) < 0)) {
	return -1;
    }

    if (link->available_services[sap]) {
	LLC_LINK_LOG (LLC_PRIORITY_ERROR, "SAP %d already bound", sap);
	return -1;
    }

    service->sap = sap;
    link->available_services[sap] = service;

    LLC_LINK_LOG (LLC_PRIORITY_TRACE, "service %p bound to SAP %d", (void *) service, sap);

    return sap;
}

void
llc_link_service_unbind (struct llc_link *link, uint8_t sap)
{
    if (link->available_services[sap]) {
	link->available_services[sap]->sap = -1;
	link->available_services[sap] = NULL;
    }
}

uint16_t
llc_link_get_wks (const struct llc_link *link)
{
    uint16_t wks = 0x0000;
    for (int i = 0; i < 16; i++) {
	wks |= (link->available_services[i] ? 1 : 0) << i;
    }

    return wks;
}

int
llc_link_activate (struct llc_link *link, uint8_t flags, const uint8_t *parameters, size_t length)
{
    assert (link);
    assert (flags == (flags & 0x03));

    link->role = flags & 0x01;
    link->version.major = LLCP_VERSION_MAJOR;
    link->version.minor = LLCP_VERSION_MINOR;
    link->local_miu  = LLCP_DEFAULT_MIU;
    link->remote_miu = LLCP_DEFAULT_MIU;
    link->remote_wks = 0x0001;
    link->local_lto.tv_sec  = 1;
    link->local_lto.tv_usec = 0;
    link->remote_lto.tv_sec  = 0;
    link->remote_lto.tv_usec = 100000;
    link->local_lsc  = 3;
    link->remote_lsc = 3;

    if (llc_link_configure (link, parameters, length) < 0) {
	LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Link configuration failed");
	llc_link_deactivate (link);
	return -1;
    }

    switch (flags & 0x01) {
    case LLC_INITIATOR:
    case LLC_TARGET:
	break;
    }

    if (!(flags & LLC_PAX_PDU_PROHIBITED)) {
	/* FIXME: Exchange PAX PDU */
    }

    /*
     * Start link
     */
    struct mq_attr attr_up = {
	.mq_msgsize = 3 + link->local_miu,
	.mq_maxmsg  = 2,
    };
    LLC_LINK_LOG (LLC_PRIORITY_DEBUG, "mq_open (%s)", link->mq_up_name);
    link->llc_up   = mq_open (link->mq_up_name, O_CREAT | O_EXCL | O_WRONLY | O_NONBLOCK, 0666, &attr_up);
    if (link->llc_up == (mqd_t)-1) {
	LLC_LINK_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->mq_up_name);
	return -1;
    }

    struct mq_attr attr_down = {
	.mq_msgsize = 3 + link->remote_miu,
	.mq_maxmsg  = 2,
    };
    LLC_LINK_LOG (LLC_PRIORITY_DEBUG, "mq_open (%s)", link->mq_down_name);
    link->llc_down = mq_open (link->mq_down_name, O_CREAT | O_EXCL | O_RDWR, 0666, &attr_down);
    if (link->llc_down == (mqd_t)-1) {
	LLC_LINK_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->mq_down_name);
	return -1;
    }

    if ((pthread_create (&link->thread, NULL, llc_service_llc_thread, link)) == 0) {
#if defined(HAVE_DECL_PTHREAD_SET_NAME_NP) && HAVE_DECL_PTHREAD_SET_NAME_NP
	pthread_set_name_np (link->thread, "LLC Link");
#endif
	LLC_LINK_MSG (LLC_PRIORITY_INFO, "LLC Link started successfully");
    } else {
	return -1;
    }

    link->status = LL_ACTIVATED;

    return 0;
}

int
llc_link_configure (struct llc_link *link, const uint8_t *parameters, size_t length)
{
    struct llcp_version version;
    uint16_t miux;
    uint8_t lto;
    uint8_t opt;

    LLC_LINK_LOG (LLC_PRIORITY_TRACE, "llc_link_configure (%p, %p, %d)", (void *)link, (void *) parameters, length);

    size_t offset = 0;
    while (offset < length) {
	if (offset > length - 2) {
	    LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Incomplete TLV field in parameters list");
	    return -1;
	}
	if (offset + 2 + parameters[offset+1] > length) {
	    LLC_LINK_LOG (LLC_PRIORITY_ERROR, "Incomplete TLV value in parameters list (expected %d bytes but only %d left)", parameters[offset+1], length - (offset + 2));
	    return -1;
	}
	switch (parameters[offset]) {
	case LLCP_PARAMETER_VERSION:
	    if (parameter_decode_version (parameters + offset, 2 + parameters[offset+1], &version) < 0) {
		LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Invalid Version TLV parameter");
		return -1;
	    }
	    LLC_LINK_LOG (LLC_PRIORITY_DEBUG, "Version: %d.%d (remote)", version.major, version.minor);
	    if (llcp_version_agreement (link, version) < 0) {
		LLC_LINK_MSG (LLC_PRIORITY_WARN, "LLCP Version Agreement Procedure failed");
		return -1;
	    }
	    LLC_LINK_LOG (LLC_PRIORITY_DEBUG, "Version: %d.%d (agreed)", version.major, version.minor);
	    break;
	case LLCP_PARAMETER_MIUX:
	    if (parameter_decode_miux (parameters + offset, 2 + parameters[offset+1], &miux) < 0) {
		LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Invalid MIUX TLV parameter");
		return -1;
	    }
	    link->remote_miu = miux + 128;
	    LLC_LINK_LOG (LLC_PRIORITY_DEBUG, "MIUX: %d (0x%02x)", miux + 128, miux);
	    break;
	case LLCP_PARAMETER_WKS:
	    if (parameter_decode_wks (parameters + offset, 2 + parameters[offset+1], &link->remote_wks) < 0) {
		LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Invalid WKS TLV parameter");
		return -1;
	    }
	    LLC_LINK_LOG (LLC_PRIORITY_DEBUG, "WKS: 0x%04x", link->remote_wks);
	    break;
	case LLCP_PARAMETER_LTO:
	    if (parameter_decode_lto (parameters + offset, 2 + parameters[offset+1], &lto) < 0) {
		LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Invalid LTO TLV parameter");
		return -1;
	    }
	    link->remote_lto.tv_sec = (lto * 10 * 1000) / 1000000;
	    link->remote_lto.tv_usec = (lto * 10 * 1000) % 1000000;
	    LLC_LINK_LOG (LLC_PRIORITY_DEBUG, "LTO: %d ms (0x%02x)", 10 * lto, lto);
	    break;
	case LLCP_PARAMETER_OPT:
	    if (parameter_decode_opt (parameters + offset, 2 + parameters[offset+1], &opt) < 0) {
		LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Invalid OPT TLV parameter");
		return -1;
	    }
	    link->remote_lsc = opt & 0x03;
	    LLC_LINK_LOG (LLC_PRIORITY_DEBUG, "OPT: 0x%02x", opt);
	    break;
	default:
	    LLC_LINK_LOG (LLC_PRIORITY_INFO, "Unknown TLV Field 0x%02x (length: %d)",
			  parameters[offset], parameters[offset+1]);
	}
	offset += 2 + parameters[offset+1];
    }
    if (offset != length) {
	LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Unprocessed TLV parameters");
	return -1;
    }
    return 0;
}

int
llc_link_encode_parameters (const struct llc_link *link, uint8_t *parameters, size_t length)
{
    int n;
    uint8_t *parameter = parameters;

    if ((n = parameter_encode_version (parameter, length, link->version)) < 0)
	return -1;
    parameter += n; length -= n;

    if ((n = parameter_encode_miux (parameter, length, link->local_miu)) < 0)
	return -1;
    parameter += n; length -= n;

    if ((n = parameter_encode_wks (parameter, length, llc_link_get_wks(link))) < 0)
	return -1;
    parameter += n; length -= n;

    uint8_t lto = link->local_lto.tv_sec * 100 + link->local_lto.tv_usec / 10000;
    if ((n = parameter_encode_lto (parameter, length, lto)) < 0)
	return -1;
    parameter += n; length -= n;

    if ((n = parameter_encode_opt (parameter, length, link->opt)) < 0)
	return -1;
    parameter += n; length -= n;

    return parameter - parameters;
}

uint8_t
llc_link_find_sap_by_uri (const struct llc_link *link, const char *uri)
{
    LLC_LINK_LOG (LLC_PRIORITY_TRACE, "Searching SAP for service '%s'", uri);
    int8_t res = 0;
    for (int i = 1; i <= MAX_LLC_LINK_ADVERTISED_SERVICE; i++) {
	if (link->available_services[i]) {
	    if (link->available_services[i]->uri &&
		(0 == strcmp (link->available_services[i]->uri, uri))) {
		LLC_LINK_LOG (LLC_PRIORITY_INFO, "Service '%s' is bound to SAP '%d'", uri, i);
		res = i;
		break;
	    }
	}
    }

    if (!res)
	LLC_LINK_LOG (LLC_PRIORITY_INFO, "Service '%s' not available", uri);

    return res;
}

int
llc_link_send_pdu (struct llc_link *link, const struct pdu *pdu)
{
    assert (link);
    assert (link->status == LL_ACTIVATED);
    if (!pdu) {
	LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Can't send empty PDU");
	return -1;
    }

    uint8_t buffer[BUFSIZ];
    int len = pdu_pack (pdu, buffer, sizeof (buffer));

    if (mq_send (link->llc_down, (char *) buffer, len, 0) < 0) {
	LLC_LINK_MSG (LLC_PRIORITY_ERROR, "Error enqueuing PDU");
	return -1;
    }

    return 0;
}

int
llc_link_send_data (struct llc_link *link, uint8_t local_sap, uint8_t remote_sap, const uint8_t *data, size_t len)
{
    struct pdu *pdu = pdu_new (remote_sap, PDU_UI, local_sap, 0, 0, data, len);
    int res = llc_link_send_pdu (link, pdu);
    pdu_free (pdu);

    return res;
}

void
llc_link_deactivate (struct llc_link *link)
{
    assert (link);

    LLC_LINK_MSG (LLC_PRIORITY_INFO, "Deactivating LLC Link");

    link->status = LL_DEACTIVATED;

    if (link->mac_link) {
	LLC_LINK_MSG (LLC_PRIORITY_DEBUG, "The LLC Link has an active MAC link");
	mac_link_deactivate (link->mac_link, MAC_DEACTIVATE_ON_REQUEST);
	link->mac_link = NULL;
	LLC_LINK_MSG (LLC_PRIORITY_DEBUG, "Back to LLC Link deactivation");
    }

    if (link->thread == pthread_self ()) {
	/*
	 * The thread is supposed to terminate once the function has returned.
	 */
    } else {
	if (link->thread) {
	    llcp_threadslayer (link->thread);
	    link->thread = NULL;
	}
    }

    uint8_t local_sap;
    uint8_t remote_sap;

    for (int i = 0; i <= MAX_LOGICAL_DATA_LINK; i++) {
	if (link->datagram_handlers[i]) {
	    remote_sap = link->datagram_handlers[i]->remote_sap;
	    local_sap = link->datagram_handlers[i]->local_sap;
	    LLC_LINK_LOG (LLC_PRIORITY_INFO, "Stopping Logical Data Link [%d -> %d]", local_sap, remote_sap);
	    llc_connection_stop (link->datagram_handlers[i]);
	    llc_connection_free (link->datagram_handlers[i]);
	    link->datagram_handlers[i] = NULL;
	    LLC_LINK_LOG (LLC_PRIORITY_INFO, "Logical Data Link [%d -> %d] stopped", local_sap, remote_sap);
	}
    }
    for (int i = 0; i <= MAX_LLC_LINK_SERVICE; i++) {
	if (link->transmission_handlers[i]) {
	    LLC_LINK_LOG (LLC_PRIORITY_INFO, "Stopping Data Link Connection [%d -> %d]", local_sap, remote_sap);
	    llc_connection_stop (link->transmission_handlers[i]);
	    llc_connection_free (link->transmission_handlers[i]);
	    link->transmission_handlers[i] = NULL;
	    LLC_LINK_LOG (LLC_PRIORITY_INFO, "Data Link Connection [%d -> %d] stopped", local_sap, remote_sap);
	}
    }

    if (link->llc_up != (mqd_t) -1)
	mq_close (link->llc_up);
    if (link->llc_down != (mqd_t) -1)
	mq_close (link->llc_down);

    if (link->mq_up_name)
	mq_unlink (link->mq_up_name);
    if (link->mq_down_name)
	mq_unlink (link->mq_down_name);

    link->llc_up   = (mqd_t) -1;
    link->llc_down = (mqd_t) -1;
    LLC_LINK_MSG (LLC_PRIORITY_INFO, "LLC Link deactivated");
}

void
llc_link_free (struct llc_link *link)
{
    assert (link);

    for (int i = MAX_LLC_LINK_SERVICE; i >= 0; i--) {
	if (link->available_services[i]) {
	    LLC_LINK_LOG (LLC_PRIORITY_INFO, "Freeing service %d", i);
	    llc_service_free (link->available_services[i]);
	    link->available_services[i] = NULL;
	}
    }

    free (link->mq_up_name);
    free (link->mq_down_name);

    free (link);
}
