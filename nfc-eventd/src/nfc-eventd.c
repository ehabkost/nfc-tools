/**
 * NFC Event Daemon
 * Generate events on tag status change
 * Copyrigt (C) 2009 Romuald Conty <rconty@il4p.fr>
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif // HAVE_CONFIG_H

#include <nfc/nfc.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <unistd.h>

#include <errno.h>
#include <signal.h>

/* Dynamic load */
#include <dlfcn.h>
/* Configuration parser */
#include "nfcconf/nfcconf.h"
/* Debugging functions */
#include "debug/debug.h"
/* Module common defines */
#include "modules/nem_common.h"

#include "types.h"

#define DEF_POLLING 1    /* 1 second timeout */
#define DEF_EXPIRE 0    /* no expire */

#define DEF_CONFIG_FILE SYSCONFDIR"/nfc-eventd.conf"

int polling_time;
int expire_time;
int daemonize;
int debug;
char *cfgfile;

nfcconf_context *ctx;
const nfcconf_block *root;

typedef struct slot_st slot_t;

static module_init_fct_t module_init_fct_ptr = NULL;
static module_event_handler_fct_t module_event_handler_fct_ptr = NULL;
static nfc_device_desc_t* nfc_device_desc = NULL;

/**
 * @brief Load and init specified (in config file) NEM module
 */
static int load_module( void ) {
    nfcconf_block **module_list, *my_module;

    module_list = nfcconf_find_blocks ( ctx, root, "module", NULL );
    if ( !module_list ) {
        ERR ( "%s", "Module item not found." );
        return -1;
    }
    my_module = module_list[0];
    free ( module_list );
    if ( !my_module ) {
        ERR ( "%s", "Module item not found." );
        return -1;
    }
    DBG("Loading module: '%s'...", my_module->name->data);
    char module_path[256]={ '\0', };
    strcat(module_path, NEMDIR"/" );
    strcat(module_path, my_module->name->data);
    strcat(module_path, ".so");
    DBG("Module found at: '%s'...", module_path);

    void *module_handler;
    module_handler = dlopen(module_path,RTLD_LAZY);
    if ( module_handler == NULL ) {
        ERR("Unable to open module: %s", dlerror());
        exit(EXIT_FAILURE);
    }

    char module_fct_name[256];
    char *error;

    module_fct_name[0]='\0';
    strcat(module_fct_name,my_module->name->data);
    strcat(module_fct_name,"_init");

    module_init_fct_ptr = dlsym(module_handler,module_fct_name);

    if ((error = dlerror()) != NULL) {
        ERR ("%s", error);
        exit(EXIT_FAILURE);
    }

    module_fct_name[0]='\0';
    strcat(module_fct_name,my_module->name->data);
    strcat(module_fct_name,"_event_handler");

    module_event_handler_fct_ptr = dlsym(module_handler,module_fct_name);
    if ((error = dlerror()) != NULL) {
        ERR ( "%s", error);
        exit(EXIT_FAILURE);
    }

    (*module_init_fct_ptr)( ctx, my_module );
    return 0;
}

/**
 * @brief Execute NEM function that handle events
 */
static int execute_event ( const nfc_device_t *nfc_device, const nfc_target_t* tag, const nem_event_t event ) {
    return (*module_event_handler_fct_ptr)( nfc_device, tag, event );
}

/**
 * @brief Parse configuration file
 */
static int parse_config_file() {
    ctx = nfcconf_new ( cfgfile );
    if ( !ctx ) {
        ERR ( "%s", "Error creating conf context" );
        return -1;
    }
    if ( nfcconf_parse ( ctx ) <= 0 ) {
        ERR ( "Error parsing file '%s'", cfgfile );
        return -1;
    }
    /* now parse options */
    root = nfcconf_find_block ( ctx, NULL, "nfc-eventd" );
    if ( !root ) {
        ERR ( "nfc-eventd block not found in config: '%s'", cfgfile );
        return -1;
    }
    debug = nfcconf_get_bool ( root, "debug", debug );
    daemonize = nfcconf_get_bool ( root, "daemon", daemonize );
    polling_time = nfcconf_get_int ( root, "polling_time", polling_time );
    expire_time = nfcconf_get_int ( root, "expire_time", expire_time );

    if ( debug ) set_debug_level ( 1 );

    DBG( "%s", "Looking for specified NFC device." );
    nfcconf_block **device_list, *my_device;
    const char* nfc_device_str = nfcconf_get_str ( root, "nfc_device", "" );
    if (strcmp( nfc_device_str, "") != 0) {
        device_list = nfcconf_find_blocks ( ctx, root, "device", NULL );
        if ( !device_list ) {
            ERR ( "%s", "Device item not found." );
            return -1;
        }
        int i = 0;
        my_device = device_list[i];
        while ( my_device != NULL ) {
            i++;
            if ( strcmp(my_device->name->data, nfc_device_str) == 0 ) {
                INFO("Specified device %s have been found.", nfc_device_str);
                nfc_device_desc = malloc(sizeof(nfc_device_desc_t));
                nfc_device_desc->pcDriver = (char*)nfcconf_get_str( my_device, "driver", "" );
		char* device_name = (char*)nfcconf_get_str( my_device, "name", "" );
		strncpy(nfc_device_desc->acDevice, device_name, sizeof(nfc_device_desc->acDevice));
                char* device_port   = (char*)nfcconf_get_str( my_device, "port", "" );
		strncpy(nfc_device_desc->acPort, device_port, sizeof(nfc_device_desc->acPort));
                nfc_device_desc->uiSpeed  = nfcconf_get_int( my_device, "speed", 9600 );
                nfc_device_desc->uiBusIndex  = nfcconf_get_int( my_device, "index", 0 );
                break;
            }
            my_device = device_list[i];
        }
        DBG( "Found %d device configuration block(s).", i );
        if ( nfc_device_desc == NULL ) {
            ERR("NFC device have been specified in configuration file but there is no device description. Unable to select specified device: %s.", nfc_device_str);
        }
        free ( device_list );
    }

    return 0;
}

/**
 * @brief Parse command line args
 */
static int parse_args ( int argc, char *argv[] ) {
    int i;
    int res;
    polling_time = DEF_POLLING;
    expire_time = DEF_EXPIRE;
    debug   = 0;
    daemonize  = 0;
    cfgfile = DEF_CONFIG_FILE;
    /* first of all check whether debugging should be enabled */
    for ( i = 0; i < argc; i++ ) {
        if ( ! strcmp ( "debug", argv[i] ) ) set_debug_level ( 1 );
    }
    /* try to find a configuration file entry */
    for ( i = 0; i < argc; i++ ) {
        if ( strstr ( argv[i], "config_file=" ) ) {
            cfgfile = 1 + strchr ( argv[i], '=' );
            break;
        }
    }
    /* parse configuration file */
    if ( parse_config_file() < 0 ) {
        ERR ( "Error parsing configuration file %s", cfgfile );
        exit ( EXIT_FAILURE );
    }

    /* and now re-parse command line to take precedence over cfgfile */
    for ( i = 1; i < argc; i++ ) {
        if ( strcmp ( "daemon", argv[i] ) == 0 ) {
            daemonize = 1;
            continue;
        }
        if ( strcmp ( "nodaemon", argv[i] ) == 0 ) {
            daemonize = 0;
            continue;
        }
        if ( strstr ( argv[i], "polling_time=" ) ) {
            res = sscanf ( argv[i], "polling_time=%d", &polling_time );
            continue;
        }
        if ( strstr ( argv[i], "expire_time=" ) ) {
            res = sscanf ( argv[i], "expire_time=%d", &expire_time );
            continue;
        }
        if ( strstr ( argv[i], "debug" ) ) {
            continue;  /* already parsed: skip */
        }
        if ( strstr ( argv[i], "nodebug" ) ) {
            set_debug_level ( 0 );
            continue;  /* already parsed: skip */
        }
        if ( strstr ( argv[i], "config_file=" ) ) {
            continue; /* already parsed: skip */
        }
        ERR ( "unknown option %s", argv[i] );

        /* arriving here means syntax error */
        printf( "NFC Event Daemon\n" );
        printf( "Usage %s [[no]debug] [[no]daemon] [polling_time=<time>] [expire_time=<limit>] [config_file=<file>]", argv[0] );
        printf( "\nDefaults: debug=0 daemon=0 polltime=%d (ms) expiretime=0 (none) config_file=%s", DEF_POLLING, DEF_CONFIG_FILE );
        exit ( EXIT_FAILURE );
    } /* for */
    /* end of config: return */
    return 0;
}

/**
 * @brief try to find a valid tag
 * @return pointer on a valid tag or NULL.
 */
nfc_target_t*
ned_select_tag(nfc_device_t* nfc_device, nfc_target_t* tag) {
  nfc_target_t* rv = malloc( sizeof(nfc_target_t) );
  nfc_modulation_t nm = {
    .nmt = NMT_ISO14443A,
    .nbr = NBR_106
  };

  if ( tag == NULL ) {
    // We are looking for any tag.
    // Poll for a ISO14443A (MIFARE) tag
    if ( !nfc_initiator_select_passive_target ( nfc_device, nm, NULL, 0, rv ) ) {
      free (rv);
      rv = NULL;
    }
  } else {
    // tag is not NULL, we are looking for specific tag
    // debug_print_tag(tag);
    if ( !nfc_initiator_select_passive_target ( nfc_device, tag->nm, tag->nti.nai.abtUid, tag->nti.nai.szUidLen, rv ) ) {
      free (rv);
      rv = NULL;
    }
  }

  if (rv != NULL) {
    nfc_initiator_deselect_target ( nfc_device );
  }

  return rv;
}

typedef enum {
  NFC_POLL_HARDWARE,
  NFC_POLL_SOFTWARE,
} nfc_poll_mode;

nfc_target_t*
ned_poll_for_tag(nfc_device_t* nfc_device, nfc_target_t* tag)
{
  static nfc_poll_mode mode = NFC_POLL_SOFTWARE;

/*
  // With libnfc 1.6 serie, we could not check the internal chip any more.
  // BTW, "Capabilities" have to be introduced in libnfc, once it will be available we could switch again to a conditional statement
  // http://code.google.com/p/libnfc/issues/detail?id=165
  if((mode == NFC_POLL_HARDWARE) && (nfc_device->nc == NC_PN531)) {
    DBG("%s doesn't support hardware polling, falling back to software polling", nfc_device->acName);
    mode = NFC_POLL_SOFTWARE;
  }
*/

  switch(mode) {
    case NFC_POLL_SOFTWARE:
      sleep ( polling_time );
      return ned_select_tag(nfc_device, tag);
    break;
    case NFC_POLL_HARDWARE: {
        byte_t btPollNr;
        const byte_t btPeriod = 2; /* 2 x 150 ms = 300 ms */
        const nfc_modulation_t nm[1] = { { .nmt = NMT_ISO14443A, .nbr = NBR_106 } };
  
        if( tag != NULL ) {
  	/* We are looking for a previous tag */
          /* In this case, to prevent for intensive polling we add a sleeping time */
          sleep ( polling_time );
          btPollNr = 3; /* Polling duration : btPollNr * szTargetTypes * btPeriod * 150 = btPollNr * 300 = 900 */
        } else {
          /* We are looking for any tag */
          btPollNr = 0xff; /* We endless poll for a new tag */
        }
        nfc_target_t antTargets[2];
        size_t szTargetFound;
  
        bool res = nfc_initiator_poll_targets (nfc_device, nm, 1, btPollNr, btPeriod, antTargets, &szTargetFound);
        if (res) {
          DBG ("%ld target(s) have been found.\n", (unsigned long) szTargetFound);
          if( szTargetFound < 1 ) {
            return NULL;
          } else {
            if ( (tag != NULL) && (0 == memcmp(tag->nti.nai.abtUid, antTargets[0].nti.nai.abtUid, antTargets[0].nti.nai.szUidLen)) ) {
              return tag;
            } else {
              nfc_target_t* rv = malloc(sizeof(nfc_target_t));
              memcpy(rv, &antTargets[0], sizeof(nfc_target_t));
              nfc_initiator_deselect_target ( nfc_device );
              return rv;
            }
          }
        } else {
          ERR ("%s", "Polling failed.");
        }
    }
    break;
  }
  return NULL;
}

int
main ( int argc, char *argv[] ) {
    nfc_target_t* old_tag = NULL;
    nfc_target_t* new_tag;

    int expire_count = 0;

    INFO ("%s", PACKAGE_STRING);

    /* parse args and configuration file */
    parse_args ( argc, argv );

    /* put my self into background if flag is set */
    if ( daemonize ) {
        DBG ( "%s", "Going to be daemon..." );
        if ( daemon ( 0, debug ) < 0 ) {
            ERR ( "Error in daemon() call: %s", strerror ( errno ) );
            return 1;
        }
    }

    load_module();

    /*
     * Wait endlessly for all events in the list of readers
     * We only stop in case of an error
     *
     * COMMENT:
     * There are no way in libnfc API to detect if a card is present or not
     * so the way we proceed is to look for an tag
     * Any ideas will be welcomed
     */
    nfc_device_t* nfc_device = NULL;

//connect:
    // Try to open the NFC device
    if ( nfc_device == NULL ) nfc_device = nfc_connect( nfc_device_desc );
//init:
    if ( nfc_device == NULL ) {
        ERR( "%s", "NFC device not found" );
        exit(EXIT_FAILURE);
    }
    nfc_initiator_init ( nfc_device );

    // Drop the field for a while
    nfc_configure ( nfc_device, NDO_ACTIVATE_FIELD, false );

    nfc_configure ( nfc_device, NDO_INFINITE_SELECT, false );

    // Configure the CRC and Parity settings
    nfc_configure ( nfc_device, NDO_HANDLE_CRC, true );
    nfc_configure ( nfc_device, NDO_HANDLE_PARITY, true );

    // Enable field so more power consuming cards can power themselves up
    nfc_configure ( nfc_device, NDO_ACTIVATE_FIELD, true );

    INFO( "Connected to NFC device: %s", nfc_device->acName, nfc_device );

    do {
detect:
        new_tag = ned_poll_for_tag(nfc_device, old_tag);

        if ( old_tag == new_tag ) { /* state unchanged */
            /* on card not present, increase and check expire time */
            if ( expire_time == 0 ) goto detect;
            if ( new_tag != NULL ) goto detect;
            expire_count += polling_time;
            if ( expire_count >= expire_time ) {
                DBG ( "%s", "Timeout on tag removed " );
                execute_event ( nfc_device, new_tag,EVENT_EXPIRE_TIME );
                expire_count = 0; /*restart timer */
            }
        } else { /* state changed; parse event */
            expire_count = 0;
            if ( new_tag == NULL ) {
                DBG ( "%s", "Event detected: tag removed" );
                execute_event ( nfc_device, old_tag, EVENT_TAG_REMOVED );
                free(old_tag);
            } else {
                DBG ( "%s", "Event detected: tag inserted " );
                execute_event ( nfc_device, new_tag, EVENT_TAG_INSERTED );
            }
            old_tag = new_tag;
        }
    } while ( 1 );
//disconnect:
    if ( nfc_device != NULL ) {
        nfc_disconnect(nfc_device);
        DBG ( "NFC device (0x%08x) is disconnected", nfc_device );
        nfc_device = NULL;
    }

    /* If we get here means that an error or exit status occurred */
    DBG ( "%s", "Exited from main loop" );
    exit ( EXIT_FAILURE );
} /* main */

