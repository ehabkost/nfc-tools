/*-
 * Copyright (C) 2010, Romain Tartiere.
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
 *
 * $Id$
 */

#include <stdlib.h>
#include <string.h>

#include <ucard.h>

#include "ucard_internal.h"

static int
ucard_application_add_file (struct ucard_application *application, struct ucard_application_file file)
{
    int res = 0;
    struct ucard_application_file *p;

    if ((p = realloc (application->files.files, (application->files.count + 1) * sizeof (*p)))) {
	application->files.files = p;
	application->files.files[application->files.count++] = file;
    } else {
	res = -1;
    }

    return res;
}

int
ucard_application_add_std_data_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, uint32_t file_size)
{
    struct ucard_application_file res;

    res.type = std_data_file;
    res.name = strdup (filename);
    res.access_rights = access_rights;
    res.properties.data_file.file_size = file_size;

    return ucard_application_add_file (ucard_application, res);
}

int
ucard_application_add_backup_data_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, uint32_t file_size)
{
    struct ucard_application_file res;

    res.type = backup_data_file;
    res.name = strdup (filename);
    res.access_rights = access_rights;
    res.properties.data_file.file_size = file_size;

    return ucard_application_add_file (ucard_application, res);
}

int
ucard_application_add_value_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, int32_t lower_limit, int32_t upper_limit, int32_t value, uint8_t limited_credit_enabled)
{
    struct ucard_application_file res;

    res.type = value_file;
    res.name = strdup (filename);
    res.access_rights = access_rights;
    res.properties.value_file.lower_limit = lower_limit;
    res.properties.value_file.upper_limit = upper_limit;
    res.properties.value_file.value = value;
    res.properties.value_file.limited_credit_enabled = limited_credit_enabled;

    return ucard_application_add_file (ucard_application, res);
}

int
ucard_application_add_linear_record_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, uint32_t record_size, uint32_t record_count)
{
	struct ucard_application_file res;

	res.type = linear_record_file;
	res.name = strdup (filename);
	res.access_rights = access_rights;
	res.properties.record_file.record_size = record_size;
	res.properties.record_file.record_count = record_count;

	return ucard_application_add_file (ucard_application, res);
}

int
ucard_application_add_cyclic_record_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, uint32_t record_size, uint32_t record_count)
{
	struct ucard_application_file res;

	res.type = cyclic_record_file;
	res.name = strdup (filename);
	res.access_rights = access_rights;
	res.properties.record_file.record_size = record_size;
	res.properties.record_file.record_count = record_count;

	return ucard_application_add_file (ucard_application, res);
}
