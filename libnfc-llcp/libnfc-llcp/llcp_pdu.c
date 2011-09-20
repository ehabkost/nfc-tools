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

#include <stdlib.h>
#include <string.h>

#include "llc_connection.h"
#include "llcp_log.h"
#include "llcp_parameters.h"
#include "llcp_pdu.h"

#define LOG_LLC_PDU "libnfc-llcp.llc.pdu"
#define LLC_PDU_MSG(priority, message) llcp_log_log (LOG_LLC_PDU, priority, "%s", message)
#define LLC_PDU_LOG(priority, format, ...) llcp_log_log (LOG_LLC_PDU, priority, format, __VA_ARGS__)

uint8_t _pdu_ptype_sequence_field[] = {
    0, /* PDU_SYMM */
    0, /* PDU_PAX */
    0, /* PDU_AGF */
    0, /* PDU_UI */
    0, /* PDU_CONNECT */
    0, /* PDU_DISC */
    0, /* PDU_CC */
    0, /* PDU_DM */
    0, /* PDU_FRMR */
    0, /* PDU_SNL */
    0, /* reserved */
    0, /* reserved */
    1, /* PDU_I */
    1, /* PDU_RR */
    1, /* PDU_RNR */
    0  /* reserved */
};

int
pdu_has_sequence_field(const struct pdu *pdu)
{
    return _pdu_ptype_sequence_field[pdu->ptype];
}

static inline uint8_t *
memdup (const uint8_t *mem, size_t len)
{
    uint8_t *res = NULL;

    if (mem && (res = malloc (len))) {
	memcpy (res, mem, len);
    }

    return res;
}

struct pdu *
pdu_new (uint8_t dsap, uint8_t ptype, uint8_t ssap, uint8_t nr, uint8_t ns, const uint8_t *information, size_t information_size)
{
    struct pdu *pdu;

    if ((pdu = malloc (sizeof (*pdu)))) {
	pdu->dsap  = dsap;
	pdu->ptype = ptype;
	pdu->ssap  = ssap;

	pdu->nr = nr;
	pdu->ns = ns;

	pdu->information_size = information_size;
	pdu->information = memdup (information, information_size);
    }

    return pdu;
}

struct pdu *
pdu_new_cc (const struct llc_connection *connection)
{
    struct pdu *res;

    uint8_t buffer[BUFSIZ];
    int len = 0, r;

    r = parameter_encode_miux (buffer + len, sizeof (buffer) - len, connection->local_miu);
    if (r >= 0)
	len += r;
    r = parameter_encode_rw (buffer + len, sizeof (buffer) - len, connection->rwl);
    if (r >= 0)
	len += r;

    res = pdu_new (connection->remote_sap, PDU_CC, connection->local_sap, 0, 0, buffer, len);
    return res;
}

struct pdu *
pdu_new_frmr (uint8_t dsap, uint8_t ssap, struct pdu *pdu, struct llc_connection *connection, int reason)
{
    uint8_t info[] = { reason | pdu->ptype, pdu_has_sequence_field(pdu) ? (pdu->nr << 4 | pdu->ns) : 0, connection->state.s << 4 | connection->state.r, connection->state.sa << 4 | connection->state.ra };
    return pdu_new (dsap, PDU_FRMR, ssap, 0, 0, info, sizeof (info));
}

int
pdu_pack (const struct pdu *pdu, uint8_t *buffer, size_t len)
{
    if (len < 2 + (pdu_has_sequence_field(pdu) ? 1 : 0) + pdu->information_size) {
	LLC_PDU_MSG (LLC_PRIORITY_ERROR, "Insuficient buffer space");
	return -1;
    }

    int n = 0;
    buffer[n++] = (pdu->dsap << 2) | (pdu->ptype >> 2);
    buffer[n++] = (pdu->ptype << 6) | (pdu->ssap);

    if (pdu_has_sequence_field (pdu)) {
	buffer[n++] = (pdu->ns << 4) | pdu->nr;
    }

    for (size_t i = 0; i < pdu->information_size; i++)
	buffer[n++] = pdu->information[i];

    return n;
}

struct pdu *
pdu_unpack (const uint8_t *buffer, size_t len)
{
    struct pdu *pdu;

    if ((pdu = malloc (sizeof *pdu))) {
	pdu->dsap = buffer[0] >> 2;
	pdu->ptype = ((buffer[0] & 0x03) << 2) | (buffer[1] >> 6);
	pdu->ssap = buffer[1] & 0x3F;

	int n = 2;

	if (pdu_has_sequence_field (pdu)) {
	    pdu->ns = buffer[n] >> 4;
	    pdu->nr = buffer[n++] & 0x0F;
	}

	pdu->information_size = len - n;
	if (pdu->information_size) {
	    if (!(pdu->information = malloc (pdu->information_size))) {
		free (pdu);
		return NULL;
	    }

	    memcpy (pdu->information, buffer + n, pdu->information_size);
	} else {
	    pdu->information = NULL;
	}
    }

    return pdu;
}

int
pdu_size (struct pdu *pdu)
{
    return 2 + (pdu_has_sequence_field (pdu) ? 1 : 0) + pdu->information_size;
}

struct pdu *
pdu_aggregate (struct pdu **pdus)
{
    struct pdu *res = NULL;
    struct pdu **pdu = pdus;

    size_t len = 0;
    while (*pdu) {
	len += 2 + pdu_size (*pdu);
	pdu++;
    }

    if ((res = malloc (sizeof (*res)))) {
	res->ssap = 0;
	res->dsap = 0;
	res->ptype = PDU_AGF;

	res->information_size = len;
	if (!(res->information = malloc (len))) {
	    free (res);
	    return NULL;
	}

	off_t offset = 0;
	pdu = pdus;
	while (*pdu) {
	    uint16_t size = pdu_size (*pdu);
	    res->information[offset++] = size >> 8;
	    res->information[offset++] = size;
	    offset += pdu_pack (*pdu, res->information + offset, len - offset);
	    pdu++;
	}
    }

    return res;
}

struct pdu **
pdu_dispatch (struct pdu *pdu)
{
    struct pdu **pdus = NULL;

    if ((pdu->ssap != 0) || (pdu->ptype != PDU_AGF) || (pdu->dsap != 0)) {
	LLC_PDU_MSG (LLC_PRIORITY_ERROR, "Invalid AGF PDU");
	return NULL;
    }

    size_t pdu_count = 0;
    size_t offset = 0;
    uint16_t pdu_length;

    while (offset < pdu->information_size) {
	if (offset + 2 > pdu->information_size) {
	    LLC_PDU_MSG (LLC_PRIORITY_ERROR, "Incomplete TLV field");
	    return NULL;
	}

	pdu_length = pdu->information[offset++] << 8;
	pdu_length += pdu->information[offset++];
	offset += pdu_length;
	pdu_count++;
    }

    if (offset != pdu->information_size) {
	LLC_PDU_MSG (LLC_PRIORITY_ERROR, "Unprocessed TLV parameters");
	return NULL;
    }

    pdus = malloc ((pdu_count + 1) * sizeof (*pdus));
    offset = 0;
    pdu_count = 0;

    while (offset < pdu->information_size) {
	pdu_length = pdu->information[offset++] << 8;
	pdu_length += pdu->information[offset++];
	pdus[pdu_count++] = pdu_unpack (pdu->information + offset, pdu_length);
	offset += pdu_length;
    }

    pdus[pdu_count] = NULL;

    return pdus;
}

void
pdu_free (struct pdu *pdu)
{
    free (pdu->information);
    free (pdu);
}
