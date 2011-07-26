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
#include <pthread.h>
#if defined(HAVE_PTHREAD_NP_H)
#  include <pthread_np.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <nfc/nfc.h>

#include "llcp.h"
#include "llcp_log.h"
#include "llc_service.h"
#include "llc_link.h"
#include "mac.h"

#define LOG_MAC_LINK "libnfc-llcp.mac.link"
#define MAC_LINK_MSG(priority, message) llcp_log_log (LOG_MAC_LINK, priority, "%s", message)
#define MAC_LINK_LOG(priority, format, ...) llcp_log_log (LOG_MAC_LINK, priority, format, __VA_ARGS__)

static uint8_t llcp_magic_number[] = { 0x46, 0x66, 0x6D };

static uint8_t defaultid[10] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

struct mac_link *
mac_link_new (nfc_device_t *device, struct llc_link *llc_link)
{
    assert (device);
    assert (llc_link);
    assert (!llc_link->mac_link);

    struct mac_link *res;

    if ((res = malloc (sizeof (*res)))) {
	res->mode = MAC_LINK_UNSET;
	res->device = device;
	res->llc_link = llc_link;
	res->llc_link->mac_link = res;

	memcpy (res->nfcid, defaultid, sizeof (defaultid));
    }

    return res;
}

int
mac_link_activate (struct mac_link *mac_link)
{
    int res;

    MAC_LINK_MSG (LLC_PRIORITY_ALERT, "mac_link_activate() will behave inconsistently");

    res = mac_link_activate_as_initiator (mac_link);
    if (res != 0)
	return res;

    res = mac_link_activate_as_target (mac_link);
    if (res != 0)
	return res;

    return -1;
}

int
mac_link_activate_as_initiator (struct mac_link *mac_link)
{
    assert (mac_link);

    int res = 0;
    nfc_target_t nt;

    /* Try to establish connection as an initiator */
    MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) Attempting to activate LLCP Link as initiator", mac_link->device->acName);
    if (nfc_initiator_init (mac_link->device)) {
	MAC_LINK_LOG (LLC_PRIORITY_DEBUG, "(%s) nfc_initiator_init() succeeded", mac_link->device->acName);
	if (nfc_initiator_select_dep_target (mac_link->device, NDM_PASSIVE, NBR_424, NULL, &nt)) {
	    MAC_LINK_LOG (LLC_PRIORITY_DEBUG, "(%s) nfc_initiator_select_dep_target() succeeded", mac_link->device->acName);
	    if ((nt.nti.ndi.szGB >= sizeof (llcp_magic_number)) &&
		(0 == memcmp (nt.nti.ndi.abtGB, llcp_magic_number, sizeof (llcp_magic_number)))) {
		MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) LLCP Link activated (initiator)", mac_link->device->acName);

		mac_link->mode = MAC_LINK_INITIATOR;
		res = 1;
		if (llc_link_activate (mac_link->llc_link, LLC_INITIATOR | LLC_PAX_PDU_PROHIBITED, NULL, 0) < 0) {
		    MAC_LINK_MSG (LLC_PRIORITY_FATAL, "Error activating LLC Link");
		    res = -1;
		}
	    }
	} else {
	    MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) nfc_initiator_select_dep_target() failed", mac_link->device->acName);
	    nfc_perror (mac_link->device, "REASON");
	    res = -1;
	}
    }

    return res;
}

int
mac_link_activate_as_target (struct mac_link *mac_link)
{
    assert (mac_link);

    nfc_target_t nt;
    int res = 0;

    uint8_t params[BUFSIZ];
    size_t params_len = llc_link_encode_parameters (mac_link->llc_link, params, sizeof (params));

    /* Wait as a target for a device to establish a connection */
    nt.nm.nmt = NMT_DEP;
    nt.nm.nbr = NBR_UNDEFINED;
    nt.nti.ndi.btPP = 0x32;
    nt.nti.ndi.ndm  = NDM_UNDEFINED;

#if 1
    /* Not used */
    nt.nti.ndi.btDID = 0x00;
    nt.nti.ndi.btBS  = 0x00;
    nt.nti.ndi.btBR  = 0x00;
    nt.nti.ndi.btTO  = 0x00;
#endif

    memcpy (nt.nti.ndi.abtNFCID3, mac_link->nfcid, sizeof (mac_link->nfcid));
    memcpy (nt.nti.ndi.abtGB, llcp_magic_number, sizeof (llcp_magic_number));
    memcpy (nt.nti.ndi.abtGB + sizeof (llcp_magic_number), params, params_len);
    nt.nti.ndi.szGB = sizeof (llcp_magic_number) + params_len;

    size_t n;
    uint8_t data[BUFSIZ];

    MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) Attempting to activate LLCP Link as target (blocking)", mac_link->device->acName);
    if (nfc_target_init (mac_link->device, &nt, data, &n)) {
	MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) LLCP Link activated (target)", mac_link->device->acName);
	mac_link->mode = MAC_LINK_TARGET;
	res = 1;
	if (llc_link_activate (mac_link->llc_link, LLC_TARGET | LLC_PAX_PDU_PROHIBITED, data + 20, n - 20) < 0) {
	    MAC_LINK_MSG (LLC_PRIORITY_FATAL, "Error activating LLC Link");
	    res = -1;
	}
    } else {
	MAC_LINK_MSG (LLC_PRIORITY_ERROR, "Cannot establish LLCP Link");
	res = -1;
    }

    return res;
}

void *
mac_link_exchange_pdus (void *arg)
{
    struct mac_link *link = (struct mac_link *)arg;

    uint8_t buffer[BUFSIZ];
    for (;;) {
	ssize_t len = pdu_receive (link, buffer, sizeof (buffer));
	if (len < 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_WARN, "pdu_receive returned %d", len);
	    break;
	}
	MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Received %d bytes", (int) len);

	mq_send (link->llc_link->llc_up, (char *) buffer, len, 0);

	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000000;
	ts.tv_nsec = 10000000;
	nanosleep (&ts, NULL); // TODO Adjust me

	len = mq_receive (link->llc_link->llc_down, (char *) buffer, sizeof (buffer), NULL);

	MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Sending %d bytes", len);
	if ((len = pdu_send (link, buffer, len)) < 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_WARN, "pdu_send returned %d", len);
	    break;
	}
    }

    //llc_link_deactivate (link->llc_link);

    return NULL;
}

int
mac_link_wait (struct mac_link *link, void **value_ptr)
{
    assert (link);
    assert (value_ptr);

    if (pthread_create (&link->exchange_pdus_thread, NULL, mac_link_exchange_pdus, (void *) link) < 0) {
	MAC_LINK_MSG (LLC_PRIORITY_FATAL, "Cannot create PDU exchanging thread");
	return -1;
    }
#if defined(HAVE_DECL_PTHREAD_SET_NAME_NP) && HAVE_DECL_PTHREAD_SET_NAME_NP
    pthread_set_name_np (link->exchange_pdus_thread, "MAC Link");
#endif

    MAC_LINK_MSG (LLC_PRIORITY_TRACE, "Waiting for MAC Link deactivation");
    int res = pthread_join (link->exchange_pdus_thread, value_ptr);
    MAC_LINK_MSG (LLC_PRIORITY_TRACE, "MAC Link deactivated");

    llc_link_deactivate (link->llc_link);

    return res;
}

int
mac_link_deactivate (struct mac_link *link)
{
    uint8_t dsl_req[] = { 0xD4, 0x08 };
    uint8_t dsl_res[] = { 0xD5, 0x09 };

    uint8_t res[3];
    size_t res_len = sizeof (res);

    MAC_LINK_MSG (LLC_PRIORITY_INFO, "MAC Link deactivation requested");

    bool st;
    if (link->mode == MAC_LINK_INITIATOR) {
	st = nfc_initiator_transceive_bytes (link->device, dsl_req, sizeof (dsl_req), res, &res_len);
    } else {
	st = nfc_target_send_bytes (link->device, dsl_res, sizeof (dsl_res));
    }

    if (st) {
	MAC_LINK_MSG (LLC_PRIORITY_INFO, "MAC Link deactivated");
	return 0;
    } else {
	MAC_LINK_MSG (LLC_PRIORITY_ERROR, "MAC Link deactivation failed");
	return -1;
    }
}

ssize_t
pdu_send (struct mac_link *link, const void *buf, size_t nbytes)
{
    MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Sending %d bytes", nbytes);
    if (link->mode == MAC_LINK_INITIATOR) {
	link->buffer_size = sizeof (link->buffer);
	if (nfc_initiator_transceive_bytes (link->device, buf, nbytes, link->buffer, &link->buffer_size))
	    return nbytes;
    } else {
	if (nfc_target_send_bytes (link->device, buf, nbytes))
	    return nbytes;
    }
    MAC_LINK_LOG (LLC_PRIORITY_FATAL, "Could not send %d bytes", nbytes);
    return -1;
}

ssize_t
pdu_receive (struct mac_link *link, void *buf, size_t nbytes)
{
    ssize_t res;
    if (link->mode == MAC_LINK_INITIATOR) {
	res = MIN (nbytes, link->buffer_size);
	MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Received %d bytes (Requested %d, buffer size %d)", res, nbytes, link->buffer_size);
	memcpy (buf, link->buffer, res);
	return res;
    } else {
	if (nfc_target_receive_bytes (link->device, buf, &nbytes)) {
	    MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Received %d bytes", nbytes);
	    return nbytes;
	}
    }
    MAC_LINK_LOG (LLC_PRIORITY_FATAL, "MAC Level error on PDU reception", nbytes);
    return -1;
}

void
mac_link_free (struct mac_link *mac_link)
{
    if (mac_link && mac_link->llc_link)
	mac_link->llc_link->mac_link = NULL;
    free (mac_link);
}
