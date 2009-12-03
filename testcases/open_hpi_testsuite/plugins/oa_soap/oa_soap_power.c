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
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 *
 * This file handles all the power related queries
 *
 *      oa_soap_get_power_state()       - gets the power state
 *
 *      oa_soap_set_power_state()       - sets the power state of the resource
 *
 *      get_server_power_state()        - gets the server blade resource power
 *                                        state
 *
 *      get_interconnect_power_state()  - gets the inter connect resource power
 *                                        state
 *
 *      set_server_power_state()        - sets the server blade resource power
 *                                        state
 *
 **/

#include "oa_soap_power.h"

/**
 * oa_soap_get_power_state
 *      @oh_handler:  Pointer to openhpi handler
 *      @resource_id: Resource id
 *      @state:       Pointer to power state
 *
 * Purpose:
 *      Gets the power state of the resource.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                       - on success.
 *      SA_ERR_HPI_INVALID_PARAMS   - on wrong parameters
 *      SA_ERR_HPI_CAPABILITY       - on power capability is not set for
 *                                    the given resource
 *      SA_ERR_HPI_INVALID_RESOURCE - on not able to find the resource
 *      SA_ERR_HPI_INTERNAL_ERROR   - on failure.
 **/
SaErrorT oa_soap_get_power_state(void *oh_handler,
                                 SaHpiResourceIdT resource_id,
                                 SaHpiPowerStateT *state)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T bay_number;
        struct oh_handler_state *handler;

        if (oh_handler == NULL || state == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;

        /* Check whether the oa_handler mutex has been locked or not */
        rv = lock_oa_soap_handler(oa_handler);
        if (rv != SA_OK) {
                err("OA SOAP handler is locked");
                return rv;
        }

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        /* Check the resource has power capability */
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_POWER)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        bay_number = rpt->ResourceEntity.Entry[0].EntityLocation;

        /* Check resource type and query server or interconnect power state*/
        switch (rpt->ResourceEntity.Entry[0].EntityType) {
                case (SAHPI_ENT_SYSTEM_BLADE):
                case (SAHPI_ENT_IO_BLADE):
                case (SAHPI_ENT_DISK_BLADE):
                        rv = get_server_power_state(oa_handler->active_con,
                                                    bay_number, state);
                        break;

                case (SAHPI_ENT_SWITCH_BLADE):
                        rv = get_interconnect_power_state(
                                oa_handler->active_con, bay_number, state);
                        break;

                default:
                        err("Invalid Resource Type");
                        rv = SA_ERR_HPI_INTERNAL_ERROR;
        }
        return rv;
}

/**
 * oa_soap_set_power_state
 *      @oh_handler:  Pointer to openhpi handler structure
 *      @resource_id: Resource id
 *      @state:       Power state
 *
 * Purpose:
 *      Sets the power state of the resource
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT oa_soap_set_power_state(void *oh_handler,
                                 SaHpiResourceIdT resource_id,
                                 SaHpiPowerStateT state)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt = NULL;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T bay_number;
        struct oh_handler_state *handler = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;

        /* Check whether the oa_handler mutex has been locked or not */
        rv = lock_oa_soap_handler(oa_handler);
        if (rv != SA_OK) {
                err("OA SOAP handler is locked");
                return rv;
        }

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err(" INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        /* Check the resource has power capability */
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_POWER)) {
                err(" INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        bay_number = rpt->ResourceEntity.Entry[0].EntityLocation;
        /* Check resource type and set the server or interconnect power state*/
        switch (rpt->ResourceEntity.Entry[0].EntityType) {
                 case (SAHPI_ENT_SYSTEM_BLADE):
                        rv = set_server_power_state(oa_handler->active_con,
                                                    bay_number, state);
                        break;

                 case (SAHPI_ENT_IO_BLADE):
                 case (SAHPI_ENT_DISK_BLADE):
                        return(SA_ERR_HPI_UNSUPPORTED_API);

                 case (SAHPI_ENT_SWITCH_BLADE):
                        rv = set_interconnect_power_state(
                                oa_handler->active_con, bay_number, state);
                        break;

                 default:
                        err("Invalid Resource Type");
                        return SA_ERR_HPI_UNKNOWN;
        }

        return rv;
}

/**
 * get_server_power_state
 *      @con:        Pointer to the soap client handler
 *      @bay_number: Bay number of the server balde
 *      @state:      Pointer to power state of the server blade
 *
 * Purpose:
 *      Gets the server blade power state.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT get_server_power_state(SOAP_CON *con,
                                SaHpiInt32T bay_number,
                                SaHpiPowerStateT *state)
{
        SaErrorT rv = SA_OK;
        struct getBladeStatus request;
        struct bladeStatus response;

        if (con == NULL || state == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        request.bayNumber = bay_number;
        rv = soap_getBladeStatus(con, &request, &response);
        if (rv != SOAP_OK) {
                err("Get blade status failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        switch (response.powered) {
                case (POWER_ON):
                        *state = SAHPI_POWER_ON;
                        break;
                case (POWER_OFF):
                        *state = SAHPI_POWER_OFF;
                        break;
                case (POWER_REBOOT):
                        err("Wrong Power State (REBOOT) detected");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                        break;
                default:
                        err("Unknown Power State detected");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * get_interconnect_power_state
 *      @con:        Pointer to the soap client handler
 *      @bay_number: Bay number of the interconnect balde
 *      @state:      Pointer to power state of the interconnect blade
 *
 * Purpose:
 *      Gets the interconnect power state.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT get_interconnect_power_state(SOAP_CON *con,
                                      SaHpiInt32T bay_number,
                                      SaHpiPowerStateT *state)
{
        SaErrorT rv = SA_OK;
        struct getInterconnectTrayStatus request;
        struct interconnectTrayStatus response;

        if (con == NULL || state == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        request.bayNumber = bay_number;
        rv = soap_getInterconnectTrayStatus(con, &request, &response);
        if (rv != SOAP_OK) {
                err("Get interconnect tray status failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        switch (response.powered) {
                case (POWER_ON):
                        *state = SAHPI_POWER_ON;
                        break;
                case (POWER_OFF):
                        *state = SAHPI_POWER_OFF;
                        break;
                case (POWER_REBOOT):
                        err("Wrong (REBOOT) Power State detected");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                        break;
                default:
                        err("Unknown Power State detected");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}



/**
 * set_server_power_state
 *      @con:        Pointer to the soap client handler
 *      @bay_namber: Bay number of the server blade
 *      @state:      Power state of the server blade
 *
 * Purpose:
 *      Sets the power state of the server blade,
 *      if the current state is not same as requested state.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                      - on success.
 *      SA_ERR_HPI_INVALID_PARAMS  - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR  - on failure.
 **/
SaErrorT set_server_power_state(SOAP_CON *con,
                                SaHpiInt32T bay_number,
                                SaHpiPowerStateT state)
{
        SaErrorT rv = SA_OK;
        SaHpiPowerStateT tmp;
        struct setBladePower blade_power;
        SaHpiInt32T pwroff_poll = 0;

        if (con == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rv = get_server_power_state(con, bay_number, &tmp);
        if (rv != SA_OK) {
                err("get server power state failed");
                return rv;
        }

        /* Check whether the current server blade power state is same
         * as requested by user. If yes, ignore the user request
         */
        if (state == tmp) {
                err("Nothing to be done. Blade is in the requested state");
                return SA_OK;
        }

        blade_power.bayNumber = bay_number;
        switch (state) {
                case (SAHPI_POWER_ON):
                        blade_power.power = MOMENTARY_PRESS;
                        rv = soap_setBladePower(con, &blade_power);
                        if (rv != SOAP_OK) {
                                err("Set blade power to power on failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;

                case (SAHPI_POWER_OFF):
                        blade_power.power = PRESS_AND_HOLD;
                        rv = soap_setBladePower(con, &blade_power);
                        if (rv != SOAP_OK) {
                                err("Set blade power to power off failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;

                case (SAHPI_POWER_CYCLE):
                        /* Power cycle requires the server to be
                         * power off and then power on
                         * Check whether the current server power state is off
                         * If yes, power on the server
                         * Else, power off and then power on
                         */
                        if (tmp != SAHPI_POWER_OFF) {
                                blade_power.power = PRESS_AND_HOLD;

                                rv = soap_setBladePower(con, &blade_power);
                                if (rv != SOAP_OK) {
                                        err("Set blade power to power off "
                                            "failed");
                                        return SA_ERR_HPI_INTERNAL_ERROR;
                                }

				/* Check whether the power state is OFF for 
				 * every OA_POWEROFF_POLL_INTERVAL seconds.
			 	 * Server blades take a few seconds (around 4
				 * secs) to power off if no operating system is
				 * running there. Otherwise, an orderly shutdown
				 * is performed, which could take a while.
				 */

				while (pwroff_poll < OA_MAX_POWEROFF_POLLS) {
					rv = get_server_power_state(con,
							      bay_number, &tmp);
					if (rv != SA_OK) {
						err("get_server_power_state failed");
						return(SA_ERR_HPI_INTERNAL_ERROR);
					}

					if ( tmp == SAHPI_POWER_OFF)
						break;

					sleep(OA_POWEROFF_POLL_INTERVAL);
					pwroff_poll++;
				}

				if( pwroff_poll >= OA_MAX_POWEROFF_POLLS){

					err("Max poweroff polls exceeded (%d)",
							 OA_MAX_POWEROFF_POLLS);
					return( SA_ERR_HPI_INTERNAL_ERROR);
				}

				/* There is a race condition if a "power on"
				 * command is sent immediately after a 
				 * "power off" command, and we may hit this
				 * even if we have polled for the current 
				 * power state, unless we wait a bit first.
				 */
				sleep(OA_SERVER_POWER_OFF_WAIT_PERIOD);

			} /* end if tmp != SAHPI_POWER_OFF */

			/* Now, turn the blade back on */

                        blade_power.power = MOMENTARY_PRESS;
                        rv = soap_setBladePower(con, &blade_power);
                        if (rv != SOAP_OK) {
                                err("Set blade power to power on failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;

                default:
                        err("Invalid power state");
                        return SA_ERR_HPI_INVALID_PARAMS;
        }
        return SA_OK;
}

/**
 * set_interconnect_power_state
 *      @con:        Pointer to the soap client handler
 *      @bay_namber: Bay number of the interconnect blade
 *      @state:      Power state of the interconnect blade
 *
 * Purpose:
 *      Sets the power state of the interconnect blade,
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                      - on success.
 *      SA_ERR_HPI_INVALID_PARAMS  - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR  - on failure.
 **/
SaErrorT set_interconnect_power_state(SOAP_CON *con,
                                      SaHpiInt32T bay_number,
                                      SaHpiPowerStateT state)
{
        SaErrorT rv = SA_OK;
        SaHpiPowerStateT tmp;
        struct setInterconnectTrayPower interconnect_power;

        if (con == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        interconnect_power.bayNumber = bay_number;
        switch (state) {
                case (SAHPI_POWER_ON):
                        interconnect_power.on = HPOA_TRUE;
                        rv = soap_setInterconnectTrayPower(con,
                                                           &interconnect_power);
                        if (rv != SOAP_OK) {
                                err("Set interconnect power to power on "
                                    "failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;

                case (SAHPI_POWER_OFF):
                        interconnect_power.on = HPOA_FALSE;
                        rv = soap_setInterconnectTrayPower(con,
                                                           &interconnect_power);
                        if (rv != SOAP_OK) {
                                err("Set interconnect power to power off "
                                    "failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;

                case (SAHPI_POWER_CYCLE):
                        /* Get the current power state of the interconnect */
                        rv = get_interconnect_power_state(con, bay_number,
                                                          &tmp);
                        if (rv != SA_OK ) {
                                err("get interconnect power state failed");
                                return rv;
                        }

                        /* Power cycle requires the server to be
                         * power off and then power on
                         * Check whether the current server power state is off
                         * If yes, power on the server
                         * Else, power off and then power on
                         */
                        if (tmp != SAHPI_POWER_OFF) {
                                interconnect_power.on = HPOA_FALSE;
                                rv = soap_setInterconnectTrayPower(
                                        con, &interconnect_power);
                                if (rv != SOAP_OK) {
                                        err("Set interconnect power to "
                                            "power on failed");
                                        return SA_ERR_HPI_INTERNAL_ERROR;
                                }
                        }
                        interconnect_power.on = HPOA_TRUE;

                        rv = soap_setInterconnectTrayPower(con,
                                                           &interconnect_power);
                        if (rv != SOAP_OK) {
                                err("Set interconnect power to power on "
                                    "failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;
                default:
                        err("Invalid power state");
                        return SA_ERR_HPI_INVALID_PARAMS;
        }
        return SA_OK;
}

void * oh_get_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT *)
                __attribute__ ((weak, alias("oa_soap_get_power_state")));

void * oh_set_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT)
                __attribute__ ((weak, alias("oa_soap_set_power_state")));

