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

#include <assert.h>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "llcp.h"
#include "llc_link.h"
#include "llcp_log.h"
#include "llcp_pdu.h"
#include "llc_service.h"

#define LOG_LLC_SERVICE "libnfc-llcp.llc.service"
#define LLC_SERVICE_MSG(priority, message) llcp_log_log (LOG_LLC_SERVICE, priority, "%s", message)
#define LLC_SERVICE_LOG(priority, format, ...) llcp_log_log (LOG_LLC_SERVICE, priority, format, __VA_ARGS__)

struct llc_service *
llc_service_new (void *(*accept_routine)(void *), void *(*thread_routine)(void *), void *user_data)
{
    return llc_service_new_with_uri (accept_routine, thread_routine, NULL, user_data);
}

struct llc_service *
llc_service_new_with_uri (void *(*accept_routine)(void *), void *(*thread_routine)(void *), char *uri, void *user_data)
{
    assert (thread_routine);

    struct llc_service *service;

    if ((service = malloc (sizeof (*service)))) {

	service->uri = (uri) ? strdup (uri) : NULL;

	service->accept_routine = accept_routine;
	service->thread_routine = thread_routine;
	service->miu = LLCP_DEFAULT_MIU;
	service->user_data = user_data;
    }

    return service;
}

uint16_t
llc_service_get_miu (const struct llc_service *service)
{
    assert (service);
    return service->miu;
}

void
llc_service_set_miu (struct llc_service *service, uint16_t miu)
{
    assert (service);
    assert (((miu - 128) & 0xfc00) == 0x0000);
    service->miu = miu;
}

uint8_t
llc_service_get_rw (const struct llc_service *service)
{
    return service->rw;
}

void
llc_service_set_rw (struct llc_service *service, uint8_t rw)
{
    service->rw = rw;
}

const char *
llc_service_get_uri (const struct llc_service *service)
{
    return service->uri;
}

const char *
llc_service_set_uri (struct llc_service *service, const char *uri)
{
    assert (service);
    free (service->uri);
    return service->uri = (uri) ? strdup (uri) : NULL;
}

void
llc_service_free (struct llc_service *service)
{
    assert (service);

    free (service->uri);
    free (service);
}
