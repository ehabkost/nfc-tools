# $Id$

lib_LTLIBRARIES = libucard.la

libucard_la_SOURCES = kiosk.c \
		      ucard.c \
		      ucard_application.c \
		      ucard_application_create.c \
		      ucard_application_file.c \
		      ucard_application_select.c \
		      ucard_authenticate.c \
		      ucard_clear_record_file.c \
		      ucard_error.c \
		      ucard_read_data.c \
		      ucard_read_records.c \
		      ucard_write_data.c \
		      ucard_write_record.c \
		      ucard_transaction_abort.c \
		      ucard_transaction_commit.c \
		      ucard_value_file_credit.c \
		      ucard_value_file_debit.c \
		      ucard_value_file_limited_credit.c \
		      ucard_value_file_get_value.c

libucard_la_CFLAGS = @LIBFREEFARE_CFLAGS@
libucard_la_LIBADD = @LIBFREEFARE_LIBS@

include_HEADERS = ucard.h

noinst_HEADERS = ucard_internal.h ucard_version.h

man_MANS = \
	   kiosk.3 \
	   ucard.3 \
	   ucard_application.3 \
	   ucard_application_create.3 \
	   ucard_application_file.3 \
	   ucard_application_select.3 \
	   ucard_authenticate.3 \
	   ucard_clear_record_file.3 \
	   ucard_error.3 \
	   ucard_read_data.3 \
	   ucard_read_records.3 \
	   ucard_transaction_abort.3 \
	   ucard_transaction_commit.3 \
	   ucard_value_file_credit.3 \
	   ucard_value_file_debit.3 \
	   ucard_value_file_get_value.3 \
	   ucard_value_file_limited_credit.3 \
	   ucard_write_data.3 \
	   ucard_write_record.3

linkedman = \
	    kiosk.3 kiosk_device_disable.3 \
	    kiosk.3 kiosk_device_enable.3 \
	    kiosk.3 kiosk_device_get_one_shot.3 \
	    kiosk.3 kiosk_device_set_one_shot.3 \
	    kiosk.3 kiosk_device_setup.3 \
	    kiosk.3 kiosk_devices_add.3 \
	    kiosk.3 kiosk_devices_remove.3 \
	    kiosk.3 kiosk_devices_scan.3 \
	    kiosk.3 kiosk_free.3 \
	    kiosk.3 kiosk_get_one_shot.3 \
	    kiosk.3 kiosk_new.3 \
	    kiosk.3 kiosk_select.3 \
	    kiosk.3 kiosk_set_one_shot.3 \
	    kiosk.3 kiosk_setup.3 \
	    kiosk.3 kiosk_start.3 \
	    kiosk.3 kiosk_stop.3 \
	    kiosk.3 kiosk_wait.3 \
	    ucard.3 ucard_get_tag.3 \
	    ucard.3 ucard_new.3 \
	    ucard_application.3 ucard_application_add_key.3 \
	    ucard_application.3 ucard_application_create.3 \
	    ucard_application.3 ucard_application_free.3 \
	    ucard_application.3 ucard_application_get_aid.3 \
	    ucard_application.3 ucard_application_get_key.3 \
	    ucard_application.3 ucard_application_get_name.3 \
	    ucard_application.3 ucard_application_new.3 \
	    ucard_application.3 ucard_application_save_metadata.3 \
	    ucard_application.3 ucard_application_select.3 \
	    ucard_application.3 ucard_application_set_action.3 \
	    ucard_application.3 ucard_application_set_aid.3 \
	    ucard_application.3 ucard_application_set_application_setup.3 \
	    ucard_application.3 ucard_application_set_name.3 \
	    ucard_application.3 ucard_authenticate.3 \
	    ucard_application_file.3 ucard_application_add_backup_data_file.3 \
	    ucard_application_file.3 ucard_application_add_cyclic_record_file.3 \
	    ucard_application_file.3 ucard_application_add_linear_record_file.3 \
	    ucard_application_file.3 ucard_application_add_std_data_file.3 \
	    ucard_application_file.3 ucard_application_add_value_file.3 \
	    ucard_error.3 kiosk_errno.3 \
	    ucard_error.3 kiosk_perror.3 \
	    ucard_error.3 libucard_strerror.3 \
	    ucard_error.3 libucard_strerror_r.3 \
	    ucard_error.3 ucard_errno.3 \
	    ucard_error.3 ucard_perror.3

install-data-hook:
	(cd $(DESTDIR)$(man3dir); for i in `echo $(linkedman) | xargs -n2 echo | awk '{print $$2}'`; do rm -f $$i; done; echo $(linkedman) | xargs -n2 $(LN_S))

uninstall-hook:
	(cd $(DESTDIR)$(man3dir); for i in `echo $(linkedman) | xargs -n2 echo | awk '{print $$2}'`; do rm -f $$i; done;)

