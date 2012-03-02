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
 * $Id: ucard.h 641M 2012-03-02 12:10:02Z (local) $
 */

#ifndef _UCARD_H
#define _UCARD_H

#include <sys/time.h>
#include <sys/types.h>

#include <freefare.h>

#define UCARD_METDATA_FILENO 8

#define UCARD_KEY_VERSION 0x00

#define UCARD_LEGACY_KEY_VERSION (0x00 | UCARD_KEY_VERSION)
#define UCARD_ISO_KEY_VERSION    (0x40 | UCARD_KEY_VERSION)
#define UCARD_AES_KEY_VERSION    (0x80 | UCARD_KEY_VERSION)

#define UCARD_INFO_AID 0x839764
#define USER_ACCESS_KEYNO 1
#define USER_ACCESS_COMMUNICATION_MODE MDCM_ENCIPHERED
#define USER_KEYRING_FILENO 11
#define USER_KEYRING_COMMUNICATION_MODE MDCM_ENCIPHERED

struct kiosk;
struct ucard;
struct ucard_application;
struct ucard_application_file;
struct ucard_application_files;
struct ucard_application_keys;

/* Error reporting functions */
int		 ucard_errno (struct ucard *ucard);
void		 ucard_perror (struct ucard *ucard, const char *message);
int		 kiosk_errno (struct kiosk *kiosk);
void		 kiosk_perror (struct kiosk *kiosk, const char *message);
const char	*libucard_strerror (const int errnum);
char		*libucard_strerror_r (const int errnum, char *strbuf, const size_t buflen);

/* Calback functions */
typedef int	 (*ucard_application_setup_callback)(MifareTag tag, MifareDESFireAID aid, MifareDESFireKey access_key);
typedef int	 (*ucard_presented_callback)(struct ucard *ucard, struct ucard_application *application);
typedef int	 (*password_request_callback)(const char *message, char *password, size_t max_length);

/* kiosk functions */
struct kiosk	*kiosk_new (void);
int		 kiosk_start (struct kiosk *kiosk);
int		 kiosk_wait (struct kiosk *kiosk, struct timeval *timeout);
int		 kiosk_select (struct kiosk *kiosk, int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
void		 kiosk_stop (struct kiosk *kiosk);
void		 kiosk_free (struct kiosk *kiosk);

int		 kiosk_devices_scan (struct kiosk *kiosk);
void		 kiosk_setup (struct kiosk *kiosk, struct ucard_application *application, ucard_presented_callback action);
int		 kiosk_get_one_shot (struct kiosk *kiosk, bool *one_shot);
int		 kiosk_set_one_shot (struct kiosk *kiosk, const bool one_shot);

struct kiosk_device *kiosk_devices_add (struct kiosk *kiosk, nfc_connstring connstring);
int		 kiosk_devices_remove (struct kiosk *kiosk, struct kiosk_device *device);

int		 kiosk_device_setup (struct kiosk_device *device, struct ucard_application *application, ucard_presented_callback action);
void		 kiosk_device_get_one_shot (const struct kiosk_device *device, bool *one_shot);
int		 kiosk_device_set_one_shot (struct kiosk_device *device, const bool one_shot);
int		 kiosk_device_enable (struct kiosk_device *device);
int		 kiosk_device_disable (struct kiosk_device *device);

/* ucard functions */
struct ucard	*ucard_new (MifareTag tag);
MifareTag	 ucard_get_tag (struct ucard *ucard);
void		 ucard_derivate_password (const char *pass, int passlen, int keylen, unsigned char *out);

int		 ucard_authenticate (struct ucard *ucard, const uint8_t key_no, MifareDESFireKey key);
int		 ucard_application_select (struct ucard *ucard, const struct ucard_application *ucard_application);
int		 ucard_application_create (struct ucard *ucard, const struct ucard_application *application);

int		 ucard_read_data (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key, const uint32_t offset, const uint32_t length, void *data);
int		 ucard_write_data (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key, const uint32_t offset, const uint32_t length, void *data);

int		 ucard_read_records (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key, const uint32_t offset, const uint32_t length, void *data);
int		 ucard_write_record (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key, const uint32_t offset, const uint32_t length, void *data);
int		 ucard_clear_record_file (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key);

int		 ucard_value_file_get_value (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key, int32_t *value);
int		 ucard_value_file_credit (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key, const int32_t amount);
int		 ucard_value_file_limited_credit (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key, const int32_t amount);
int		 ucard_value_file_debit (struct ucard *ucard, const struct ucard_application *application, const uint8_t file_no, const uint8_t key_no, MifareDESFireKey key, const int32_t amount);

int		 ucard_transaction_commit (struct ucard *ucard);
int		 ucard_transaction_abort (struct ucard *ucard);

/* ucard_application functions */
struct ucard_application *ucard_application_new (password_request_callback on_password_requested);
void		 ucard_application_free (struct ucard_application *application);

int		 ucard_application_save_metadata (struct ucard *ucard, const struct ucard_application *application);

const char	*ucard_application_get_name (const struct ucard_application *application);
void		 ucard_application_set_name (struct ucard_application *application, const char *name);

void		 ucard_application_set_aid (struct ucard_application *application, const uint32_t aid);
uint32_t	 ucard_application_get_aid (const struct ucard_application *application);

void		 ucard_application_set_application_setup (struct ucard_application *application, ucard_application_setup_callback application_setup);

void		 ucard_application_set_action (struct ucard_application *application, ucard_presented_callback on_ucard);

int		 ucard_application_add_key (struct ucard_application *application, const MifareDESFireKey key);
MifareDESFireKey ucard_application_get_key (const struct ucard_application *application, const uint8_t key_no);

int		 ucard_application_add_std_data_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, uint32_t file_size);
int		 ucard_application_add_backup_data_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, uint32_t file_size);
int		 ucard_application_add_value_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, int32_t lower_limit, int32_t upper_limit, int32_t value, uint8_t limited_credit_enabled);
int		 ucard_application_add_linear_record_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, uint32_t record_size, uint32_t record_count);
int		 ucard_application_add_cyclic_record_file (struct ucard_application *ucard_application, const char *filename, uint32_t access_rights, uint32_t record_size, uint32_t record_count);

#endif /* !_UCARD_H */
