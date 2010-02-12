/*
 * NFC Event daemon
 * Copyright (C) 2009 Romuald Conty <rconty@il4p.fr>,
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif // HAVE_CONFIG_H

#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <syslog.h>
#include <unistd.h>

#include "../types.h"

/* current debug level */
static int debug_level = 0;

void set_debug_level(int level) {
    debug_level = level;
}

int get_debug_level() {
    return debug_level;
}

void debug_print(int level, const char *file, int line, const char *format, ...) {
    va_list ap;
    if (debug_level >= level) {
        /* is stdout is a tty */
        if (isatty(1)) {
            const char *t = "\033[34mDEBUG"; /* blue */

            if (-1 == level)
                t = "\033[31mERROR"; /* red */
            else if (0 == level)
                t = ""; /* standard color */

            /* print preamble */
            if ( level > 0 ) printf("%s:%s:%d: ", t, file, line);
            else printf("%s", t);

            /* print message */
            va_start(ap, format);
            vprintf(format, ap);
            va_end(ap);
            /* print postamble */
            printf("\033[0m\n");
        } else {
            /* else we use syslog(3) */
            char buf[100];

            /* print message */
            va_start(ap, format);
            vsnprintf(buf, sizeof(buf), format, ap);
            va_end(ap);

            syslog(LOG_INFO, buf);
        }
    }
}

void _debug_print_tag(const tag_t* tag)
{
  switch (tag->modulation) {
    case NM_ISO14443A_106: {
        /*
      printf ( "The following (NFC) ISO14443A tag was found:\n\n" );
      printf ( "    ATQA (SENS_RES): " ); print_hex ( ti.nai.abtAtqa,2 );
      printf ( "       UID (NFCID%c): ", ( ti.nai.abtUid[0]==0x08?'3':'1' ) ); print_hex ( ti.nai.abtUid,ti.nai.szUidLen );
      printf ( "      SAK (SEL_RES): " ); print_hex ( &ti.nai.btSak,1 );
      if ( ti.nai.uiAtsLen )
      {
      printf ( "          ATS (ATR): " );
      print_hex ( ti.nai.abtAts,ti.nai.uiAtsLen );
    }
        */
      uint32_t uiPos;
      char *uid = malloc(tag->ti.nai.szUidLen*sizeof(char));
      char *uid_ptr = uid;
      for (uiPos=0; uiPos < tag->ti.nai.szUidLen; uiPos++) {
        sprintf(uid_ptr, "%02x",tag->ti.nai.abtUid[uiPos]);
        uid_ptr += 2;
      }
      uid_ptr[0]='\0';

      printf( "ISO14443A (MIFARE) tag with uid=%s", uid );
      free(uid);
    }
    break;
    default:
      DBG( "%s", "Debug this kind of modulation is not yet supported." );
      
  }
}
