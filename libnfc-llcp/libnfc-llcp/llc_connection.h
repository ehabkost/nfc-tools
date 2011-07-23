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

#ifndef _LLC_CONNECTION_H
#define _LLC_CONNECTION_H

#include "llcp_pdu.h"
#include "llc_service.h"

struct llc_connection {
    uint8_t sap;
    uint8_t dsap;
    uint8_t ssap;
    enum {
	DLC_CONNECTION_REQUESTED,
	DLC_CONNECTED,
	DLC_DISCONNECTED
    } status;
    pthread_t thread;
    char *mq_up_name;
    char *mq_down_name;
    mqd_t llc_up;
    mqd_t llc_down;
    struct {
	uint8_t s;	    /* Send State Variable */
	uint8_t sa;	    /* Send Acknowledgement State Variable */
	uint8_t r;	    /* Receive State Variable */
	uint8_t ra;	    /* Receive Acknowledgement State Variable */
    } state;
    uint8_t miu;    /* Maximum Information Unit Size for I PDUs */
    uint8_t rwl;    /* Local Receive Window Size */
    uint8_t rwr;    /* Remote Receive Window Size */
    struct llc_link *link;
};

struct llc_connection *llc_data_link_connection_new (struct llc_link *link, const struct pdu *pdu, int *reason);
struct llc_connection *llc_logical_data_link_new (struct llc_link *link, const struct pdu *pdu);
void		 llc_connection_accept (struct llc_connection *connection);
void		 llc_connection_reject (struct llc_connection *connection);
int		 llc_connection_stop (struct llc_connection *connection);
void		 llc_connection_free (struct llc_connection *connection);

#endif /* !_LLC_CONNECTION_H */
