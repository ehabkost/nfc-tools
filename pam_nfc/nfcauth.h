#ifndef _NFCAUTH_H
#define _NFCAUTH_H

/* Common public functions */
int	 nfcauth_authorize (char *user);
int	 nfcauth_get_targets (char **targets[]);

/* Backend specific public functions */
int	 nfcauth_check (void);
int	 nfcauth_add_authorization (char *user, char *target);

#endif /* _NFCAUTH_H */
