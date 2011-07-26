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

#ifndef _MAC_H
#define _MAC_H

#include <sys/types.h>

#include <nfc/nfc.h>

struct mac_link {
    enum { MAC_LINK_UNSET, MAC_LINK_INITIATOR, MAC_LINK_TARGET } mode;
    nfc_device_t *device;
    struct llc_link *llc_link;
    uint8_t nfcid[10];
    uint8_t buffer[BUFSIZ];
    size_t buffer_size;
    pthread_t exchange_pdus_thread;
};

struct mac_link	*mac_link_new (nfc_device_t *device, struct llc_link *llc_link);

int		 mac_link_activate (struct mac_link *mac_link);
int		 mac_link_activate_as_initiator (struct mac_link *mac_link);
int		 mac_link_activate_as_target (struct mac_link *mac_link);

ssize_t		 pdu_send (struct mac_link *link, const void *buf, size_t nbytes);
ssize_t		 pdu_receive (struct mac_link *link, void *buf, size_t nbytes);
int		 mac_link_wait (struct mac_link *link, void **value_ptr);
int		 mac_link_deactivate (struct mac_link *link);

void		 mac_link_free (struct mac_link *mac_link);

#endif /* !_MAC_H */
