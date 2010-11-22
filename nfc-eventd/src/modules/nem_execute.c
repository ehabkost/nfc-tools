/*
*  C Implementation: nfc-event-execute
*
* Description:
*
*
* Author: Romuald Conty <rconty@il4p.fr>, (C) 2009
*
* Copyright: See COPYING file that comes with this distribution
*
*/
#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif // HAVE_CONFIG_H

#include "nem_execute.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define ONERROR_IGNORE	0
#define ONERROR_RETURN	1
#define ONERROR_QUIT	2

static nfcconf_context* _nem_execute_config_context;
static nfcconf_block* _nem_execute_config_block;

static char * _tag_uid = NULL;

static int strsubst(char *dest, const char *src, const char *search, const char *subst) {
    const char *delim = strstr(src, search);
    if ( delim != NULL ) {
        size_t before_match = (delim - src);
        strncpy(dest, src, before_match);
        src += before_match + strlen(search);
        dest += before_match;
        strcpy(dest, subst);
        dest += strlen(subst);
        strcpy(dest, src);
        dest += strlen(src);
        return 0;
    } else {
        strcpy(dest, src);
        return -1;
    }
}

static int my_system ( char *command ) {
    extern char **environ;
    int pid, status;
    if ( !command ) return 1;
    pid = fork();
    if ( pid == -1 ) return -1;
    if ( pid == 0 ) {
        char *argv[4];
        argv[0] = "/bin/sh";
        argv[1] = "-c";
        argv[2] = command;
        argv[3] = 0;
        execve ( "/bin/sh", argv, environ );
        exit ( 127 );
    }
    do {
        if ( waitpid ( pid, &status, 0 ) == -1 ) {
            if ( errno != EINTR ) return -1;
        } else return status;
    } while ( 1 );
}

void
nem_execute_init( nfcconf_context *module_context, nfcconf_block* module_block ) {
    set_debug_level ( 1 );
    _nem_execute_config_context = module_context;
    _nem_execute_config_block = module_block;
}

void
tag_get_uid(nfc_device_t* nfc_device, nfc_target_t* tag, char **dest) {
  debug_print_tag(tag);

  /// @TODO We don't need to reselect tag to get his UID: nfc_target_t contains this data.
  // Poll for a ISO14443A (MIFARE) tag
  if ( nfc_initiator_select_passive_target ( nfc_device, tag->nm, tag->nti.nai.abtUid, tag->nti.nai.szUidLen, tag ) ) {
      *dest = malloc(tag->nti.nai.szUidLen*sizeof(char)*2+1);
      size_t szPos;
      char *pcUid = *dest;
      for (szPos=0; szPos < tag->nti.nai.szUidLen; szPos++) {
          sprintf(pcUid, "%02x",tag->nti.nai.abtUid[szPos]);
          pcUid += 2;
      }
      pcUid[0]='\0';
      DBG( "ISO14443A tag found: UID=0x%s", *dest );
      nfc_initiator_deselect_target ( nfc_device );
  } else {
      *dest = NULL;
      DBG("%s", "ISO14443A (MIFARE) tag not found" );
      return;
  }
}

int
nem_execute_event_handler(nfc_device_t* nfc_device, nfc_target_t* tag, const nem_event_t event) {
    int onerr;
    const char *onerrorstr;
    const nfcconf_list *actionlist;
    nfcconf_block **blocklist, *myblock;

    const char* action;

    switch (event) {
    case EVENT_TAG_INSERTED:
        action = "tag_insert";
        if ( _tag_uid != NULL ) {
            free(_tag_uid);
        }
        tag_get_uid(nfc_device, tag, &_tag_uid);
        break;
    case EVENT_TAG_REMOVED:
        action = "tag_remove";
        break;
    default:
	return -1;
	break;
    }

    blocklist = nfcconf_find_blocks ( _nem_execute_config_context, _nem_execute_config_block, "event", action );
    if ( !blocklist ) {
        DBG ( "%s", "Event block list not found" );
        return -1;
    }
    myblock = blocklist[0];
    free ( blocklist );
    if ( !myblock ) {
        DBG ( "Event item not found: '%s'", action );
        return -1;
    }
    onerrorstr = nfcconf_get_str ( myblock, "on_error", "ignore" );
    if ( !strcmp ( onerrorstr, "ignore" ) ) onerr = ONERROR_IGNORE;
    else if ( !strcmp ( onerrorstr, "return" ) ) onerr = ONERROR_RETURN;
    else if ( !strcmp ( onerrorstr, "quit" ) ) onerr = ONERROR_QUIT;
    else {
        onerr = ONERROR_IGNORE;
        DBG ( "Invalid onerror value: '%s'. Assumed 'ignore'", onerrorstr );
    }

    /* search actions */
    actionlist = nfcconf_find_list ( myblock, "action" );
    if ( !actionlist ) {
        DBG ( "No action list for event '%s'", action );
        return 0;
    }
    // DBG ( "Onerror is set to: '%s'", onerrorstr );

    if ( _tag_uid == NULL ) {
        ERR( "%s", "Unable to read tag UID... This should not happend !" );
        switch ( onerr ) {
        case ONERROR_IGNORE:
            break;
        case ONERROR_RETURN:
            return 0;
        case ONERROR_QUIT:
            exit ( EXIT_FAILURE );
        default:
            DBG ( "%s", "Invalid onerror value" );
            return -1;
        }
    } else {
        while ( actionlist ) {
            int res;
            char *action_cmd_src = actionlist->data;
            char *action_cmd_dest = malloc((strlen(action_cmd_src) + strlen(_tag_uid))*sizeof(char));
            strsubst(action_cmd_dest, action_cmd_src, "$TAG_UID", _tag_uid);

            DBG ( "Executing action: '%s'", action_cmd_dest );
            /*
            there are some security issues on using system() in
            setuid/setgid programs. so we will use an alternate function
            	*/
            /* res=system(action_cmd); */
            res = my_system ( action_cmd_dest );
            actionlist = actionlist->next;
            /* evaluate return and take care on "onerror" value */
            DBG ( "Action '%s' returns %d", action_cmd_dest, res );
            if ( !res ) continue;
            switch ( onerr ) {
            case ONERROR_IGNORE:
                continue;
            case ONERROR_RETURN:
                return 0;
            case ONERROR_QUIT:
                exit ( EXIT_FAILURE );
            default:
                DBG ( "%s", "Invalid onerror value" );
                return -1;
            }
        }
    }
    return 0;
}

