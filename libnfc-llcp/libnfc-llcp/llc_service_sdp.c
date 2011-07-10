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

#include <sys/mqueue.h>

#include <fcntl.h>
#include <pthread.h>
#include <string.h>

#include "llcp.h"
#include "llcp_log.h"
#include "llc_service.h"
#include "llc_service_sdp.h"
#include "llcp_parameters.h"

#define LOG_LLC_SDP "libnfc-llcp.llc.sdp"
#define LLC_SDP_MSG(priority, message) llcp_log_log (LOG_LLC_SDP, priority, "%s", message)
#define LLC_SDP_LOG(priority, format, ...) llcp_log_log (LOG_LLC_SDP, priority, format, __VA_ARGS__)

/* Service Discovery Protocol */

void
llc_service_sdp_thread_cleanup (void *arg)
{
    (void) arg;
}

void *
llc_service_sdp_thread (void *arg)
{
    struct llc_link *link = (struct llc_link *) arg;
    mqd_t llc_up, llc_down;

    int old_cancelstate;

    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_cancelstate);

    llc_up   = mq_open (link->services[1]->mq_up_name, O_RDONLY);
    llc_down = mq_open (link->services[1]->mq_down_name, O_WRONLY);

    if (llc_up == (mqd_t)-1)
	LLC_SDP_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->services[1]->mq_up_name);
    if (llc_down == (mqd_t)-1)
	LLC_SDP_LOG (LLC_PRIORITY_ERROR, "mq_open(%s)", link->services[1]->mq_down_name);

    pthread_cleanup_push (llc_service_sdp_thread_cleanup, arg);
    pthread_setcancelstate (old_cancelstate, NULL);
    LLC_SDP_LOG (LLC_PRIORITY_INFO, "(%p) Service Discovery Protocol started", (void *)pthread_self ());

    for (;;) {
	int res;

	uint8_t buffer[1024];
	LLC_SDP_LOG (LLC_PRIORITY_TRACE, "(%p[1]) mq_receive+", pthread_self ());
	pthread_testcancel ();
	res = mq_receive (llc_up, (char *) buffer, sizeof (buffer), NULL);
	pthread_testcancel ();
	if (res < 0) {
	    pthread_testcancel ();
	}
	LLC_SDP_LOG (LLC_PRIORITY_TRACE, "(%p[1]) received %d bytes", pthread_self (), res);

	uint8_t tid;
	char *uri;

	switch (buffer[2]) {
	case LLCP_PARAMETER_SDREQ:
	    if (parameter_decode_sdreq (buffer + 2, res - 2, &tid, &uri) < 0) {
		LLC_SDP_LOG (LLC_PRIORITY_ERROR, "(%p[1]) Ignoring PDU", pthread_self ());
	    } else {
		LLC_SDP_LOG (LLC_PRIORITY_TRACE, "(%p[1]) Service Discovery Request #0x%02x for '%s'", pthread_self (), tid, uri);

		int sap = 0;
		for (int i = 1; i <= 0x1F; i++) {
		    if (link->services[i]) {
			if (link->services[i]->uri &&
			    (0 == strcmp (link->services[i]->uri, uri))) {
			    LLC_SDP_LOG (LLC_PRIORITY_INFO, "(%p[1]) Service '%s' is bound to SAP '%d'", pthread_self (), uri, i);
			    sap = i;
			    break;
			}
		    }
		}

		if (!sap) {
		    LLC_SDP_LOG (LLC_PRIORITY_ERROR, "(%p[1]) No registered service provide '%s'", pthread_self (), uri);
		}
		buffer[0] = 0x06;
		buffer[1] = 0x41;
		int n = parameter_encode_sdres (buffer + 2, sizeof (buffer) -2, tid, sap);

		mq_send (llc_down, (char *) buffer, n + 2, 0);
		LLC_SDP_LOG (LLC_PRIORITY_TRACE, "(%p[1]) sent %d bytes", pthread_self (), n+2);

	    }
	    break;
	default:
	    LLC_SDP_LOG (LLC_PRIORITY_ERROR, "(%p[1]) invalid parameter type", pthread_self ());
	}
    }

    pthread_cleanup_pop (1);
    return NULL;
}
