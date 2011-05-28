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

#ifndef _LLC_SERVICE_H
#define _LLC_SERVICE_H

#include <pthread.h>
#include <mqueue.h>

struct llc_service {
    char *urn;

    pthread_t thread;
    void *(*thread_routine)(void *);
    mqd_t llc_up;
    mqd_t llc_down;
    char *mq_up_name;
    char *mq_down_name;

    /* Unit tests metadata */
    void *cut_test_context;
};

int		 llc_service_new (struct llc_link *link, uint8_t service, void *(*thread_routine)(void *));
int		 llc_service_start(struct llc_link *link, uint8_t service);
void		 llc_service_stop (struct llc_link *link, uint8_t service);
void		 llc_service_free (struct llc_link *link, uint8_t service);

#endif /* !_LLC_SERVICE_H */
