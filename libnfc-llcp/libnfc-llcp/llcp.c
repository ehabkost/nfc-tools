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

#include <sys/param.h>
#include <sys/types.h>

#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "llc_link.h"
#include "llcp_log.h"
#include "llcp_pdu.h"
#include "llc_service.h"

#define LOG_LLCP "libnfc-llcp"
#define LLCP_MSG(priority, message) llcp_log_log (LOG_LLCP, priority, "%s", message)
#define LLCP_LOG(priority, format, ...) llcp_log_log (LOG_LLCP, priority, format, __VA_ARGS__)

void
sigusr1_handler (int x)
{
    (void) x;
    printf ("(%p) SIGUSR1\n", (void *)pthread_self ());
}

int
llcp_init (void)
{
    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sa.sa_flags  = 0;
    sigemptyset (&sa.sa_mask);
    if (sigaction (SIGUSR1, &sa, NULL) < 0)
	return -1;

    return llcp_log_init ();
}

int
llcp_fini (void)
{
    return llcp_log_fini ();
}

int
llcp_version_agreement (struct llc_link *link, struct llcp_version version)
{
    int res = -1;

    if (link->version.major == version.major) {
	link->version.minor = MIN (link->version.minor, version.minor);
	res = 0;
    } else if (link->version.major > version.major) {
	if (version.major >= 1) {
	    link->version = version;
	    res = 0;
	}
    } else {
	/* Let the remote LLC component perform version agreement */
	res = 0;
    }

    return res;
}
