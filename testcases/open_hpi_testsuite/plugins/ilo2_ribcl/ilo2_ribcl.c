/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *     Shuah Khan <shuah.khan@hp.com>
 *     Richard White <richard.white@hp.com>
 */

/***************
 * This source file contains open, close, get_event, and discover_resources
 * HPI ABI routines iLO2 RIBCL plug-in implements. Other source files will 
 * provide support functionality for these ABIs.
***************/

#include <ilo2_ribcl.h>
#include <oh_ssl.h>
#include <ilo2_ribcl_ssl.h>
#include <ilo2_ribcl_xml.h>
#include <ilo2_ribcl_discover.h>
#include <ilo2_ribcl_sensor.h>

static SaHpiEntityPathT g_epbase; /* root entity path (from config) */

/*****************************
	iLO2 RIBCL plug-in ABI Interface functions
*****************************/

/**
 * ilo2_ribcl_open: open (initiate) instance of the iLO2 RIBCL plug-in
 * @handler_config: Pointer to openhpi config file.
 *
 * This function opens an instance of the iLO2 RIBCL plugin.
 * Detailed description:
 * 	- Reads iLO2 IP address and hostname from the configfile hash
 * 	- Reads iLO2 user name and password from the configfile hash.
 * 	- Intializes plugin internal data structures. Allocates memory
 * 	  for RIBCL send/receive buffers.
 * 	- Initilaizes iLO2 RIBCL SSL module to communicate with iLO2.
 * 	- Error handling: Frees allocated memory (if any) before returning.
 * 
 * Return values:
 * 	Plugin handle - normal operation.
 * 	NULL - on error.
 **/
void *ilo2_ribcl_open(GHashTable *handler_config,
                             unsigned int hid,
                             oh_evt_queue *eventq)
{
	struct oh_handler_state *oh_handler = NULL;
	ilo2_ribcl_handler_t *ilo2_ribcl_handler = NULL;
	char *ilo2_hostname = NULL;
	char *ilo2_port_str = NULL;
	char *ilo2_user_name = NULL;
	char *ilo2_password = NULL;
	char *entity_root = NULL;
	int host_len = 0;
	int port_len = 0;
	int temp_len = 0;

#ifdef ILO2_RIBCL_SIMULATE_iLO2_RESPONSE
	char *d_responsefile;
	size_t fnamesize;
#endif /* #ifdef ILO2_RIBCL_SIMULATE_iLO2_RESPONSE */

	/* check input parameters */
	if (!handler_config) {
		err("ilo2_ribcl Open:No config file provided.");
		return(NULL);
	} else if (!hid) {
		err("ilo2 ribcl Open:Bad handler id passed.");
		return NULL;
	} else if (!eventq) {
		err("ilo2 ribcl Open:No event queue was passed.");
		return NULL;
	}

	/* set up entity root in g_epbase */
	entity_root = (char *)g_hash_table_lookup(handler_config,
		"entity_root");
	if(!entity_root) {
		err("ilo2 ribcl Open:entity root is not present");
		return(NULL);
	}
	oh_encode_entitypath(entity_root, &g_epbase);

	/* read hostname, port string user_name, and password from the 
	   config file */
	ilo2_hostname = (char *)g_hash_table_lookup(handler_config,
		"ilo2_ribcl_hostname");
	if(!ilo2_hostname) {
		err("ilo2 ribcl Open:ilo2_ribcl_hostname is not specified in the config file");
		return(NULL);
	}
	host_len = strlen(ilo2_hostname);
	if((host_len < ILO2_HOST_NAME_MIN_LEN) ||
		(host_len > ILO2_HOST_NAME_MAX_LEN)) {
		err("ilo2 ribcl Open: Invalid iLO2 IP address");
		return(NULL);
	}

	ilo2_port_str = (char *)g_hash_table_lookup(handler_config,
		"ilo2_ribcl_portstr");
	if(!ilo2_port_str) {
		err("ilo2 ribcl Open:ilo2_ribcl_port_str is not specified in the config file");
		return(NULL);
	} else if((port_len = strlen(ilo2_port_str)) < ILO2_MIN_PORT_STR_LEN) {
		err("ilo2 ribcl Open:Invalid iLO2 port");
		return(NULL);
	}

	ilo2_user_name = (char *)g_hash_table_lookup(handler_config,
		"ilo2_ribcl_username");
	if(!ilo2_user_name) {
		err("ilo2 ribcl Open:ilo2_ribcl_username is not specified in the config file");
		return(NULL);
	}
	temp_len = strlen(ilo2_user_name);
	if(temp_len > ILO2_RIBCL_USER_NAME_MAX_LEN) {
		err("ilo2 ribcl Open:Invalid ilo2_ribcl_username - too long");
		return(NULL);
	} else if(temp_len < ILO2_RIBCL_USER_NAME_MIN_LEN) {
		err("ilo2 ribcl Open:Invalid ilo2_ribcl_username - too short");
		return(NULL);
	}

	ilo2_password = (char *)g_hash_table_lookup(handler_config,
		"ilo2_ribcl_password");
	if(!ilo2_password) {
		err("ilo2 ribcl Open:ilo2_ribcl_password is not specified in the config file");
		return(NULL);
	}
	temp_len = strlen(ilo2_password);
	if(temp_len > ILO2_RIBCL_PASSWORD_MAX_LEN) {
		err("ilo2 ribcl Open:Invalid ilo2_ribcl_password - too long");
		return(NULL);
	} else if(temp_len < ILO2_RIBCL_PASSWORD_MIN_LEN) {
		err("ilo2 ribcl Open:Invalid ilo2_ribcl_password - too short");
		return(NULL);
	}

	/* allocate main handler and initialize it */
	oh_handler = malloc(sizeof(*oh_handler));
	if(!oh_handler) {
		err("ilo2 ribcl Open:unable to allocate main handler");
		return(NULL);
	}
	memset(oh_handler, '\0', sizeof(*oh_handler));

	/* assign config to handler_config and initialize rptcache */
	oh_handler->config = handler_config;

	oh_handler->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));
	if(!oh_handler->rptcache) {
		err("ilo2 ribcl Open:unable to allocate RPT cache");
		free(oh_handler);
		return(NULL);
	}

	oh_handler->hid = hid;
	oh_handler->eventq = eventq;

	/* allocate memory for ilo2 ribcl private handler */
	ilo2_ribcl_handler = malloc(sizeof(*ilo2_ribcl_handler));
	if(!ilo2_ribcl_handler) {
		err("ilo2 ribcl Open:unable to allocate main handler");
		free(oh_handler->rptcache);
		free(oh_handler);
		return(NULL);
	}
	memset(ilo2_ribcl_handler, '\0', sizeof(*ilo2_ribcl_handler));
	oh_handler->data = ilo2_ribcl_handler;

	/* Save configuration in the handler */
	ilo2_ribcl_handler->entity_root = entity_root;

	/* build complete hostname with port string appended */
	/* add one extra byte to account for : in the middle of hostname:port
	   string example: 10.100.1.1:443 */
	ilo2_ribcl_handler->ilo2_hostport = g_malloc(host_len+port_len+2);
	if(ilo2_ribcl_handler->ilo2_hostport == NULL) {
		err("ilo2 ribcl Open:unable to allocate memory");
		free(ilo2_ribcl_handler);
		free(oh_handler->rptcache);
		free(oh_handler);
		return(NULL);
	}
	snprintf(ilo2_ribcl_handler->ilo2_hostport,
		(host_len+port_len+2), "%s:%s",
		ilo2_hostname, ilo2_port_str);

	ilo2_ribcl_handler->user_name = ilo2_user_name;
	ilo2_ribcl_handler->password = ilo2_password;

	/* Build the customized RIBCL command strings containing the
	 * login and password for this ilo2 host */

	if (ir_xml_build_cmdbufs( ilo2_ribcl_handler) != RIBCL_SUCCESS){
		err("ilo2_ribcl_open(): ir_xml_build_cmdbufs failed to build buffers.");
		free(ilo2_ribcl_handler->ilo2_hostport);
		free(ilo2_ribcl_handler);
		free(oh_handler->rptcache);
		free(oh_handler);
		return(NULL);
	}

#ifdef ILO2_RIBCL_SIMULATE_iLO2_RESPONSE

	/* Check if a iLO2 response file should be used for discovery testing.
	 * We will use the contents of this file as the command response, 
	 * rather than communucating with an actual iLO2. */

	d_responsefile = (char *)g_hash_table_lookup(handler_config,
		"discovery_responsefile");
	if(  d_responsefile){

		fnamesize = strlen( d_responsefile) + 1;
		ilo2_ribcl_handler->discovery_responsefile = malloc( fnamesize);

		if( ilo2_ribcl_handler->discovery_responsefile == NULL){
			err("ilo2_ribcl_open(): allocation for discovery_responsefile failed.");
		} else {
			strncpy( ilo2_ribcl_handler->discovery_responsefile,
				 d_responsefile, fnamesize);
		}
	}

#endif /* ILO2_RIBCL_SIMULATE_iLO2_RESPONSE */

	/* initialize SSL */
	ilo2_ribcl_handler->ssl_ctx = oh_ssl_ctx_init();
	if(ilo2_ribcl_handler->ssl_ctx == NULL) {
		err("ilo2_ribcl_open(): failed to initialize ssl connection to %s",
			ilo2_ribcl_handler->ilo2_hostport);
		free(ilo2_ribcl_handler->ilo2_hostport);
		free(ilo2_ribcl_handler);
		free(oh_handler->rptcache);
		free(oh_handler);
		return(NULL);
	}

	/* Initialize sensor data */
	ilo2_ribcl_init_sensor_data( ilo2_ribcl_handler);

	return((void *)oh_handler);
}

/**
 * ilo2_ribcl_close:
 * @oh_handler: Handler data pointer.
 *
 * This function closes the instance of the iLO2 RIBCL plugin specified 
 * by the oh_handler input parameter.
 * Detailed description:
 *	- Free allocated memory
 *	- Assumption: RIBCL connection is closed after each transaction.
 *	  If this assumption is incorrect, close open SSL connections to
 *	  iLO2.
 * Return values:
 * Void
 **/
void ilo2_ribcl_close(void *handler)
{
	struct oh_handler_state *oh_handler =
		(struct oh_handler_state *) handler;
        ilo2_ribcl_handler_t *ilo2_ribcl_handler;

        if(oh_handler == NULL) {
                return;
        }

        ilo2_ribcl_handler = (ilo2_ribcl_handler_t *) oh_handler->data;
	if(ilo2_ribcl_handler == NULL) {
        	free(oh_handler);
                return;
	}

	/* Free SSL infrastructure */
	oh_ssl_ctx_free(ilo2_ribcl_handler->ssl_ctx);

	/* Free the RIBCL command strings in the ilo2_ribcl_handler */
	ir_xml_free_cmdbufs( ilo2_ribcl_handler);

	/* Free any allocated discovery data in the ilo2_ribcl_handler */
	ilo2_ribcl_free_discoverydata( ilo2_ribcl_handler);

#ifdef ILO2_RIBCL_SIMULATE_iLO2_RESPONSE
	/* If we specified a response file for discovery testing,
	 * free the space we used for its namestring. */
 
	if( ilo2_ribcl_handler->discovery_responsefile){
		free( ilo2_ribcl_handler->discovery_responsefile);
	}
#endif /* ILO2_RIBCL_SIMULATE_iLO2_RESPONSE */

	oh_flush_rpt(oh_handler->rptcache);
	free(ilo2_ribcl_handler->ilo2_hostport);
	free(ilo2_ribcl_handler);
        free(oh_handler->rptcache);
        free(oh_handler);

        return;
}



/**
 * ilo2_ribcl_get_event:
 * @hnd: Handler data pointer.
 * @event: Infra-structure event pointer. 
 *
 * Passes plugin events up to the infra-structure for processing.
 *
 * Return values:
 * 1 - events to be processed.
 * SA_OK - No events to be processed.
 * SA_ERR_HPI_INVALID_PARAMS - @event is NULL.
 **/
SaErrorT ilo2_ribcl_get_event(void *handler)
{

        struct oh_handler_state *oh_handler =
		(struct oh_handler_state *) handler;
        ilo2_ribcl_handler_t *ilo2_ribcl_handler;
	
        if (!handler) {
                err("ilo2 ribcl get event: Invalid parameter");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        ilo2_ribcl_handler = (ilo2_ribcl_handler_t *) oh_handler->data;
	if(! ilo2_ribcl_handler) {
		err("ilo2 ribcl get event: Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
        if (g_slist_length(ilo2_ribcl_handler->eventq) > 0) {
                struct oh_event *e = ilo2_ribcl_handler->eventq->data;
                e->hid = oh_handler->hid;
                oh_evt_queue_push(oh_handler->eventq, e);
                ilo2_ribcl_handler->eventq = 
			g_slist_remove_link(ilo2_ribcl_handler->eventq,
				ilo2_ribcl_handler->eventq);
                return(ILO2_RIBCL_EVENTS_PENDING);
        } 

	/* No events for infrastructure to process */
	return(SA_OK);
}

/*****************************
	OpenHPI plug-in to iLO2 RIBCL plug-in ABI function mapping
*****************************/
void * oh_open (GHashTable *, unsigned int, oh_evt_queue *) __attribute__ ((weak, alias("ilo2_ribcl_open")));

void * oh_close (void *) __attribute__ ((weak, alias("ilo2_ribcl_close")));

void * oh_get_event (void *) 
                __attribute__ ((weak, alias("ilo2_ribcl_get_event")));
