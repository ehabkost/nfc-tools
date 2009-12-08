#if defined(PAM_NFC_BACKEND_CONFIG)

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(HAVE_CRYPT_H)
#include <crypt.h>
#endif /* HAVE_CRYPT_H */

#include <nfc/nfc.h>

#include "nfcauth.h"

#if !defined(SYSCONFDIR)
# define SYSCONFDIR "/etc"
#endif /* !SYSCONFDIR */

#if !defined(PAM_NFC_FILE)
# define PAM_NFC_FILE SYSCONFDIR "/pam_nfc.conf"
#endif /* !PAM_NFC_FILE */

#define CRED_FORMAT "%s %s\n"

#if !defined(CRYPT_SALT)
# define CRYPT_SALT "RC"
#endif

int	 nfcauth_is_authorized (char *user, char *target);

int
nfcauth_check (void)
{
    struct stat conffile_fileinfo;

    if (stat (PAM_NFC_FILE, &conffile_fileinfo)) {
	return 0;
    }

    if ( ( conffile_fileinfo.st_mode & S_IWOTH )
	    || !S_ISREG ( conffile_fileinfo.st_mode ) )
    {
	/* If the file is world writable or is not a normal file, return error */
	return 0;
    }
    return 1;
}

int
nfcauth_add_authorization (char *user, char *target)
{
    int ret;

    FILE *config;

    /*
     * If the config file exists it is supposed to be read-only.
     * In such a situation, chmod it so that we can write to it.
     */
    if ((config = fopen (PAM_NFC_FILE, "r"))) {
	fclose (config);
	chmod (PAM_NFC_FILE, 0600);
    }

    /* If no file exists, avoid race condition. */
    umask (0077);

    if (!(config = fopen (PAM_NFC_FILE, "a")))
	return 0;

    ret = (fprintf (config, CRED_FORMAT, user, crypt(target, CRYPT_SALT)) > 0);
    
    if (fclose (config) != 0)
	return 0;

    /* Protect teh configuration file setting it read-only. */
    chmod (PAM_NFC_FILE, 0400);

    return ret;
}

int
nfcauth_is_authorized (char *user, char *target)
{
    int found = 0;
    FILE *config;

    char needle[BUFSIZ];

    snprintf (needle, BUFSIZ, CRED_FORMAT, user, crypt(target, CRYPT_SALT));

    if ((config = fopen(PAM_NFC_FILE, "r"))) {
	char buffer[BUFSIZ];
	while (!found && fgets (buffer, BUFSIZ, config)) {
	    if (strcmp (buffer, needle) == 0)
		found = 1;
	}
	fclose (config);
    }

    return found;
}

#endif /* PAM_NFC_BACKEND_CONFIG */
