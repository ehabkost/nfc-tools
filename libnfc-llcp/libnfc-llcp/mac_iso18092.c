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
#include <pthread.h>
#if defined(HAVE_PTHREAD_NP_H)
#  include <pthread_np.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

int		 mac_link_run (struct mac_link *link);

struct mac_link *
mac_link_new (nfc_device *device, struct llc_link *llc_link)
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
	res->exchange_pdus_thread = NULL;

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
    nfc_target nt;

    /* Try to establish connection as an initiator */
    MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) Attempting to activate LLCP Link as initiator", nfc_device_get_name (mac_link->device));
    if ((res = nfc_initiator_init (mac_link->device)) == 0) {
	MAC_LINK_LOG (LLC_PRIORITY_DEBUG, "(%s) nfc_initiator_init() succeeded", nfc_device_get_name (mac_link->device));

	nfc_dep_info info;
	memcpy (info.abtNFCID3, mac_link->nfcid, sizeof (mac_link->nfcid));
	memcpy (info.abtGB, llcp_magic_number, sizeof (llcp_magic_number));
	info.szGB = sizeof (llcp_magic_number);
#if 0
	info.btDID = 0x00;
	info.btBS = 0x0f;
	info.btBR = 0x0f;
	info.btTO = 0;
	info.btPP = 0x32;
	info.ndm = NDM_PASSIVE;
#endif

	if ((res = nfc_initiator_poll_dep_target (mac_link->device, NDM_PASSIVE, NBR_424, &info, &nt, 10000)) > 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_DEBUG, "(%s) nfc_initiator_poll_dep_target() succeeded", nfc_device_get_name (mac_link->device));

	    if ((nt.nti.ndi.szGB >= sizeof (llcp_magic_number)) &&
		(0 == memcmp (nt.nti.ndi.abtGB, llcp_magic_number, sizeof (llcp_magic_number)))) {
		MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) LLCP Link activated (initiator)", nfc_device_get_name (mac_link->device));

		mac_link->mode = MAC_LINK_INITIATOR;
		if (llc_link_activate (mac_link->llc_link, LLC_INITIATOR | LLC_PAX_PDU_PROHIBITED, NULL, 0) < 0) {
		    MAC_LINK_MSG (LLC_PRIORITY_FATAL, "Error activating LLC Link");
		    res = -1;
		} else {
		    res = mac_link_run (mac_link);
		}
	    }
	} else if ((res == 0) || (res == NFC_ETIMEOUT)) {
	    MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) No DEP target available.", nfc_device_get_name (mac_link->device));
	    res = -1;
	} else { 
	    MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) nfc_initiator_poll_dep_target() failed", nfc_device_get_name (mac_link->device));
	    nfc_perror (mac_link->device, "REASON");
	    res = -1;
	}
    } else {
	MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) nfc_initiator_init() failed", nfc_device_get_name (mac_link->device));
	nfc_perror (mac_link->device, "REASON");
	return -1;
    }

    return res;
}

int
mac_link_activate_as_target (struct mac_link *mac_link)
{
    assert (mac_link);

    nfc_target nt;

    uint8_t params[BUFSIZ];
    size_t params_len = llc_link_encode_parameters (mac_link->llc_link, params, sizeof (params));

    /* Wait as a target for a device to establish a connection */
    nt.nm.nmt = NMT_DEP;
    nt.nm.nbr = NBR_UNDEFINED;
    nt.nti.ndi.btPP = 0x32;
    nt.nti.ndi.ndm  = NDM_PASSIVE;

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

    int res = 0;
    uint8_t data[BUFSIZ];

    MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) Attempting to activate LLCP Link as target (blocking)", nfc_device_get_name (mac_link->device));
    if ((res = nfc_target_init (mac_link->device, &nt, data, sizeof(data), 5000)) >= 0) {
	MAC_LINK_LOG (LLC_PRIORITY_INFO, "(%s) LLCP Link activated (target)", nfc_device_get_name (mac_link->device));
	mac_link->mode = MAC_LINK_TARGET;
	if (res < 20) {
	    MAC_LINK_MSG (LLC_PRIORITY_ERROR, "Frame too short");
	    res = -1;
	} else if (memcmp (data + 17, llcp_magic_number, sizeof (llcp_magic_number))) {
	    MAC_LINK_MSG (LLC_PRIORITY_ERROR, "LLCP Magic Number not found");
	    res = -1;
	} else if (llc_link_activate (mac_link->llc_link, LLC_TARGET | LLC_PAX_PDU_PROHIBITED, data + 20, res - 20) < 0) {
	    MAC_LINK_MSG (LLC_PRIORITY_FATAL, "Error activating LLC Link");
	    res = -1;
	} else {
	    res = mac_link_run (mac_link);
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

    if (link->mode == MAC_LINK_INITIATOR) {
	/* Bootstrap the LLC communication sending a SYMM PDU */
	uint8_t symm[2] = { 0x00, 0x00 };
	if (pdu_send (link, symm, sizeof (symm)) < 0)
	    return NULL;
    }

    uint8_t buffer[BUFSIZ];
    for (;;) {
	ssize_t len = pdu_receive (link, buffer, sizeof (buffer));
	if (len < 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_WARN, "pdu_receive returned %d", len);
	    break;
	}
	MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Received %d PDU bytes", (int) len);

	if (LL_ACTIVATED == link->llc_link->status) {
	if (mq_send (link->llc_link->llc_up, (char *) buffer, len, 0) < 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_FATAL, "Can't send data to LLC Link: %s", strerror (errno));
	    break;
	}
	}

	struct timespec ts;
	ts.tv_sec = link->llc_link->local_lto.tv_sec;
	ts.tv_nsec = link->llc_link->local_lto.tv_usec * 1000;

	/* Wait LTO - 2ms */
	if (ts.tv_nsec < 2000000) {
	    ts.tv_sec -= 1;
	    ts.tv_nsec = ts.tv_nsec + 1000000000 - 2000000;
	} else {
	    ts.tv_nsec -= 2000000;
	}

	len = mq_timedreceive (link->llc_link->llc_down, (char *) buffer, sizeof (buffer), NULL, &ts);

	if (len < 0) {
	    switch (errno) {
	    case ETIMEDOUT:
		buffer[0] = buffer[1] = 0x00;
		len = 2;
		break;
	    default:
		break;
	    }
	}
	if (len < 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_FATAL, "Can't receive data from LLC Link: %s", strerror (errno));
	    break;
	}

	MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Sending %d bytes", len);
	if ((len = pdu_send (link, buffer, len)) < 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_WARN, "pdu_send returned %d", len);
	    break;
	}
    }

    link->exchange_pdus_thread = NULL;

    MAC_LINK_LOG (LLC_PRIORITY_ERROR, "NFC error: %s", nfc_strerror (link->device));
    switch (nfc_device_get_last_error (link->device)) {
    case NFC_ETGRELEASED:
	/* The initiator has left the target's field */
	return (void *) MAC_DEACTIVATE_ON_FAILURE;
	break;
    default:
	return (void *) MAC_DEACTIVATE_ON_REQUEST;
	break;
    }
}

void *
mac_link_drain (void *arg)
{
    struct mac_link *link = (struct mac_link *)arg;

    uint8_t buffer[BUFSIZ];
    uint8_t sym_pdu[] = { 0x00, 0x00 };

    for (;;) {
	ssize_t len = pdu_receive (link, buffer, sizeof (buffer));
	if (len < 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_WARN, "pdu_receive returned %d (drain)", len);
	    break;
	}
	MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Received %d bytes (drain)", (int) len);

	MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Sending %d bytes (drain)", sizeof (sym_pdu));
	if ((len = pdu_send (link, sym_pdu, sizeof (sym_pdu))) < 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_WARN, "pdu_send returned %d (drain)", len);
	    break;
	}
    }

    return NULL;
}

int
mac_link_run (struct mac_link *link)
{
    assert (link);

    if ((link->exchange_pdus_thread = malloc (sizeof (pthread_t))) == NULL) {
	MAC_LINK_MSG (LLC_PRIORITY_FATAL, "Cannot allocate PDU exchanging thread structure");
	return -1;
    }

    if (pthread_create (link->exchange_pdus_thread, NULL, mac_link_exchange_pdus, (void *) link) < 0) {
	MAC_LINK_MSG (LLC_PRIORITY_FATAL, "Cannot create PDU exchanging thread");
	return -1;
    }
#if defined(HAVE_DECL_PTHREAD_SET_NAME_NP) && HAVE_DECL_PTHREAD_SET_NAME_NP
    pthread_set_name_np (*link->exchange_pdus_thread, "MAC Link");
#endif

    return 1;
}

int
mac_link_wait (struct mac_link *link, void **value_ptr)
{
    assert (link);
    assert (value_ptr);

    *value_ptr = NULL;

    MAC_LINK_MSG (LLC_PRIORITY_TRACE, "Waiting for MAC Link PDU exchange thread to exit");
    int res = pthread_join (*link->exchange_pdus_thread, value_ptr);
    MAC_LINK_LOG (LLC_PRIORITY_TRACE, "MAC Link exchange PDU exchange thread terminated (returned %x)", *value_ptr);

    return res;
}

int
mac_link_deactivate (struct mac_link *link, intptr_t reason)
{
    assert (link);
    assert (*link->exchange_pdus_thread != pthread_self ());

    MAC_LINK_LOG (LLC_PRIORITY_INFO, "MAC Link deactivation requested (reason: %d)", reason);

    if (!link->exchange_pdus_thread) {
	MAC_LINK_MSG (LLC_PRIORITY_WARN, "MAC Link already stopped");
	return 0;
    }

    llcp_threadslayer (*link->exchange_pdus_thread);
    link->exchange_pdus_thread = NULL;


    bool st;
    if (link->mode == MAC_LINK_INITIATOR) {
	switch (reason) {
	case MAC_DEACTIVATE_ON_REQUEST:
	    st = nfc_initiator_deselect_target (link->device);
	    break;
	case MAC_DEACTIVATE_ON_FAILURE:
	    /*
	     * If a failure already occured, the DEP connection is alreday
	     * broken so sending a deselect request would fail.
	     */
	    st = true;
	    break;
	default:
	    abort ();
	    break;
	}
    } else {
	switch (reason) {
	case MAC_DEACTIVATE_ON_REQUEST:
	    MAC_LINK_MSG (LLC_PRIORITY_INFO, "Drain mode");
	    st = 0 == pthread_create (link->exchange_pdus_thread, NULL, mac_link_drain, link);
	    pthread_join (*link->exchange_pdus_thread, NULL);
	    link->exchange_pdus_thread = NULL;
	    break;
	case MAC_DEACTIVATE_ON_FAILURE:
	    st = true;
	    break;
	default:
	    abort ();
	    break;
	}
    }

    if (st) {
	MAC_LINK_MSG (LLC_PRIORITY_INFO, "MAC Link deactivated");
	return 0;
    } else {
	MAC_LINK_MSG (LLC_PRIORITY_ERROR, "MAC Link deactivation failed");
	return -1;
    }
}

int timeval_to_ms (const struct timeval tv)
{
  return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

ssize_t
pdu_send (struct mac_link *link, const void *buf, size_t nbytes)
{
    ssize_t res = -1;
    int oldstate;
    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &oldstate);

    MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Sending %d bytes", nbytes);
    if (link->mode == MAC_LINK_INITIATOR) {
	link->buffer_size = sizeof (link->buffer);
	MAC_LINK_LOG (LLC_PRIORITY_TRACE, "LTOs: %d ms (local), %d ms (remote)", timeval_to_ms (link->llc_link->local_lto), timeval_to_ms (link->llc_link->remote_lto));
	const int timeout = timeval_to_ms (link->llc_link->local_lto) + timeval_to_ms (link->llc_link->remote_lto);
	res = nfc_initiator_transceive_bytes (link->device, buf, nbytes, link->buffer, &link->buffer_size, timeout);
    } else {
	res = nfc_target_send_bytes (link->device, buf, nbytes, -1);
    }

    if (res < 0)
	MAC_LINK_LOG (LLC_PRIORITY_FATAL, "Could not send %d bytes", nbytes);
    pthread_setcancelstate (oldstate, NULL);

    return res;
}

ssize_t
pdu_receive (struct mac_link *link, void *buf, size_t nbytes)
{
    ssize_t res;
    int timeout = timeval_to_ms (link->llc_link->local_lto);

    if (link->mode == MAC_LINK_INITIATOR) {
	res = MIN (nbytes, link->buffer_size);
	MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Received %d bytes (Requested %d, buffer size %d)", res, nbytes, link->buffer_size);
	memcpy (buf, link->buffer, res);
	return res;
    } else {
	if ((res = nfc_target_receive_bytes (link->device, buf, nbytes, timeout + 2000)) >= 0) {
	    MAC_LINK_LOG (LLC_PRIORITY_TRACE, "Received %d bytes", res);
	    return res;
	}
    }
    MAC_LINK_LOG (LLC_PRIORITY_FATAL, "MAC Level error on PDU reception (%d)", res);
    return -1;
}

void
mac_link_free (struct mac_link *mac_link)
{
    if (mac_link->exchange_pdus_thread)
	free (mac_link->exchange_pdus_thread);

    if (mac_link && mac_link->llc_link)
	mac_link->llc_link->mac_link = NULL;
    free (mac_link);
}
