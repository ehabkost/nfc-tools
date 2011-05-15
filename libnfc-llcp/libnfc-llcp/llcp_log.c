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

#include <log4c.h>

#include "llcp_log.h"

int
llcp_log_init (void)
{
    return log4c_init ();
}

int
llcp_log_fini (void)
{
    return log4c_fini ();
}

void
llcp_log_msg (char *category, int priority, char *message)
{
    log4c_category_log (log4c_category_get (category), priority, message);
}

void
llcp_log_set_appender (char *category, char *appender)
{
    log4c_category_set_appender (log4c_category_get (category), log4c_appender_get (appender));
}

void
llcp_log_log (char *category, int priority, char *format, ...)
{
    const log4c_category_t *cat = log4c_category_get (category);
    if (log4c_category_is_priority_enabled (cat, priority)) {
	va_list va;
	va_start (va, format);
	log4c_category_vlog (cat, priority, format, va);
    }
}
