/*
 * Copyright (C) 2007-2009, Hewlett-Packard Development Company, LLP
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
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Raja Kumar Thatte <raja-kumar.thatte@hp.com>
 *      Sudesh Acharya <sudesh.acharya@hp.com>
 *      Vivek Kumar <vivek.kumar2@hp.com>
 *      Mohan Devarajulu <mohan@fc.hp.com>
 *
 * This file implements the entry point of the oa soap plug-in. This handles
 * customer handler for oa soap interface for handling the any request for soap
 * interface. This APIs uses soap_open and soap_close APIs from soap interface
 * for initiating and closing the communication with SOAP interface / oa
 *
 *      build_oa_soap_custom_handler()  - Builds the OA SOAP custom handler
 *
 *      oa_soap_open()                  - Opens the oa soap interface connection
 *                                        for initiating communication with OA.
 *
 *      oa_soap_close()                 - Closes the communication with OA.
 *
 *      oa_soap_set_resource_tag()      - Handles setting the tags to resources
 *
 *      oa_soap_set_resource_severity() - Sets the resource's severity.
 *
 *      oa_soap_control_parm()          - Handles the control parameter handling
 *                                        for setting and reading config data
 *                                        from OA
 **/

#include "oa_soap.h"
#include "oa_soap_utils.h"

/* For maintaining the patch versions */
static char const rcsid[] __attribute__ ((used)) =
        "$Version: oa_soap plugin for openhpi-2.11.2, patch level 12. "
        "Created on May 16, 2008 $";

/**
 * build_oa_soap_custom_handler:
 *      @oh_handler: Pointer to OpenHPI handler.
 *
 * Purpose:
 *      Builds and initializes the OA SOAP custom handler.
 *
 * Detailed Description:
 *      - If the plugin initialization fails, then this method will be called
 *        more than once, until the plugin intialization succeeds.
 *      - If the data field in the oh_handler is not NULL then we assume that
 *        this call is done because of some issues with OA / switchover case
 *        This case, we just re-assign the data into oa_handler
 *      - In case of the data field in oh_handler is null, we are filling the
 *        oa_handler information
 *      - Using get_oa_info, we will get the OA information to initialize
 *        SOAP_CON objects.
 *      - Based on the OA info, active OA will be identified, even if user
 *        specifies the same in the configuration file.
 *
 * Return values:
 *      SA_OK                     - on sucess.
 *      SA_ERR_HPI_INVALID_PARAMS - parameter(s) are NULL.
 *      SA_ERR_HPI_OUT_OF_MEMORY  - not enough memory to allocate.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/

SaErrorT build_oa_soap_custom_handler(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;

        if (oh_handler == NULL) {
                err("Invalid parmaters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Check whether oa_soap_handler is initialized or not.
         * If the plugin initialization fails, then this method will be
         * called more than once.
         *
         * The below if statement is to avoid multiple initialization
         * of the oa_soap_handler.
         */
        if (oh_handler->data == NULL) {
                /* Initialize the oa_soap_handler */
                oa_handler = (struct oa_soap_handler *)
                        g_malloc0(sizeof(struct oa_soap_handler));
                if (oa_handler == NULL) {
                        err("out of memory");
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
                oa_handler->status = PRE_DISCOVERY;
                oa_handler->active_con = NULL;
                oa_handler->mutex = g_mutex_new();
                oa_handler->oa_1 = NULL;
                oa_handler->oa_2 = NULL;
                oa_handler->oa_switching=SAHPI_FALSE;
                oa_handler->shutdown_event_thread = SAHPI_FALSE;

                /* Initialize the oa_info structure */
                oa_handler->oa_1 = (struct oa_info *)
                        g_malloc0(sizeof(struct oa_info));
                if (oa_handler->oa_1 == NULL) {
                        err("Out of memory");
                        g_free(oa_handler);
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }

                oa_handler->oa_2 = (struct oa_info *)
                        g_malloc0(sizeof(struct oa_info));
                if (oa_handler->oa_2 == NULL) {
                        err("Out of memory");
                        g_free(oa_handler->oa_1);
                        g_free(oa_handler);
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }

                /* Initialize the oa_1 structure */
                oa_handler->oa_1->oa_status = OA_ABSENT;
                oa_handler->oa_1->hpi_con = NULL;
                oa_handler->oa_1->event_con = NULL;
                oa_handler->oa_1->event_con2 = NULL;
                oa_handler->oa_1->thread_handler = NULL;
                oa_handler->oa_1->mutex = g_mutex_new();
                memset(oa_handler->oa_1->server, 0, MAX_URL_LEN);
		oa_handler->oa_1->oh_handler = oh_handler;

                /* Initialize the oa_2 structure */
                oa_handler->oa_2->oa_status = OA_ABSENT;
                oa_handler->oa_2->hpi_con = NULL;
                oa_handler->oa_2->event_con = NULL;
                oa_handler->oa_2->event_con2 = NULL;
                oa_handler->oa_2->thread_handler = NULL;
                oa_handler->oa_2->mutex = g_mutex_new();
                memset(oa_handler->oa_1->server, 0, MAX_URL_LEN);
		oa_handler->oa_2->oh_handler = oh_handler;

                /* Put the oa_handler in oh_handler */
                oh_handler->data = oa_handler;
        } else {
                /* oa_soap_handler is already initialized
                 * Get the oa_handler from oh_handler
                 */
                oa_handler = (struct oa_soap_handler *) oh_handler->data;
        }

        /* Get the OA information and intialize SOAP_CON structures */
        rv = get_oa_soap_info(oh_handler);
        if (rv != SA_OK) {
                oa_handler->status = PLUGIN_NOT_INITIALIZED;
                err("Get OA SOAP info failed");
                return rv;
        }

        /* Point the active_con to active OA's hpi_con object */
        if (oa_handler->oa_1->oa_status == ACTIVE)
                oa_handler->active_con = oa_handler->oa_1->hpi_con;
        else if (oa_handler->oa_2->oa_status == ACTIVE)
                oa_handler->active_con = oa_handler->oa_2->hpi_con;
        else {
                /* Active OA is not accessible */
                oa_handler->status = PLUGIN_NOT_INITIALIZED;
                err("Active OA is not reachable");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * oa_soap_open:
 *      @handler_config: Handler data pointer.
 *      @handler_id:     Id for the handler
 *      @eventq:         Pointer to the infrstructure event queue.
 *
 * Purpose:
 *      This function opens OA SOAP plugin handler instance.
 *
 * Detailed Description:
 *      - This function will be the entry point for the oa soap plug-in.
 *      - We will check all configuration parameters. We won't validate the
 *        content of the variables. We will check whether the parameters are
 *        assigned or not. User can assign even the empty string into the
 *        configuration file.
 *      - Will assign handler_config, handler id and allocate memory for the RPT
 *        table and cache.
 *      - "build_oa_soap_custom_handler" will be called for initializing the
 *        customer handler address into framework.
 *
 * Return values:
 *      Plugin handle - on success.
 *      NULL          - on error.
 **/

void *oa_soap_open(GHashTable *handler_config,
                   unsigned int handler_id,
                   oh_evt_queue *eventq)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler = NULL;

        if (handler_config == NULL || handler_id == 0 || eventq == NULL) {
                err("Invalid parameters");
                return NULL;
        }

        /* Check whether all the parameters are provided or not */
        rv = check_config_parameters(handler_config);
        if (rv != SA_OK) {
                err("config file has some missing parameters");
                return NULL;
        }

        /* Initialize the handler */
        handler = (struct oh_handler_state *)
                g_malloc0(sizeof(struct oh_handler_state));
        if (handler == NULL) {
                err("Out of memory");
                return NULL;
        }

        handler->config = handler_config;
        handler->hid = handler_id;
        handler->eventq = eventq;
        handler->rptcache = (RPTable *) g_malloc0(sizeof(RPTable));
        if (handler->rptcache == NULL) {
                g_free(handler);
                err("Out of memory");
                return NULL;
        }

        rv = oh_init_rpt(handler->rptcache);
        if (rv != SA_OK) {
                err("Initializing rptcache failed");
                g_free(handler->rptcache);
                g_free(handler);
                return NULL;
        }

        handler->data = NULL;

        /* Build the custom handler for OA SOAP plugin */
        rv = build_oa_soap_custom_handler(handler);
        if (rv != SA_OK) {
                err("Build OA SOAP custom handler failed");
                /* If the failure due to out of memory, return NULL
                 * Else, try to build the oa_soap_handler during discovery call
                 */
                if (rv == SA_ERR_HPI_OUT_OF_MEMORY) {
                        g_free(handler->rptcache);
                        g_free(handler);
                        return NULL;
                }
        }

        return ((void *)handler);
}

/**
 * oa_soap_close:
 *      @oh_handler: Handler data pointer.
 *
 * Purpose:
 *      This function closes OA SOAP plugin handler instance.
 *
 * Detailed Description:
 *      - Releases all the memory allocated by OA SOAP plugin handler
 *      - As per current framework implementation, this api won't be called
 *        during process shutdown as there is no graceful shutdown implemented
 *        as part of the openhpi framework.
 *
 * Return values:
 *      NONE
 **/

void oa_soap_close(void *oh_handler)
{
        struct oh_handler_state *handler = NULL;
        struct oa_soap_handler *oa_handler = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameter");
                return;
        }

        dbg("Shutting down the OA SOAP plugin");
        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;

        /* Check whether oa_handler is initialized or not */
        if (oa_handler == NULL)
                return;

	/* Check whether the oa_1 or oa_2 is NULL
	 * Ideally, if oa_handler is not NULL, oa_1 and oa_2 will not be NULL
	 */
	if (oa_handler->oa_1 == NULL || oa_handler->oa_2 == NULL)
		return;

        /* Set the event thread shutdown status to TRUE */
        oa_handler->shutdown_event_thread = SAHPI_TRUE;

	/* Wait for the event threads to exit */
	if (oa_handler->oa_1->thread_handler != NULL)
		g_thread_join(oa_handler->oa_1->thread_handler);
	if (oa_handler->oa_2->thread_handler != NULL)
		g_thread_join(oa_handler->oa_2->thread_handler);
	dbg("Stopped the OA SOAP event threads");

        /* Cleanup the RPTable */
        cleanup_plugin_rptable(handler);
	g_free(handler->rptcache);
        dbg("Cleaned the OA SOAP RPTable");

        /* Release the mutexes. Check whether the mutex is unlocked or not. If
	 * mutex is not unlocked by the event thread, then g_mutex_free will
	 * crash
         */
	if (oa_handler->mutex != NULL) {
		if (g_mutex_trylock(oa_handler->mutex) == FALSE) {
			err("Mutex in OA handler is not unlocked by the event"
			    " thread");
			err("Mutex in OA handler is not released");
		} else {
			g_mutex_unlock(oa_handler->mutex);
			g_mutex_free(oa_handler->mutex);
		}
	}

	if (oa_handler->oa_1->mutex != NULL) {
		if (g_mutex_trylock(oa_handler->oa_1->mutex) == FALSE) {
			err("Mutex in oa_1 is not unlocked by the event"
			    " thread");
			err("Mutex in oa_1 is not released");
		} else {
			g_mutex_unlock(oa_handler->oa_1->mutex);
			g_mutex_free(oa_handler->oa_1->mutex);
		}
	}

	if (oa_handler->oa_2->mutex != NULL) {
		if (g_mutex_trylock(oa_handler->oa_2->mutex) == FALSE) {
			err("Mutex in oa_2 is not unlocked by the event"
			    " thread");
			err("Mutex in oa_2 is not released");
		} else {
			g_mutex_unlock(oa_handler->oa_2->mutex);
			g_mutex_free(oa_handler->oa_2->mutex);
		}
	}
        dbg("Released the OA SOAP handler mutexes");

        /* Cleanup the SOAP_CON */
        if (oa_handler->oa_1->hpi_con != NULL)
                soap_close(oa_handler->oa_1->hpi_con);
        if (oa_handler->oa_1->event_con != NULL)
                soap_close(oa_handler->oa_1->event_con);
        if (oa_handler->oa_1->event_con2 != NULL)
                soap_close(oa_handler->oa_1->event_con2);
        if (oa_handler->oa_2->hpi_con != NULL)
                soap_close(oa_handler->oa_2->hpi_con);
        if (oa_handler->oa_2->event_con != NULL)
                soap_close(oa_handler->oa_2->event_con);
        if (oa_handler->oa_2->event_con2 != NULL)
                soap_close(oa_handler->oa_2->event_con2);
        dbg("Released the SOAP CON structures from handler");

        /* Release the oa info structure */
        g_free(oa_handler->oa_1);
        g_free(oa_handler->oa_2);
        dbg("Released the oa_info structures from handler");

        /* Release the oa handler structure */
        g_free(oa_handler);
        g_free(handler);
        dbg("Released the OA SOAP handler");

        return;
}

/**
 * oa_soap_set_resource_tag:
 *      @oh_handler:  Pointer to openhpi handler.
 *      @resource_id: Resource Id.
 *      @tag:         Pointer to new tag.
 *
 * Purpose:
 *      Sets resource's tag.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - parameter(s) are NULL @tag is invalid.
 *      SA_ERR_HPI_NOT_PRESENT    - resource does not exist.
 **/

SaErrorT oa_soap_set_resource_tag(void *oh_handler,
                                  SaHpiResourceIdT resource_id,
                                  SaHpiTextBufferT *tag)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiBoolT valid_tag = SAHPI_TRUE;

        if (tag == NULL || oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Validate the tag */
        valid_tag = oh_valid_textbuffer(tag);
        if (valid_tag == SAHPI_FALSE) {
                err("The tag is not correct format");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler =(struct oh_handler_state *) oh_handler;
        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("Invalid resource id");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Copy the tag to the resource tag */
        rv = oh_copy_textbuffer(&(rpt->ResourceTag), tag);
        if (rv != SA_OK) {
                err("Copying textbuffer failed");
                return rv;
        }

        return SA_OK;
}

/**
 * oa_soap_set_resource_severity:
 *      @oh_handler: Handler data pointer.
 *      @id:         Resource Id.
 *      @severity:   Resource severity.
 *
 * Purpose:
 *      Sets severity of the resource.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on invalid parameters.
 *      SA_ERR_HPI_NOT_PRESENT    - resource does not exist.
 **/

SaErrorT oa_soap_set_resource_severity(void *oh_handler,
                                       SaHpiResourceIdT resource_id,
                                       SaHpiSeverityT severity)
{
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Validate the severity */
        if (oh_lookup_severity(severity) == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("Not able to find the resource. Invalid resource id");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        rpt->ResourceSeverity = severity;
        return SA_OK;
}

/**
 * oa_soap_control_parm:
 *      @oh_handler:  Pointer to openhpi handler.
 *      @resource_id: Resource Id.
 *      @action:      Configuration action.
 *
 * Purpose:
 *      Save and restore saved configuration parameters.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API.
 **/

SaErrorT oa_soap_control_parm(void *oh_handler,
                              SaHpiResourceIdT resource_id,
                              SaHpiParmActionT action)
{
        err("oa_soap control parm is not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

void * oh_open (GHashTable *,
                unsigned int,
                oh_evt_queue *)
                __attribute__ ((weak, alias("oa_soap_open")));

void * oh_close (void *)
                __attribute__ ((weak, alias("oa_soap_close")));

void * oh_set_resource_tag (void *,
                            SaHpiResourceIdT,
                            SaHpiTextBufferT *)
                __attribute__ ((weak, alias("oa_soap_set_resource_tag")));

void * oh_set_resource_severity (void *,
                                 SaHpiResourceIdT,
                                 SaHpiSeverityT)
                __attribute__ ((weak, alias("oa_soap_set_resource_severity")));

void * oh_control_parm (void *,
                        SaHpiResourceIdT,
                        SaHpiParmActionT)
                __attribute__ ((weak, alias("oa_soap_control_parm")));

