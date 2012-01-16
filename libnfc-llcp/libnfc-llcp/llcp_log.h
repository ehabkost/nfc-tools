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

#ifndef _LLC_LOG_H
#define _LLC_LOG_H

#ifdef DEBUG

int	 llcp_log_init (void);
int	 llcp_log_fini (void);
void	 llcp_log_log (char *category, int priority, char *format, ...);

#define LLC_PRIORITY_FATAL  0
#define LLC_PRIORITY_ALERT  1
#define LLC_PRIORITY_CRIT   2
#define LLC_PRIORITY_ERROR  3
#define LLC_PRIORITY_WARN   4
#define LLC_PRIORITY_NOTICE 5
#define LLC_PRIORITY_INFO   6
#define LLC_PRIORITY_DEBUG  7
#define LLC_PRIORITY_TRACE  8

#else

#define llcp_log_init() (0)
#define llcp_log_fini() (0)
#define llcp_log_msg(category, priority, message) do {} while (0)
#define llcp_log_set_appender(category, appender) do {} while (0)
#define llcp_log_log(category, priority, format, ...) do {} while (0)

#define LLC_PRIORITY_FATAL  8
#define LLC_PRIORITY_ALERT  7
#define LLC_PRIORITY_CRIT   6
#define LLC_PRIORITY_ERROR  5
#define LLC_PRIORITY_WARN   4
#define LLC_PRIORITY_NOTICE 3
#define LLC_PRIORITY_INFO   2
#define LLC_PRIORITY_DEBUG  1
#define LLC_PRIORITY_TRACE  0

#endif

#endif
