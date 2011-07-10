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
#include "llcp_log.h"
#include "llc_service.h"

#define LOG_LLC_SERVICE "libnfc-llcp.llc.service"
#define LLC_SERVICE_MSG(priority, message) llcp_log_log (LOG_LLC_SERVICE, priority, "%s", message)
#define LLC_SERVICE_LOG(priority, format, ...) llcp_log_log (LOG_LLC_SERVICE, priority, format, __VA_ARGS__)

static char *mq_fmt = "/libnfc-llcp-%d-%x-%s";

static const char *mq_dirction_up = "up";
static const char *mq_dirction_down = "down";

static inline void
mq_name (char **name, const struct llc_service *service, const char *direction)
{
    asprintf (name, mq_fmt, getpid(), service, direction);
}

struct llc_service *
llc_service_new (void *(*thread_routine)(void *))
{
    assert (thread_routine);

    struct llc_service *service;

    if ((service = malloc (sizeof (*service)))) {

	service->uri = NULL;

	service->thread_routine = thread_routine;
	service->thread = NULL;
	service->user_data = service;

	service->llc_up = (mqd_t)-1;
	service->llc_down = (mqd_t)-1;
	service->mq_down_flags = O_NONBLOCK;

	mq_name (&service->mq_up_name, service, mq_dirction_up);
	mq_name (&service->mq_down_name, service, mq_dirction_down);
    }

    return service;
}

void
llc_service_set_user_data (struct llc_service *service, void *user_data)
{
    assert (service);

    service->user_data = user_data;
}

void
llc_service_set_mq_down_non_blocking (struct llc_service *service)
{
    assert (service);

    service->mq_down_flags = 0;
}

int
llc_service_start (struct llc_service *service)
{
    assert (service);

    LLC_SERVICE_LOG (LLC_PRIORITY_DEBUG, "mq_open (%s)", service->mq_up_name);
    service->llc_up   = mq_open (service->mq_up_name, O_CREAT | O_EXCL | O_WRONLY | O_NONBLOCK, 0666, NULL);
    if (service->llc_up == (mqd_t)-1) {
	LLC_SERVICE_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", service->mq_up_name);
	return -1;
    }

    LLC_SERVICE_LOG (LLC_PRIORITY_DEBUG, "mq_open (%s)", service->mq_down_name);
    service->llc_down = mq_open (service->mq_down_name, O_CREAT | O_EXCL | O_RDONLY | service->mq_down_flags, 0666, NULL);
    if (service->llc_down == (mqd_t)-1) {
	LLC_SERVICE_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", service->mq_down_name);
	return -1;
    }

    return pthread_create (&service->thread, NULL, service->thread_routine, service->user_data);
}

void
llc_service_stop (struct llc_service *service)
{
    assert (service);

    if (service->thread) {
	/* A thread SHALL not be canceled while logging */
	sem_wait (log_sem);
	pthread_cancel (service->thread);

	/*
	 * Send a signal to the thread
	 *
	 * Thread cancellation is not handled by message queue functions so the
	 * only way to unlock a thread blocked on message operations is by
	 * sending a signal to it so that it gets a chance to see the
	 * cancelation state.  However, if send too early in the thread's life,
	 * the signal may be missed.  So loop on pthread_kill() until it fails.
	 *
	 * XXX This is a dirty hack.
	 */
	struct timespec delay = {
	    .tv_sec  = 0,
	    .tv_nsec = 10000000,
	};
	while (0 == pthread_kill (service->thread, SIGUSR1)) {
	    nanosleep (&delay, NULL);
	}
	sem_post (log_sem);
	LLC_SERVICE_LOG (LLC_PRIORITY_DEBUG, "(%p) JOIN", service->thread);
	pthread_join (service->thread, NULL);
	LLC_SERVICE_LOG (LLC_PRIORITY_DEBUG, "(%p) JOINED", service->thread);
	service->thread = NULL;
	mq_close (service->llc_up);
	service->llc_up = (mqd_t) -1;
	mq_close (service->llc_down);
	service->llc_down = (mqd_t) -1;
    }
}

void
llc_service_free (struct llc_service *service)
{
    assert (service);

    free (service->uri);

    if (service->llc_up != (mqd_t)-1)
	mq_close (service->llc_up);
    if (service->llc_down != (mqd_t)-1)
	mq_close (service->llc_down);

    mq_unlink (service->mq_up_name);
    mq_unlink (service->mq_down_name);

    free (service);
}
