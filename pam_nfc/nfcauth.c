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

extern int     nfcauth_is_authorized (const char *user, char *target);
extern int     nfcauth_get_targets (char **targets[]);

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
    nfc_connstring devices[MAX_DEVICES];
    size_t device_count;
    size_t target_count;
    int i;
    
    nfc_init(NULL);
    device_count = nfc_list_devices(NULL, devices, MAX_DEVICES);

    nfc_modulation nm = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_UNDEFINED
    };

    for (i = 0; i < device_count; i++) {
        nfc_device* initiator = nfc_open (NULL, devices[i]);
        if (initiator) {
            nfc_initiator_init (initiator);
            nfc_target ant[MAX_TARGET];

            if ((target_count = nfc_initiator_list_passive_targets (initiator, nm, ant, MAX_TARGET)) >= 0) {
                size_t  iTarget;
                for (iTarget = 0; iTarget < target_count; iTarget++) {
                    if ((*targets)[ret] = malloc (2 * ant[iTarget].nti.nai.szUidLen + 1)) {
                        int n;
                        (*targets)[ret][0] = '\0';
                        for (n = 0; n < ant[iTarget].nti.nai.szUidLen; n++) {
                            sprintf ((*targets)[ret] + 2 * n, "%02x", ant[iTarget].nti.nai.abtUid[n]);
                        }
                        ret++;
                    }
                }
            }
            nfc_close (initiator);
        }
    }
    nfc_exit(NULL);
    return ret;
}
