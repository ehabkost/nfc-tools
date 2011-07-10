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

struct pdu {
    uint8_t ptype;

    // Address fields
    uint8_t dsap;
    uint8_t ssap;

    // Sequence field
    uint8_t send_sequence_number;
    uint8_t receive_sequence_number;
    uint8_t n_s, n_r;

    // Information filed
    size_t information_size;
    uint8_t *information;
};

int		 pdu_pack (const struct pdu *pdu, uint8_t *buffer, size_t len);
struct pdu	*pdu_unpack (const uint8_t *buffer, size_t len);
int		 pdu_size (struct pdu *pdu);
struct pdu	*pdu_aggregate (struct pdu **pdus);
struct pdu     **pdu_dispatch (struct pdu *pdu);
void		 pdu_free (struct pdu *pdu);

#endif /* !_LLCP_PDU_H */
