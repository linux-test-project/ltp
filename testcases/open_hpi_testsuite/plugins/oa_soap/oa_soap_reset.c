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
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 *
 *
 * This file handles all the resource reset states related apis.
 *
 *      oa_soap_get_reset_state()   - API to return the reset state of chassis
 *                                    components
 *
 *      oa_soap_set_reset_state()   - API to set the reset state for all the
 *                                    chassis components
 **/

#include "oa_soap_reset.h"

/**
 * oa_soap_get_reset_state
 *      @oh_handler:  Pointer to openhpi handler
 *      @resource_id: Resource id
 *      @action:      Pointer to reset action
 *
 * Purpose:
 *      gets the reset state of the chassis resource
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT oa_soap_get_reset_state(void *oh_handler,
                                 SaHpiResourceIdT resource_id,
                                 SaHpiResetActionT *action)
{
        SaErrorT rv = SA_OK;
        SaHpiPowerStateT state;
        struct oh_handler_state *handler = NULL;

        if (oh_handler == NULL || action == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;

        /* Get the current power state of the resource */
        rv = oa_soap_get_power_state(oh_handler, resource_id, &state);
        if (rv != SA_OK) {
                err("Get server power state failed");
                return rv;
        }

        switch (state) {
                case (SAHPI_POWER_ON):
                        *action = SAHPI_RESET_DEASSERT;
                        break;

                case (SAHPI_POWER_OFF):
                        *action = SAHPI_RESET_ASSERT;
                        break;

                /* Power cycle is a momentary state
                 * Hence, resource should never give the 'power cycle' as
                 * its current power state
                 */
                case (SAHPI_POWER_CYCLE):
                        err("Wrong reset state (Power cycle) detected");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                        break;
                default:
                        err("Wrong reset state");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * oa_soap_set_reset_state
 *      @oh_handler:  Pointer to openhpi handler
 *      @resource_id: Resource id
 *      @action:      Reset action
 *
 * Purpose:
 *      sets the reset state of the resource in the chassis
 *
 * Detailed Description:
 *      - Resource capability will be checked based on the resource id
 *      - and then based on the action and type of the entity, different
 *        interface api is used for resetting the resource component.
 *
 * Return values:
 *      SA_OK                      - on success.
 *      SA_ERR_HPI_INVALID_PARAMS  - on wrong parameters
 *      SA_ERR_HPI_INVALID_REQUEST - on request to reset a resource
 *                                   which is powered off.
 *                                   Or on wrong reset request
 *      SA_ERR_HPI_INTERNAL_ERROR  - on failure.
 **/
SaErrorT oa_soap_set_reset_state(void *oh_handler,
                                 SaHpiResourceIdT resource_id,
                                 SaHpiResetActionT action)
{
        SaErrorT rv = SA_OK;
        SaHpiPowerStateT tmp;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiInt32T bay_number;
        struct setBladePower server_request;
        struct resetInterconnectTray interconnect_request;
        struct oh_handler_state *handler = NULL;
        struct oa_soap_handler *oa_handler = NULL;

        if (oh_handler == NULL) {
                err("invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        /* Check whether resource has reset capability */
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_RESET)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        switch (action) {
                case SAHPI_RESET_DEASSERT:
                        /* RESET_DEASSERT is equivalent to power on
                         * Call the set power state function with power on
                         */
                        rv = oa_soap_set_power_state(oh_handler, resource_id,
                                                     SAHPI_POWER_ON);
                        if (rv != SA_OK)
                                err("Set power ON failed");
                        return rv;
                        break;

                case SAHPI_RESET_ASSERT:
                        /* RESET_ASSERT is equivalent to power off
                         * Call the set power state function with power off
                         */
                        rv = oa_soap_set_power_state(oh_handler, resource_id,
                                                     SAHPI_POWER_OFF);
                        if (rv != SA_OK)
                                err("Set power OFF failed");
                        return rv;
                        break;

                case SAHPI_COLD_RESET:
                case SAHPI_WARM_RESET:
                        /* Get the current power state */
                        rv = oa_soap_get_power_state(oh_handler, resource_id,
                                                     &tmp);
                        if (rv != SA_OK) {
                                err("Get power state failed");
                                return rv;
                        }

                        /* Reset can not be done when resource is in power
                         * off state
                         */
                        if (tmp == SAHPI_POWER_OFF) {
                                return SA_ERR_HPI_INVALID_REQUEST;
                        }

                        /* Check whether oa_handler mutex is locked or not */
                        rv = lock_oa_soap_handler(oa_handler);
                        if (rv != SA_OK) {
                                err("OA SOAP handler is locked");
                                return rv;
                        }

                        bay_number =
                                rpt->ResourceEntity.Entry[0].EntityLocation;
                        /* Check the resource entity type */
                        switch (rpt->ResourceEntity.Entry[0].EntityType) {
                                case SAHPI_ENT_SYSTEM_BLADE:
                                        /* Resource type is server blade.
                                         * Reset the server blade
                                         */
                                        server_request.bayNumber = bay_number;
                                        server_request.power = RESET;
                                        rv = soap_setBladePower(
                                                oa_handler->active_con,
                                                &server_request);
                                        if (rv != SOAP_OK) {
                                                err("Set blade power to power "
                                                    "reset failed");
                                                return SA_ERR_HPI_INTERNAL_ERROR;
                                        }
                                        return SA_OK;
                                        break;

                                case SAHPI_ENT_IO_BLADE:
                                case SAHPI_ENT_DISK_BLADE:
                                        return(SA_ERR_HPI_UNSUPPORTED_API);

                                case SAHPI_ENT_SWITCH_BLADE:
                                        /* Resource type is interconnect blade.
                                         * Reset the interconnect blade
                                         */
                                        interconnect_request.bayNumber =
                                                bay_number;
                                        rv = soap_resetInterconnectTray(
                                                oa_handler->active_con,
                                                &interconnect_request);
                                        if (rv != SOAP_OK) {
                                                err("Reset interconnect reset "
                                                    "failed");
                                                return SA_ERR_HPI_INTERNAL_ERROR;
                                        }
                                        return SA_OK;
                                        break;
                                default:
                                        err("Invalid Resource Type");
                                        return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;
                default:
                        err("Invalid reset state requested");
                        return SA_ERR_HPI_INVALID_REQUEST;
        }
        return SA_OK;
}

void * oh_get_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT *)
               __attribute__ ((weak, alias("oa_soap_get_reset_state")));

void * oh_set_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT)
               __attribute__ ((weak, alias("oa_soap_set_reset_state")));

