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

#ifndef _LLCP_PDU_H
#define _LLCP_PDU_H

#include <sys/types.h>

#include <stdint.h>

struct llc_connection;

#define PDU_SYMM    0x0
#define PDU_PAX	    0x1
#define PDU_AGF     0x2
#define PDU_UI      0x3
#define PDU_CONNECT 0x4
#define PDU_DISC    0x5
#define PDU_CC      0x6
#define PDU_DM      0x7
#define PDU_FRMR    0x8
#define PDU_I       0xC
#define PDU_RR      0xD
#define PDU_RNR     0xE
/* LLCP 1.1 PDU */
#define PDU_SNL     0x9

#define FRMR_W 0x80
#define FRMR_I 0x40
#define FRMR_R 0x20
#define FRMR_S 0x10

struct pdu {
    uint8_t ptype;

    // Address fields
    uint8_t dsap;
    uint8_t ssap;

    // Sequence field
    uint8_t nr;
    uint8_t ns;

    // Information filed
    size_t information_size;
    uint8_t *information;
};

int		 pdu_has_sequence_field(const struct pdu *pdu);
struct pdu	*pdu_new (uint8_t dsap, uint8_t ptype, uint8_t ssap, uint8_t nr, uint8_t ns, const uint8_t *information, size_t information_size);
struct pdu	*pdu_new_frmr (uint8_t dsap, uint8_t ssap, struct pdu *pdu, struct llc_connection *connection, int reason);
int		 pdu_pack (const struct pdu *pdu, uint8_t *buffer, size_t len);
struct pdu	*pdu_unpack (const uint8_t *buffer, size_t len);
int		 pdu_size (struct pdu *pdu);
struct pdu	*pdu_aggregate (struct pdu **pdus);
struct pdu     **pdu_dispatch (struct pdu *pdu);
void		 pdu_free (struct pdu *pdu);

#define pdu_new_cc(dsap, ssap) pdu_new (dsap, PDU_CC, ssap, 0, 0, NULL, 0)
#define pdu_new_i(dsap, ssap, conn, info, len) pdu_new (dsap, PDU_I, ssap, conn->state.r, conn->state.s, info, len)
//#define pdu_new_rr(dsap, ssap, conn) pdu_new (dsap, PDU_RR, ssap, conn->state.r, conn->state.s, NULL, 0)
#define pdu_new_rr(conn) pdu_new (conn->ssap, PDU_RR, conn->dsap, conn->state.r, conn->state.s, NULL, 0)
#define pdu_new_rnr(conn) pdu_new (conn->ssap, PDU_RNR, conn->dsap, conn->state.r, conn->state.s, NULL, 0)
#define pdu_new_dm(dsap, ssap, reason) pdu_new (dsap, PDU_DM, ssap, 0, 0, reason, 1)
#define pdu_new_ui(dsap, ssap, info, len) pdu_new (dsap, PDU_I, ssap, 0, 0, info, len)

#endif /* !_LLCP_PDU_H */
