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
 * $Id: ucard_internal.h 565M 2012-03-02 12:12:20Z (local) $
 */

#ifndef _UCARD_INTERNAL_H
#define _UCARD_INTERNAL_H

#include <pthread.h>

enum libucard_error {
    LIBUCARD_SUCCESS = 0,
    LIBUCARD_INVALID_ARGUMENT,
    LIBUCARD_MALLOC,
    UCARD_FREEFARE_ERROR,
    KIOSK_NO_DEVICE,
    KIOSK_BUSY,
    KIOSK_PIPE,
    KIOSK_INCONSISTENT
};

struct ucard {
    MifareTag tag;
    enum libucard_error last_error;
    uint32_t selected_aid;
    int current_key;
    bool transaction_in_progress;
};

struct kiosk_device;

struct kiosk {
    enum libucard_error last_error;
    int max_fd;
    size_t device_count;
    struct kiosk_device *devices;
};

struct kiosk_device {
    struct kiosk *kiosk;
    bool enabled;
    bool running;
    bool one_shot;
    pthread_t thread;
    nfc_connstring connstring;
    struct ucard_application *application;
    ucard_presented_callback action;
    int fds[2];
};

struct ucard_application_keys {
    size_t count;
    MifareDESFireKey *keys;
};

struct ucard_application_files {
    size_t count;
    struct ucard_application_file *files;
};

struct ucard_application {
    uint32_t aid;
    char *name;
    struct ucard_application_keys keys;
    struct ucard_application_files files;

    ucard_application_setup_callback application_setup;
    ucard_presented_callback on_ucard;
    password_request_callback on_password_requested;
};

struct ucard_application_file {
    enum { std_data_file, backup_data_file, value_file, linear_record_file, cyclic_record_file } type;
    char *name;
    uint32_t access_rights;
    union {
	struct {
	    uint32_t file_size;
	} data_file;
	struct {
	    int32_t lower_limit;
	    int32_t upper_limit;
	    int32_t value;
	    uint8_t limited_credit_enabled;
	} value_file;
	struct {
	    uint32_t record_size;
	    uint32_t record_count;
	} record_file;
    } properties;
};

#endif /* !_UCARD_INTERNAL_H */
