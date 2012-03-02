#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "on_password_requested.h"

int
on_password_requested (const char *message, char *password, size_t max_size)
{
    printf ("%s: ", message);
    fgets (password, max_size, stdin);
    char *p;
    if ((p = strchr (password, '\n')))
	*p = '\0';
    return 1;
}
