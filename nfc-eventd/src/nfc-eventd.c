/*
    Generate events on tag status change
    Copyrigt (C) 2009 Romuald Conty <rconty@il4p.fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#include <libnfc/libnfc.h>

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

#define DEF_POLLING 1    /* 1 second timeout */
#define DEF_EXPIRE 0    /* no expire */
#define DEF_CONFIG_FILE "/etc/nfc-eventd.conf"

typedef enum {
	TAG_ERROR = -1,
	TAG_NOT_PRESENT = 0,
	TAG_PRESENT = 1,
} tag_state_t;

int polling_time;
int expire_time;
int daemonize;
int debug;
char *cfgfile;

nfcconf_context *ctx;
const nfcconf_block *root;

typedef struct slot_st slot_t;

static int execute_event ( dev_info *nfc_device, const nem_event_t event )
{
	nfcconf_block **module_list, *my_module;

	module_list = nfcconf_find_blocks ( ctx, root, "module", NULL );
	if ( !module_list ) {
		ERR ( "Module item not found." );
		return -1;
	}
	my_module = module_list[0];
	free ( module_list );
	if ( !my_module ) {
		ERR ( "Module item not found." );
		return -1;
	}
	DBG("Loading module: '%s'...", my_module->name->data);
	char module_path[256]={ '\0', };
	strcat(module_path, "/usr/lib/nfc-eventd/");
	strcat(module_path, "modules/");
	strcat(module_path, my_module->name->data);
	strcat(module_path, ".so");
	DBG("Module found at: '%s'...", module_path);

	void *module_handler;
	module_handler = dlopen(module_path,RTLD_LAZY);
	if ( module_handler == NULL ){
		ERR("Unable to open module: %s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	char module_fct_name[256];
	char *error;

	module_fct_name[0]='\0';
	strcat(module_fct_name,my_module->name->data);
	strcat(module_fct_name,"_init");

	void (*module_init_fct_ptr)(nfcconf_context*, nfcconf_block*);
	module_init_fct_ptr = dlsym(module_handler,module_fct_name);

	if ((error = dlerror()) != NULL) {
		fprintf (stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	module_fct_name[0]='\0';
	strcat(module_fct_name,my_module->name->data);
	strcat(module_fct_name,"_event_handler");
	int (*module_event_handler_fct_ptr)( dev_info*, const nem_event_t );

	module_event_handler_fct_ptr = dlsym(module_handler,module_fct_name);
	if ((error = dlerror()) != NULL) {
		fprintf (stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	(*module_init_fct_ptr)( ctx, my_module );
	(*module_event_handler_fct_ptr)( nfc_device, event );

	return 0;
}

static int parse_config_file()
{
	ctx = nfcconf_new ( cfgfile );
	if ( !ctx ) {
		DBG ( "Error creating conf context" );
		return -1;
	}
	if ( nfcconf_parse ( ctx ) <= 0 ) {
		DBG ( "Error parsing file '%s'", cfgfile );
		return -1;
	}
	/* now parse options */
	root = nfcconf_find_block ( ctx, NULL, "nfc-eventd" );
	if ( !root ) {
		DBG ( "nfc-eventd block not found in config: '%s'", cfgfile );
		return -1;
	}
	debug = nfcconf_get_bool ( root, "debug", debug );
	daemonize = nfcconf_get_bool ( root, "daemon", daemonize );
	polling_time = nfcconf_get_int ( root, "polling_time", polling_time );
	expire_time = nfcconf_get_int ( root, "expire_time", expire_time );

	if ( debug ) set_debug_level ( 1 );
	return 0;
}

static int parse_args ( int argc, char *argv[] )
{
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
		fprintf ( stderr, "Error parsing configuration file %s\n", cfgfile );
		exit ( -1 );
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
		fprintf ( stderr, "unknown option %s\n", argv[i] );
		/* arriving here means syntax error */
		fprintf ( stderr, "NFC Event Daemon\n\n" );
		fprintf ( stderr, "Usage %s [[no]debug] [[no]daemon] [polling_time=<time>] [expire_time=<limit>] [config_file=<file>]\n", argv[0] );
		fprintf ( stderr, "\n\nDefaults: debug=0 daemon=0 polltime=%d (ms) expiretime=0 (none) config_file=%s\n", DEF_POLLING, DEF_CONFIG_FILE );
		exit ( 1 );
	} /* for */
	/* end of config: return */
	return 0;
}

/*
* try to find a valid tag
* returns TAG_PRESENT, TAG_NOT_PRESENT or TAG_ERROR
*/
tag_state_t
nfc_get_tag_state(dev_info* nfc_device)
{
	tag_state_t rv;

	tag_info ti;

	// Poll for a ISO14443A (MIFARE) tag
	if ( nfc_initiator_select_tag ( nfc_device, IM_ISO14443A_106, NULL, 0, &ti ) ) {
/*
		printf ( "The following (NFC) ISO14443A tag was found:\n\n" );
		printf ( "    ATQA (SENS_RES): " ); print_hex ( ti.tia.abtAtqa,2 );
		printf ( "       UID (NFCID%c): ", ( ti.tia.abtUid[0]==0x08?'3':'1' ) ); print_hex ( ti.tia.abtUid,ti.tia.uiUidLen );
		printf ( "      SAK (SEL_RES): " ); print_hex ( &ti.tia.btSak,1 );
		if ( ti.tia.uiAtsLen )
		{
			printf ( "          ATS (ATR): " );
			print_hex ( ti.tia.abtAts,ti.tia.uiAtsLen );
		}
*/
		uint32_t uiPos;
		char *uid = malloc(ti.tia.uiUidLen*sizeof(char));
		char *uid_ptr = uid;
		for (uiPos=0; uiPos < ti.tia.uiUidLen; uiPos++)
		{
			sprintf(uid_ptr, "%02x",ti.tia.abtUid[uiPos]);
			uid_ptr += 2;
		}
		uid_ptr[0]='\0';

		DBG( "ISO14443A (MIFARE) tag found: %s", uid );
		free(uid);
		nfc_initiator_deselect_tag ( nfc_device );
		sleep ( 1 );
		rv = TAG_PRESENT;
	} else {
// 		DBG( "ISO14443A (MIFARE) tag not found." );
		rv = TAG_NOT_PRESENT;
	}
	return rv;
}

int
main ( int argc, char *argv[] )
{
	int rv;

	int first_loop   = 0;
	int old_state    = TAG_NOT_PRESENT;
	int new_state    = TAG_NOT_PRESENT;
	int expire_count = 0;

	/* parse args and configuration file */
	parse_args ( argc, argv );

	/* put my self into background if flag is set */
	if ( daemonize ) {
		DBG ( "Going to be daemon..." );
		if ( daemon ( 0, debug ) < 0 ) {
			DBG ( "Error in daemon() call: %s", strerror ( errno ) );
			return 1;
		}
	}

	/*
	 * Wait endlessly for all events in the list of readers
	 * We only stop in case of an error
	 *
	 * COMMENT:
	 * There are no way in libnfc API to detect if a card is present or no
	 * so the way we proced is to look for an slot whit available token
	 * Any ideas will be wellcomed
	 *
	 */
	dev_info* nfc_device = NULL;

connect:
	// Try to open the NFC device
	if( nfc_device == NULL ) nfc_device = nfc_connect(NULL);
init:
	if ( nfc_device == INVALID_DEVICE_INFO ) {
		DBG( "NFC device not found" );
		return ( TAG_ERROR );
	}
	nfc_initiator_init ( nfc_device );

	// Drop the field for a while
	nfc_configure ( nfc_device, DCO_ACTIVATE_FIELD, false );

	// Let the reader only try once to find a tag
	nfc_configure ( nfc_device, DCO_INFINITE_SELECT, false );

	// Configure the CRC and Parity settings
	nfc_configure ( nfc_device, DCO_HANDLE_CRC, true );
	nfc_configure ( nfc_device, DCO_HANDLE_PARITY, true );

	// Enable field so more power consuming cards can power themselves up
	nfc_configure ( nfc_device, DCO_ACTIVATE_FIELD, true );

	DBG( "Connected to NFC device: %s (0x%08x)", nfc_device->acName, nfc_device );

	do {
detect:
		sleep ( polling_time );
		new_state = nfc_get_tag_state(nfc_device);
		switch(new_state) {
			case TAG_NOT_PRESENT:
				DBG("new_state == TAG_NOT_PRESENT");
				break;
			case TAG_PRESENT:
				DBG("new_state == TAG_PRESENT");
				break;
			case TAG_ERROR:
				DBG("new_state == TAG_PRESENT");
				break;
		}

		if ( new_state == TAG_ERROR ) {
			ERR ( "Error trying to get a tag" );
			break;
		}
		if ( old_state == new_state ) { /* state unchanged */
			/* on card not present, increase and check expire time */
			if ( expire_time == 0 ) goto detect;
			if ( new_state == TAG_PRESENT ) goto detect;
			expire_count += polling_time;
			if ( expire_count >= expire_time ) {
				DBG ( "Timeout on tag removed " );
				execute_event ( nfc_device, EVENT_EXPIRE_TIME );
				expire_count = 0; /*restart timer */
			}
		} else { /* state changed; parse event */
			old_state = new_state;
			expire_count = 0;
// 			if ( !first_loop++ ) continue; /*skip first pass */
			if ( new_state == TAG_NOT_PRESENT ) {
				DBG ( "Tag removed" );
				execute_event ( nfc_device, EVENT_TAG_REMOVED );
			}
			if ( new_state == TAG_PRESENT ) {
				DBG ( "Tag inserted " );
				execute_event ( nfc_device, EVENT_TAG_INSERTED );
			}
		}
	} while ( 1 );
disconnect:
	if ( nfc_device != NULL ) {
		nfc_disconnect(nfc_device);
		DBG ( "NFC device (0x%08x) is disconnected", nfc_device );
		nfc_device = NULL;
	}

	/* If we get here means that an error or exit status occurred */
	DBG ( "Exited from main loop" );
	exit ( EXIT_FAILURE );
} /* main */

