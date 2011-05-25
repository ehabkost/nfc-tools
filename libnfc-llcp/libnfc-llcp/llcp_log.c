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
#include <log4c.h>
#include <semaphore.h>

#include "llcp_log.h"

sem_t *log_sem;
const char *sem_name = "/libnfc-llcp";

int
llcp_log_init (void)
{
    if ((log_sem = sem_open (sem_name, O_CREAT, 0666, 1)) == SEM_FAILED) {
	perror ("sem_open");
	return -1;
    }

    return log4c_init ();
}

int
llcp_log_fini (void)
{
    sem_close (log_sem);
    sem_unlink (sem_name);
    return log4c_fini ();
}

void
llcp_log_log (char *category, int priority, char *format, ...)
{
    sem_wait (log_sem);

    const log4c_category_t *cat = log4c_category_get (category);
    if (log4c_category_is_priority_enabled (cat, priority)) {
	va_list va;
	va_start (va, format);
	log4c_category_vlog (cat, priority, format, va);
    }

    sem_post (log_sem);
}
