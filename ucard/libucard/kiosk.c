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

#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <ucard.h>

#include "ucard_internal.h"

struct kiosk *
kiosk_new (void)
{
    struct kiosk *res;

    if ((res = malloc (sizeof (*res)))) {
	res->last_error = LIBUCARD_SUCCESS;
	res->max_fd = 0;
	res->device_count = 0;
	res->devices = NULL;
    }

    return res;
}

int
kiosk_devices_scan (struct kiosk *kiosk)
{
    kiosk->last_error = LIBUCARD_SUCCESS;

    nfc_connstring nfc_devices[8];
    size_t nfc_device_count;

    nfc_device_count = nfc_list_devices (NULL, nfc_devices, 8);

    for (size_t n = 0; n < nfc_device_count; n++) {
	fprintf (stderr, "[Kiosk] Found device %d: %s\n", (int) n, nfc_devices[n]);
	if (!kiosk_devices_add (kiosk, nfc_devices[n])) {
	    free (kiosk->devices);
	    return -1;
	}
    }

    return (int) nfc_device_count;
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))

struct kiosk_device *
kiosk_devices_add (struct kiosk *kiosk, nfc_connstring connstring)
{
    kiosk->last_error = LIBUCARD_SUCCESS;

    struct kiosk_device *new_devices;

    if (!(new_devices = realloc (kiosk->devices, (1 + kiosk->device_count) * sizeof (*new_devices)))) {
	kiosk->last_error = LIBUCARD_MALLOC;
	return NULL;
    }
    
	kiosk->devices = new_devices;
	kiosk->devices[kiosk->device_count].kiosk = kiosk;
	memcpy (kiosk->devices[kiosk->device_count].connstring, connstring, sizeof (nfc_connstring));
	kiosk->devices[kiosk->device_count].application = NULL;
	kiosk->devices[kiosk->device_count].action = NULL;
	kiosk->devices[kiosk->device_count].enabled = true;
	kiosk->devices[kiosk->device_count].running = false;
	kiosk->devices[kiosk->device_count].one_shot = true;
	if (pipe (kiosk->devices[kiosk->device_count].fds) < 0){
	    kiosk->last_error = KIOSK_PIPE;
	    return NULL;
        }
	kiosk->max_fd = MAX (kiosk->max_fd, MAX (kiosk->devices[kiosk->device_count].fds[0],
						 kiosk->devices[kiosk->device_count].fds[1]));
	kiosk->device_count++;

    return new_devices;
}

void
kiosk_setup (struct kiosk *kiosk, struct ucard_application *application, ucard_presented_callback action)
{
    kiosk->last_error = LIBUCARD_SUCCESS;

    for (size_t n = 0; n < kiosk->device_count; n++) {
	kiosk_device_setup (&kiosk->devices[n], application, action);
    }
}

int
kiosk_get_one_shot (struct kiosk *kiosk, bool *one_shot)
{
    kiosk->last_error = LIBUCARD_SUCCESS;

    bool ref = false;

    if (!kiosk->device_count) {
	kiosk->last_error = KIOSK_NO_DEVICE;
	return -1;
    }

    for (size_t n = 0; n < kiosk->device_count; n++) {
	bool val;

	if (!n) {
	    kiosk_device_get_one_shot (&kiosk->devices[n], &ref);
	} else {
	    kiosk_device_get_one_shot (&kiosk->devices[n], &val);
	    if (ref != val) {
		kiosk->last_error = KIOSK_INCONSISTENT;
		return -1;
	    }
	}
    }

    *one_shot = ref;

    return 0;
}
int
kiosk_set_one_shot (struct kiosk *kiosk, const bool one_shot)
{
    kiosk->last_error = LIBUCARD_SUCCESS;

    for (size_t n = 0; n < kiosk->device_count; n++) {
	if (kiosk_device_set_one_shot (&kiosk->devices[n], one_shot) < 0)
	    return -1;
    }

    return 0;
}

static void *
kiosk_run (void *user_data)
{
    struct kiosk_device *kiosk_device = (struct kiosk_device *)user_data;
    fprintf (stderr, "[Thread %p] Starting\n", (void *) kiosk_device->thread);

    kiosk_device->running = true;

    nfc_device *nfc_device = nfc_open (NULL, kiosk_device->connstring);

    bool quit = false;

    while (!quit) {
	fprintf (stderr, "[Thread %p] Poll\n", (void *) kiosk_device->thread);
	MifareTag *tags = freefare_get_tags (nfc_device);
	for (int i = 0; (!quit) && tags[i]; i++) {
	    fprintf (stderr, "[Thread %p] Found TAG\n", (void *) kiosk_device->thread);
	    if (DESFIRE == freefare_get_tag_type (tags[i])) {
		fprintf (stderr, "[Thread %p] TAG is Mifare DESFire\n", (void *) kiosk_device->thread);

		struct ucard *ucard = ucard_new (tags[i]);
		if (mifare_desfire_connect (tags[i]) < 0) {
		    quit = true;
		    continue;
		}
		if (!kiosk_device->action (ucard, kiosk_device->application))
		    quit = true;
		mifare_desfire_disconnect (tags[i]);
		free (ucard);

		if (kiosk_device->one_shot)
		    quit = true;
	    }
	}
	freefare_free_tags (tags);
	sleep (1);
	pthread_testcancel ();
    }

    nfc_close (nfc_device);

    fprintf (stderr, "[Thread %p] Terminating\n", (void *) kiosk_device->thread);
    close (kiosk_device->fds[1]);

    kiosk_device->running = false;

    return NULL;
}

int
kiosk_start (struct kiosk *kiosk)
{
    kiosk->last_error = LIBUCARD_SUCCESS;

    if (!kiosk->device_count) {
      kiosk->last_error = KIOSK_NO_DEVICE;
      return -1;
    }
    for (size_t n = 0; n < kiosk->device_count; n++) {
	if (!kiosk->devices[n].enabled)
	    continue;
	pthread_create (&kiosk->devices[n].thread, NULL, kiosk_run, &kiosk->devices[n]);
	fprintf (stderr, "[Kiosk] Starting device %d: Thread %p\n", (int) n, (void *) kiosk->devices[n].thread);
    }

    return 0;
}

int
kiosk_wait (struct kiosk *kiosk, struct timeval *timeout)
{
    kiosk->last_error = LIBUCARD_SUCCESS;

    if (!kiosk->device_count) {
      kiosk->last_error = KIOSK_NO_DEVICE;
      return -1;
    }

    fd_set readfds;
    FD_ZERO (&readfds);

    return kiosk_select (kiosk, 0, &readfds, NULL, NULL, timeout);
}

int
kiosk_select (struct kiosk *kiosk, int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    kiosk->last_error = LIBUCARD_SUCCESS;

    if (!readfds) {
	kiosk->last_error = LIBUCARD_INVALID_ARGUMENT;
	return -1;
    }

    if (!kiosk->device_count) {
      kiosk->last_error = KIOSK_NO_DEVICE;
      return -1;
    }

    for (size_t n = 0; n < kiosk->device_count; n++) {
	if (!kiosk->devices[n].enabled)
	    continue;
	FD_SET (kiosk->devices[n].fds[0], readfds);
    }

    nfds = MAX (nfds, kiosk->max_fd);

    return select (nfds, readfds, writefds, exceptfds, timeout);
}

void
kiosk_stop (struct kiosk *kiosk)
{
    kiosk->last_error = LIBUCARD_SUCCESS;

    for (size_t n = 0; n < kiosk->device_count; n++) {
	if (!kiosk->devices[n].enabled)
	    continue;
	fprintf (stderr, "[Kiosk] Canceling thread %p\n", (void *) kiosk->devices[n].thread);
	pthread_cancel (kiosk->devices[n].thread);
	fprintf (stderr, "[Kiosk] Joining thread %p\n", (void *) kiosk->devices[n].thread);
	pthread_join (kiosk->devices[n].thread, NULL);
	kiosk->devices[n].running = false;
    }
}

int
kiosk_device_setup (struct kiosk_device *device, struct ucard_application *application, ucard_presented_callback action)
{
    if (!device->running) {
	device->application = application;
	device->action = action;
	return 0;
    } else {
	device->kiosk->last_error = KIOSK_BUSY;
	return -1;
    }
}

int
kiosk_device_set_one_shot (struct kiosk_device *device, const bool one_shot)
{
    if (!device->running) {
	device->one_shot = one_shot;
	return 0;
    } else {
	device->kiosk->last_error = KIOSK_BUSY;
	return -1;
    }
}

void
kiosk_device_get_one_shot (const struct kiosk_device *device, bool *one_shot)
{
    *one_shot = device->one_shot;
}

int
kiosk_device_enable (struct kiosk_device *device)
{
    if (!device->running) {
	device->enabled = true;
	return 0;
    } else {
	device->kiosk->last_error = KIOSK_BUSY;
	return -1;
    }
}

int
kiosk_device_disable (struct kiosk_device *device)
{
    if (!device->running) {
	device->enabled = false;
	return 0;
    } else {
	device->kiosk->last_error = KIOSK_BUSY;
	return -1;
    }
}

void
kiosk_free (struct kiosk *kiosk)
{
    free (kiosk->devices);
    free (kiosk);
}
