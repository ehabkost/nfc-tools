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
#include <stdint.h>

struct llc_service {
    char *uri;
    void *(*accept_routine)(void *);
    void *(*thread_routine)(void *);
    int8_t sap;
    uint16_t miu;
};

struct llc_service *llc_service_new (void *(*accept_routine)(void *), void *(*thread_routine)(void *));
struct llc_service *llc_service_new_with_uri (void *(*accept_routine)(void *), void *(*thread_routine)(void *), char *uri);
uint16_t	 llc_service_get_miu (const struct llc_service *service);
void		 llc_service_set_miu (struct llc_service *service, uint16_t miu);
const char	*llc_service_get_uri (const struct llc_service *service);
const char	*llc_service_set_uri (struct llc_service *service, const char *uri);
int		 llc_service_wait (struct llc_service *service, void **value_ptr);
void		 llc_service_free (struct llc_service *service);

#endif /* !_LLC_SERVICE_H */
