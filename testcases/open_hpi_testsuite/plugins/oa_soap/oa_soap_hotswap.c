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
 *      Raja Kumar Thatte <raja-kumar.thatte@hp.com>
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *
 * This file handles all the hotswap related event handling
 *
 *      oa_soap_get_hotswap_state()             - gets the hotswap state of the
 *                                                resource
 *
 *      oa_soap_set_hotswap_state()             - sets the hotswap state of the
 *                                                resource
 *
 *      oa_soap_get_indicator_state()           - gets the hotswap LED indicator
 *                                                state of the resource
 *
 *      oa_soap_set_indicator_state()           - sets the hotswap LED indicator
 *                                                state of the resource
 *
 *      oa_soap_request_hotswap_action()        - requests the hotswap action
 *
 *      oa_soap_hotswap_policy_cancel()         - requests hotswap policy cancel
 *
 *      oa_soap_get_autoinsert_timeout()        - gets the auto insert event
 *                                                Timeout period
 *
 *      oa_soap_set_autoinsert_timeout()        - sets the auto insert event
 *                                                Timeout period
 *
 *      oa_soap_get_autoextract_timeout()       - gets the auto extract event
 *                                                Timeout period
 *
 *      oa_soap_set_autoextract_timeout()       - sets the auto extract event
 *                                                Timeout period
 **/

#include "oa_soap_hotswap.h"

/**
 * oa_soap_get_hotswap_state
 *      @oh_handler:  Pointer to openhpi handler structure
 *      @resource_id: Resource id
 *      @state:       Hotswap state
 *
 * Purpose:
 *      Gets the hotswap state of the resource
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                       - on success.
 *      SA_ERR_HPI_INVALID_PARAMS   - on wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - on invalid resource id
 *      SA_ERR_HPI_CAPABILITY       - on not having hotswap capability
 *      SA_ERR_HPI_INTERNAL_ERROR   - on failure.
 **/
SaErrorT oa_soap_get_hotswap_state(void *oh_handler,
                                   SaHpiResourceIdT resource_id,
                                   SaHpiHsStateT *state)
{
        struct oa_soap_hotswap_state *hotswap_state = NULL;
        struct oh_handler_state *handler = NULL;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || state == NULL) {
                err("Invalid parameters");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        handler = (struct oh_handler_state *) oh_handler;
        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("failed to get rpt entry");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
                err("Resource does not have MANAGED_HOTSWAP capability");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Get the hotswap structure from rpt */
        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(handler->rptcache, resource_id);
        if (hotswap_state == NULL) {
                err("Unable to get the resource private data");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        if (hotswap_state->currentHsState == SAHPI_HS_STATE_NOT_PRESENT) {
                /* We can never have any resouce information in RPT with
                 * NOT_PRESENT hotswap state
                 * Ideally, this code should never gets executed
                 */
                return  SA_ERR_HPI_INVALID_RESOURCE;
        }

        *state  = hotswap_state->currentHsState;
        return SA_OK;
}

/**
 * oa_soap_set_hotswap_state
 *      @oh_handler:  Pointer to openhpi handler structure
 *      @resource_id: Resource id
 *      @state:       Hotswap state
 *
 * Purpose:
 *      Sets the hotswap state of the resource
 *
 * Detailed Description:
 *      Currently, the OA plug-in does not stay in the InsertionPending or
 *      ExtractionPending states.  Because of this, the ActiveSet() and
 *      InactiveSet() will always be an invalid request, as per the HPI
 *      specification.
 *
 *      As it turns out, the current infrastructure code does not even call
 *      this plug-in routine.  However, if it's ever called, we need to be
 *      returning the correct values.
 *
 * Return value:
 *      SA_ERR_HPI_INVALID_REQUEST  - We're not in one of the pending states
 **/
SaErrorT oa_soap_set_hotswap_state(void *oh_handler,
                                   SaHpiResourceIdT resource_id,
                                   SaHpiHsStateT state)
{
        return SA_ERR_HPI_INVALID_REQUEST;
}


/**
 * oa_soap_get_indicator_state
 *      @oh_handler:  Pointer to openhpi handler structure
 *      @resource_id: Resource id
 *      @state:       Hotswap state
 *
 * Purpose:
 *      Gets the hotswap LED indicator state of the resource
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                       - on success.
 *      SA_ERR_HPI_INVALID_PARAMS   - on wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - on invalid resource id
 *      SA_ERR_HPI_CAPABILITY       - on not having hotswap capability
 *      SA_ERR_HPI_INTERNAL_ERROR   - on failure.
 **/
SaErrorT oa_soap_get_indicator_state(void *oh_handler,
                                     SaHpiResourceIdT resource_id,
                                     SaHpiHsIndicatorStateT *state)
{
        err("oa_soap_get_indicator_state not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_set_indicator_state
 *      @oh_handler:  Pointer to openhpi handler structure
 *      @resource_id: Resource id
 *      @state:       Hotswap state
 *
 * Purpose:
 *      Sets the hotswap LED indicator state of the resource
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                       - on success.
 *      SA_ERR_HPI_INVALID_PARAMS   - on wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - on invalid resource id
 *      SA_ERR_HPI_CAPABILITY       - on not having hotswap capability
 *      SA_ERR_HPI_INTERNAL_ERROR   - on failure.
 **/
SaErrorT oa_soap_set_indicator_state(void *oh_handler,
                                     SaHpiResourceIdT resource_id,
                                     SaHpiHsIndicatorStateT state)
{
        err("oa_soap_set_indicator_state not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_request_hotswap_action
 *      @oh_handler:  Pointer to openhpi handler structure
 *      @resource_id: Resource id
 *      @action:      Hotswap action
 *
 * Purpose:
 *      Requests the hotswap action
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                       - on success.
 *      SA_ERR_HPI_INVALID_PARAMS   - on wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - on invalid resource id
 *      SA_ERR_HPI_CAPABILITY       - on not having hotswap capability
 *      SA_ERR_HPI_INTERNAL_ERROR   - on failure.
 **/
SaErrorT oa_soap_request_hotswap_action(void *oh_handler,
                                        SaHpiResourceIdT resource_id,
                                        SaHpiHsActionT action)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_hotswap_state *hotswap_state = NULL;
        struct oh_handler_state *handler = NULL;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        char *type = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;
        rv = lock_oa_soap_handler(oa_handler);
        if (rv != SA_OK) {
                err("OA SOAP handler is locked");
                return rv;
        }

        /* Check whether hotswap action is corrrect or not */
        type = oh_lookup_hsaction(action);
        if (type == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("Failed to get rpt entry");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        /* Check whether the resource has managed hotswap capability */
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
                err("Resource does not have MANAGED_HOTSWAP capability");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Get the hotswap structure from rpt entry */
        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(handler->rptcache, resource_id);
        if (hotswap_state == NULL) {
                err("Unable to get the resource private data");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        switch (action) {
                case SAHPI_HS_ACTION_INSERTION:
                        /* Check whether resource is in INACTIVE state
                         * Setting to INSERTION state is possible
                         * when theresource is in INACTIVE state
                         */
                        if (hotswap_state->currentHsState ==
                            SAHPI_HS_STATE_INACTIVE) {
                                rv = oa_soap_set_power_state(
                                        oh_handler,
                                        resource_id,
                                        SAHPI_POWER_ON);
                        }
                        else {
                                err("Setting to INSERTION state is possible "
                                    "when theresource is in INACTIVE state.");
                                err("The resource is not in INACTIVE state");
                                rv = SA_ERR_HPI_INVALID_REQUEST;
                        }
                        break;

                case SAHPI_HS_ACTION_EXTRACTION:
                        /* Check whether resource is in ACTIVE state
                         * Setting to EXTRACTION state is possible
                         * when theresource is in ACTIVE state
                         */
                        if (hotswap_state->currentHsState ==
                                SAHPI_HS_STATE_ACTIVE) {
                                rv = oa_soap_set_power_state(
                                        oh_handler,
                                        resource_id,
                                        SAHPI_POWER_OFF);
                        }
                        else {
                                err("Setting to EXTRACTION state is possible "
                                    "when theresource is in ACTIVE state.");
                                err("The resource is not in ACTIVE state");
                                rv = SA_ERR_HPI_INVALID_REQUEST;
                        }
                        break;

                default:
                        err("Invalid parameter");
                        rv = SA_ERR_HPI_INVALID_PARAMS;
        }
        return SA_OK;
}

/**
 * oa_soap_hotswap_policy_cancel
 *      @oh_handler:  Pointer to openhpi handler structure
 *      @resource_id: Resource id
 *      @tm:          Timeout value
 *
 * Purpose:
 *      Requests hotswap policy cancel
 *
 * Detailed Description:
 *      Currently, the OA plug-in does not stay in the InsertionPending or
 *      ExtractionPending states.  Because of this, the policy_cancel request
 *      will always be an invalid request, as per the HPI specification.
 *
 *      As it turns out, the current infrastructure code does not even call
 *      this plug-in routine.  However, if it's ever called, we need to be
 *      returning the correct values.
 *
 * Return value:
 *      SA_ERR_HPI_INVALID_REQUEST  - We're not in one of the pending states
 **/
SaErrorT oa_soap_hotswap_policy_cancel(void *oh_handler,
                                       SaHpiResourceIdT resource_id,
                                       SaHpiTimeoutT timeout)
{
        return SA_ERR_HPI_INVALID_REQUEST;
}

/**
 * oa_soap_get_autoinsert_timeout:
 *      @oh_handler:  Handler data pointer.
 *      @resource_id: Resource ID.
 *      @timeout:     Timeout to set.
 *
 * Purpose:
 *      Get hotswap autoinsert timeout.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success
 *      SAHPI_TIMEOUT_IMMEDIATE   - autonomous handling is immediate
 *      SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oa_soap_get_autoinsert_timeout(void *oh_handler,
                                        SaHpiResourceIdT resource_id,
                                        SaHpiTimeoutT timeout)
{
        err("oa_soap_get_autoinsert_timeout is not supported");
        err("Default auto insert timeout is IMMEDIATE and read only");
        return SAHPI_TIMEOUT_IMMEDIATE;
}

/**
 * oa_soap_set_autoinsert_timeout:
 *      @oh_handler: Handler data pointer.
 *      @timeout:    Timeout to set.
 *
 * Purpose:
 *      Set hotswap autoinsert timeout.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success
 *      SA_ERR_HPI_READ_ONLY      - auto-insert timeout is fixed
 *      SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oa_soap_set_autoinsert_timeout(void *oh_handler,
                                        SaHpiTimeoutT timeout)
{
        err("oa_soap_set_autoinsert_timeout is not supported");
        err("Default auto insert timeout is IMMEDIATE and read only");
        return SA_ERR_HPI_READ_ONLY;
}

/**
 * oa_soap_get_autoextract_timeout:
 *      @oh_handler:  Handler data pointer.
 *      @resource_id: Resource ID.
 *      @timeout:     Timeout value.
 *
 * Purpose:
 *      Get a resource's hotswap autoextract timeout.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - Normal case.
 *      SAHPI_TIMEOUT_IMMEDIATE   - autonomous handling is immediate
 *      SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oa_soap_get_autoextract_timeout(void *oh_handler,
                                         SaHpiResourceIdT resource_id,
                                         SaHpiTimeoutT *timeout)
{
        err("oa_soap_get_autoextract_timeout is not supported");
        err("Default auto extract timeout is IMMEDIATE and read only");
        return SAHPI_TIMEOUT_IMMEDIATE;
}

/**
 * oa_soap_set_autoextract_timeout:
 *      @oh_handler:  Handler data pointer.
 *      @resource_id: Resource ID.
 *      @timeout:     Timeout to set.
 *
 * Purpose:
 *      Set a resource hotswap autoextract timeout.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success
 *      SA_ERR_HPI_READ_ONLY      - auto-insert timeout is fixed
 *      SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oa_soap_set_autoextract_timeout(void *oh_handler,
                                         SaHpiResourceIdT resource_id,
                                         SaHpiTimeoutT timeout)
{
        err("oa_soap_set_autoextract_timeout is not supported");
        err("Default auto extract timeout is IMMEDIATE and read only");
        return SA_ERR_HPI_READ_ONLY;
}

void * oh_get_hotswap_state (void *, SaHpiResourceIdT, SaHpiHsStateT *)
                __attribute__ ((weak, alias("oa_soap_get_hotswap_state")));

void * oh_set_hotswap_state (void *, SaHpiResourceIdT, SaHpiHsStateT)
                __attribute__ ((weak, alias("oa_soap_set_hotswap_state")));

void * oh_request_hotswap_action (void *, SaHpiResourceIdT, SaHpiHsActionT)
                __attribute__ ((weak, alias("oa_soap_request_hotswap_action")));

void * oh_hotswap_policy_cancel (void *, SaHpiResourceIdT, SaHpiTimeoutT)
                __attribute__ ((weak, alias("oa_soap_hotswap_policy_cancel")));

void * oh_get_indicator_state (void *, SaHpiResourceIdT,
                               SaHpiHsIndicatorStateT *)
                __attribute__ ((weak, alias("oa_soap_get_indicator_state")));

void * oh_set_indicator_state (void *, SaHpiResourceIdT, SaHpiHsIndicatorStateT)
                __attribute__ ((weak, alias("oa_soap_set_indicator_state")));

void * oh_get_autoinsert_timeout (void *, SaHpiResourceIdT, SaHpiTimeoutT)
                __attribute__ ((weak, alias("oa_soap_get_autoinsert_timeout")));

void * oh_set_autoinsert_timeout (void *, SaHpiTimeoutT)
                __attribute__ ((weak, alias("oa_soap_set_autoinsert_timeout")));

void * oh_get_autoextract_timeout (void *, SaHpiResourceIdT, SaHpiTimeoutT *)
                __attribute__ ((weak,
                               alias("oa_soap_get_autoextract_timeout")));

void * oh_set_autoextract_timeout (void *, SaHpiResourceIdT, SaHpiTimeoutT)
                __attribute__ ((weak,
                               alias("oa_soap_set_autoextract_timeout")));

