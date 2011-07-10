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
    char *uri;

    pthread_t thread;
    void *(*thread_routine)(void *);
    void *user_data;
    mqd_t llc_up;
    mqd_t llc_down;
    char *mq_up_name;
    char *mq_down_name;
    int mq_down_flags;
    int8_t sap;

    /* Unit tests metadata */
    void *cut_test_context;
};

struct llc_service *llc_service_new (void *(*thread_routine)(void *));
struct llc_service *llc_service_new_with_uri (void *(*thread_routine)(void *), char *uri);
const char	*llc_service_get_uri (const struct llc_service *service);
const char	*llc_service_set_uri (struct llc_service *service, const char *uri);
void		 llc_service_set_user_data (struct llc_service *service, void *user_data);
void	    	 llc_service_set_mq_down_non_blocking (struct llc_service *service);
int		 llc_service_start(struct llc_service *service);
void		 llc_service_stop (struct llc_service *service);
void		 llc_service_free (struct llc_service *service);

#endif /* !_LLC_SERVICE_H */
