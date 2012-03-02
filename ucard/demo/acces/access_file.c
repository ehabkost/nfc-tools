/*-
 * Copyright (C) 2010, Audrey Diacre.
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
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "access_file.h"

#define CRED_FORMAT "%s\n"
#define TEMP_FILE "./tmp-auth.txt"


int
access_file_add ( char *access_key, char *auth_file )
{
    int ret;
    FILE *accessFile;

    if ( ( accessFile = fopen ( auth_file, "r" ) ) ) {
        fclose ( accessFile );
        chmod ( auth_file, 0600 );
    }

    umask ( 0077 );

    if ( ! ( accessFile = fopen ( auth_file, "a" ) ) )
        return 0;

    ret = ( fprintf ( accessFile, CRED_FORMAT, access_key ) > 0 );

    if ( fclose ( accessFile ) != 0 )
        return 0;

    chmod ( auth_file, 0400 );

    return ret;
}

int
access_file_check(char *access_key, char *auth_file){
    int found =0;
    int num_ligne = 0;
    FILE *accessFile;

    if ((accessFile = fopen(auth_file, "r"))) {
        char buffer[BUFSIZ];

        while (!found && fgets (buffer, BUFSIZ, accessFile)) {
            num_ligne ++;
            if (strncmp (access_key,buffer,strlen(access_key)) == 0) {
                found = 1;
            }
        }
        fclose (accessFile);
    }
    if (found) {
        return num_ligne;
    } else {
        return 0;
    }
}

int
access_file_delete ( char *access_key, char *auth_file )
{
    int found = 0;
    FILE *accessFile;
    FILE *temp = NULL;
    int num_lignes = 0;
    int cpt = 1;

    num_lignes = access_file_check ( access_key, auth_file );

    if ( ( accessFile = fopen ( auth_file, "r" ) ) ) {

        temp = fopen ( TEMP_FILE, "w" );

        if ( !temp ) {
            fclose ( accessFile );
        }
        char buf_temp[BUFSIZ];

        while ( fgets ( buf_temp, BUFSIZ, accessFile ) ) {
            if ( cpt != num_lignes ) {
                fputs ( buf_temp, temp );
            }

            memset ( buf_temp, 0, sizeof ( buf_temp ) );
            cpt++;
        }
    }

    fclose ( accessFile );
    fclose ( temp );

    remove ( auth_file );
    rename ( TEMP_FILE, auth_file );

    return found;
}
