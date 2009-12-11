#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nfc/nfc.h>

#include "nfcauth.h"

#if !defined(SYSCONFDIR)
# define SYSCONFDIR "/etc"
#endif /* !SYSCONFDIR */

#if !defined(PAM_NFC_FILE)
# define PAM_NFC_FILE SYSCONFDIR "/pam_nfc.conf"
#endif /* !PAM_NFC_FILE */

#define CRED_FORMAT "%s %s\n"

#define MAX_DEVICES 8
#define MAX_TARGET 32

extern int	 nfcauth_is_authorized (const char *user, char *target);
extern int	 nfcauth_get_targets (char **targets[]);

int
nfcauth_authorize (const char *user)
{
    int grant_access = 0;

    char **targets;
    int i, target_count;

    target_count = nfcauth_get_targets(&targets);
    for (i = 0; i < target_count; i++) {
	if (nfcauth_is_authorized (user, targets[i])) {
	    grant_access = 1;
	    break;
	}
    }

    for (i = 0; i < target_count; i++) {
	free (targets[i]);
    }
    free (targets);

    return grant_access;
}

int
nfcauth_get_targets (char **targets[])
{
    int ret = 0;
    *targets = malloc (MAX_TARGET * sizeof (char *));
    nfc_device_desc_t devices[MAX_DEVICES];
    size_t device_count;
    int i;

    nfc_list_devices(devices, MAX_DEVICES, &device_count);

    for (i = 0; i < device_count; i++) {
	nfc_device_t* initiator = nfc_connect (&(devices[i]));
	if (initiator) {
	    nfc_initiator_init (initiator);


	    // Drop the field for a while
	    nfc_configure (initiator, NDO_ACTIVATE_FIELD, false);

	    // Let the reader only try once to find a tag
	    nfc_configure (initiator, NDO_INFINITE_SELECT, false);

	    // Configure the CRC and Parity settings
	    nfc_configure (initiator, NDO_HANDLE_CRC, true);
	    nfc_configure (initiator, NDO_HANDLE_PARITY, true);

	    // Enable field so more power consuming cards can power themselves up
	    nfc_configure (initiator, NDO_ACTIVATE_FIELD, true);

	    nfc_target_info_t target;

	    while (nfc_initiator_select_tag (initiator, NM_ISO14443A_106, NULL, 0, &target)) {

		if ((*targets)[ret] = malloc (2 * target.nai.szUidLen + 1)) {
		    int n;
		    (*targets)[ret][0] = '\0';
		    for (n = 0; n < target.nai.szUidLen; n++) {
		    	sprintf ((*targets)[ret] + 2 * n, "%02x", target.nai.abtUid[n]);
		    }
		    ret++;
		}

		nfc_initiator_deselect_tag (initiator);
	    }

	    nfc_disconnect (initiator);
	}
    }

    return ret;
}
