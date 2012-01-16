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

#include <sys/types.h>

#include <mqueue.h>
#include <pthread.h>
#include <stdint.h>

struct pdu;
struct llc_link;

struct llc_connection {
    uint8_t service_sap;
    uint8_t remote_sap;
    uint8_t local_sap;
    char *remote_uri;
    enum {
	DLC_NEW,
	DLC_ACCEPTED,
	DLC_REJECTED,
	DLC_RECEIVED_CC,
	DLC_CONNECTED,
	DLC_DISCONNECTED,
	DLC_TERMINATED
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
    uint16_t local_miu;     /* Maximum Information Unit Size for I PDUs */
    uint16_t remote_miu;    /* Maximum Information Unit Size for I PDUs */
    uint8_t rwl;    /* Local Receive Window Size */
    uint8_t rwr;    /* Remote Receive Window Size */
    struct llc_link *link;
    void *user_data;
};

struct llc_connection *llc_data_link_connection_new (struct llc_link *link, const struct pdu *pdu, int *reason);
struct llc_connection *llc_logical_data_link_new (struct llc_link *link, const struct pdu *pdu);
struct llc_connection *llc_outgoing_data_link_connection_new (struct llc_link *link, uint8_t local_sap, uint8_t remote_sap);
struct llc_connection *llc_outgoing_data_link_connection_new_by_uri (struct llc_link *link, uint8_t local_sap, const char *remote_uri);
int		 llc_connection_connect (struct llc_connection *connection);
void		 llc_connection_accept (struct llc_connection *connection);
void		 llc_connection_reject (struct llc_connection *connection);
int		 llc_connection_send_pdu (struct llc_connection *connection, const struct pdu *pdu);
int		 llc_connection_send (struct llc_connection *connection, const uint8_t *data, size_t len);
int		 llc_connection_recv (struct llc_connection *connection, uint8_t *data, size_t len, uint8_t *ssap);
int		 llc_connection_stop (struct llc_connection *connection);
void		 llc_connection_free (struct llc_connection *connection);

#endif /* !_LLC_CONNECTION_H */
