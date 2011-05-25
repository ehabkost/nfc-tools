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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "llcp.h"
#include "llcp_log.h"
#include "llc_service.h"

#define LOG_LLC_SERVICE "libnfc-llcp.llc.service"
#define LLC_SERVICE_MSG(priority, message) llcp_log_log (LOG_LLC_SERVICE, priority, "%s", message)
#define LLC_SERVICE_LOG(priority, format, ...) llcp_log_log (LOG_LLC_SERVICE, priority, format, __VA_ARGS__)

static char *mq_fmt = "/libnfc-llcp-%d-%x-service-%d-%s";

static const char *mq_dirction_up = "up";
static const char *mq_dirction_down = "down";

static inline void
mq_name (char **name, const struct llc_link *link, uint8_t service, const char *direction)
{
    asprintf (name, mq_fmt, getpid(), link, service, direction);
}

int
llc_service_new (struct llc_link *link, uint8_t service, void *(*thread_routine)(void *))
{
    assert (link);
    assert (service <= MAX_LLC_LINK_SERVICE);
    assert (thread_routine);

    if (!(link->services[service] = malloc (sizeof (*link->services[service])))) {
	return -1;
    }

    link->services[service]->urn = NULL;

    link->services[service]->thread_routine = thread_routine;
    link->services[service]->thread = NULL;

    link->services[service]->llc_up = (mqd_t)-1;
    link->services[service]->llc_down = (mqd_t)-1;

    mq_name (&link->services[service]->mq_up_name, link, service, mq_dirction_up);
    mq_name (&link->services[service]->mq_down_name, link, service, mq_dirction_down);

    LLC_SERVICE_LOG (LLC_PRIORITY_DEBUG, "mq_open (%s)", link->services[service]->mq_up_name);
    link->services[service]->llc_up   = mq_open (link->services[service]->mq_up_name, O_CREAT | O_EXCL | O_RDONLY, 0666, NULL);
    if (link->services[service]->llc_up == (mqd_t)-1) {
	LLC_SERVICE_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->services[service]->mq_up_name);
	return -1;
    }

    LLC_SERVICE_LOG (LLC_PRIORITY_DEBUG, "mq_open (%s)", link->services[service]->mq_down_name);
    link->services[service]->llc_down = mq_open (link->services[service]->mq_down_name, O_CREAT | O_EXCL | O_WRONLY | O_NONBLOCK, 0666, NULL);
    if (link->services[service]->llc_down == (mqd_t)-1) {
	LLC_SERVICE_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->services[service]->mq_down_name);
	return -1;
    }

    return 0;
}

int
llc_service_start (struct llc_link *link, uint8_t service)
{
    assert (link);
    assert (service <= MAX_LLC_LINK_SERVICE);
    assert (link->services[service]);

    return pthread_create (&link->services[service]->thread, NULL, link->services[service]->thread_routine, link->services[service]);
}

void
llc_service_stop (struct llc_link *link, uint8_t service)
{
    assert (link);
    assert (service <= MAX_LLC_LINK_SERVICE);
    assert (link->services[service]);

    if (link->services[service]->thread) {
	/* A thread SHALL not be canceled while logging */
	sem_wait (log_sem);
	pthread_cancel (link->services[service]->thread);
	sem_post (log_sem);
	pthread_join (link->services[service]->thread, NULL);
	link->services[service]->thread = NULL;
    }
}

void
llc_service_free (struct llc_link *link, uint8_t service)
{
    assert (link);
    assert (service <= MAX_LLC_LINK_SERVICE);

    free (link->services[service]->urn);

    if (link->services[service]->llc_up != (mqd_t)-1)
	mq_close (link->services[service]->llc_up);
    if (link->services[service]->llc_down != (mqd_t)-1)
	mq_close (link->services[service]->llc_down);

    mq_unlink (link->services[service]->mq_up_name);
    mq_unlink (link->services[service]->mq_down_name);

    free (link->services[service]->mq_up_name);
    free (link->services[service]->mq_down_name);

    free (link->services[service]);
}
