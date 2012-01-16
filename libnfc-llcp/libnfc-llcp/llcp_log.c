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

#include <fcntl.h>
#include <semaphore.h>

#include "llcp_log.h"

int
llcp_log_init (void)
{
    return 0;
}

int
llcp_log_fini (void)
{
    return 0;
}

void
llcp_log_log (char *category, int priority, char *format, ...)
{
    switch (priority) {
    case LLC_PRIORITY_FATAL:
	printf ("\033[37;41;1m");
	break;
    case LLC_PRIORITY_ALERT:
    case LLC_PRIORITY_CRIT:
    case LLC_PRIORITY_ERROR:
	printf ("\033[31;1m");
	break;
    case LLC_PRIORITY_WARN:
	printf ("\033[33;1m");
	break;
    case LLC_PRIORITY_NOTICE:
	printf ("\033[34;1m");
	break;
    default:
	printf ("\033[32m");
    }
    va_list va;
    va_start (va, format);
    printf ("%s\t", category);
    vprintf (format, va);
    printf("[0m");
    printf ("\n");
    fflush (stdout);
}
