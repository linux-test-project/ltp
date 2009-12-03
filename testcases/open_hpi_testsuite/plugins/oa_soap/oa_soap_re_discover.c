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
 *      Vivek Kumar <vivek.kumar2@hp.com>
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 *      Mohan Devarajulu <mohan@fc.hp.com>
 *
 * This file implements the re-discovery functionality. The resources of the
 * HP BladeSystem c-Class are re-discovered, whenever the connection to the
 * active OA is broken or on OA switchover.  Re-discovery is done to sync the
 * plugin with the current state of the resources.
 *
 *      oa_soap_re_discover_resources() - Starts the re-discovery of server
 *                                        blades, interconnect blades, OA,
 *                                        fan and power supplies.
 *
 *      re_discover_server()            - Re-discovers the ProLiant server
 *                                        blades
 *
 *      re_discover_interconnect()      - Re-discovers the interconnect blades
 *
 *      re_discover_fan()               - Re-discovers the fan
 *
 *      re_discover_power_supply()      - Re-discovers the power supply units
 *
 *      re_discover_oa()                - Re-discovers the onboard administrator
 *
 *	oa_soap_re_disc_oa_sen()	- Re-discovers the OA sensor states
 *
 * 	oa_soap_re_disc_server_sen()	- Re-discovers the server sensor states
 *
 *	oa_soap_re_disc_interconct_sen()- Re-discovers the interconnect sensor
 *					  states
 *
 *	oa_soap_re_disc_ps_sen()	- Re-discovers the power supply sensor
 *					  states
 *
 *	oa_soap_re_disc_enc_sen()	- Re-discovers the enclosure sensor
 *					  states
 *
 *	oa_soap_re_disc_ps_subsys_sen()	- Re-discovers the power subsystem
 *					  sensor states
 *
 *	oa_soap_re_disc_lcd_sen()	- Re-discovers the LCD sensor states
 *
 *	oa_soap_re_disc_fz_sen()	- Re-discovers the fan zone sensor
 *					  states
 *
 * 	oa_soap_re_disc_therm_subsys_sen()- Re-discovers the thermal subsystem
 *					    sensor states
 */

#include "oa_soap_re_discover.h"

/* Forward declarations for static functions */
static SaErrorT oa_soap_re_disc_oa_sen(struct oh_handler_state *oh_handler,
				       SOAP_CON *con,
				       SaHpiInt32T bay_number);
static SaErrorT oa_soap_re_disc_server_sen(struct oh_handler_state *oh_handler,
					   SOAP_CON *con,
					   SaHpiInt32T bay_number);
static SaErrorT oa_soap_re_disc_interconct_sen(struct oh_handler_state
							*oh_handler,
					      SOAP_CON *con,
					      SaHpiInt32T bay_number);
static SaErrorT oa_soap_re_disc_ps_sen(struct oh_handler_state *oh_handler,
				       SOAP_CON *con,
				       SaHpiInt32T bay_number);
static SaErrorT oa_soap_re_disc_enc_sen(struct oh_handler_state *oh_handler,
					SOAP_CON *con);
static SaErrorT oa_soap_re_disc_ps_subsys_sen(struct oh_handler_state
						*oh_handler,
					      SOAP_CON *con);
static SaErrorT oa_soap_re_disc_lcd_sen(struct oh_handler_state *oh_handler,
					SOAP_CON *con);
static SaErrorT oa_soap_re_disc_fz_sen(struct oh_handler_state *oh_handler,
				       SOAP_CON *con);
static SaErrorT oa_soap_re_disc_therm_subsys_sen(struct oh_handler_state
							*oh_handler,
						SOAP_CON *con);

/**
 * oa_soap_re_discover_resources
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to the SOAP_CON structure
 *
 * Purpose:
 *      Re-discover the resources.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT oa_soap_re_discover_resources(struct oh_handler_state *oh_handler,
                                       struct oa_info *oa, int oa_switched)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        if (oh_handler == NULL || oa == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        err("Re-discovery started");

	/* The following is applicable only to OA Switchover cases
	Just rediscover oa and end the whole thing. If some other hardware
	is removed at the same time, then it is a separate case that needs to
	be handled separately */

        if ( oa_switched == SAHPI_TRUE ) {
                OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
                rv = re_discover_oa(oh_handler, oa->event_con);
                if (rv != SA_OK) {
                        err("Re-discovery of OA failed");
                }
                return rv;
        }

	/* Re-discovery is called by locking the OA handler mutex and oa_info
	 * mutex. Hence on getting request to shutdown, pass the locked mutexes
	 * for unlocking
	 */
	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
        rv = re_discover_blade(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of server blade failed");
                return rv;
        }

	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
        rv = re_discover_interconnect(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of interconnect failed");
                return rv;
        }

	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
        rv = re_discover_fan(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of fan failed");
                return rv;
        }

	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
        rv = re_discover_ps_unit(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of power supply unit failed");
                return rv;
        }

	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
        rv = re_discover_oa(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of OA failed");
                return rv;
        }

	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
	rv = oa_soap_re_disc_enc_sen(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of enclosure failed");
                return rv;
        }

	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
	rv = oa_soap_re_disc_ps_subsys_sen(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of power subsystem failed");
                return rv;
        }

	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
	rv = oa_soap_re_disc_lcd_sen(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of LCD failed");
                return rv;
        }

	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
	rv = oa_soap_re_disc_fz_sen(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of fan zone failed");
                return rv;
        }

	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex, oa->mutex,
				  NULL);
	rv = oa_soap_re_disc_therm_subsys_sen(oh_handler, oa->event_con);
        if (rv != SA_OK) {
                err("Re-discovery of thermal subsystem failed");
                return rv;
        }


        err("Re-discovery completed");
        return SA_OK;
}

/**
 * re_discover_oa
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to the SOAP_CON structure
 *
 * Purpose:
 *      Re-discover the OAs.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT re_discover_oa(struct oh_handler_state *oh_handler,
                        SOAP_CON *con)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        struct getOaStatus request;
        struct oaStatus response;
        struct getOaInfo info_request;
        struct oaInfo info_response;
        SaHpiInt32T i;
        enum resource_presence_status state = RES_ABSENT;
        SaHpiBoolT replace_resource = SAHPI_FALSE;

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        for (i = 1; i <= oa_handler->oa_soap_resources.oa.max_bays; i++) {
                request.bayNumber = i;
                rv = soap_getOaStatus(con, &request, &response);
                if (rv != SOAP_OK) {
                        err("get OA status failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* Sometimes, if the OA is absent, then OA status is shown as
                 * STANDBY in getOaStatus response.  As workaround, if OA
                 * status is STANDBY and oaRedudancy state is set to false,
                 * Then, it is considered as ABSENT.
                 *
                 * But, if the OA is recently inserted, then oaRedudancy state
                 * will be set to false.  In this scenario, the OA state will
                 * be wrongly considered as ABSENT.  This is a known limitation.
                 *
                 * TODO: Remove this workaround once the fix is available in
                 * OA firmware
                 */
                if ((response.oaRole == OA_ABSENT) ||
                    (response.oaRole == STANDBY &&
                     response.oaRedundancy == HPOA_FALSE)) {
                        /* The OA is absent, check OA is absent in presence
                         * matrix
                         */
                        if (oa_handler->oa_soap_resources.oa.presence[i - 1] ==
                            RES_ABSENT)
                                continue;
                        else
                                state = RES_ABSENT;
                } else {
                        /* The OA is present, check OA is present in presence
                         * matrix
                         */
                        if (oa_handler->oa_soap_resources.oa.presence[i - 1] ==
                            RES_PRESENT) {
                                /* Check whether OA has been replaced */
                                info_request.bayNumber = i;
                                rv = soap_getOaInfo(con, &info_request,
                                                    &info_response);
                                if (rv != SOAP_OK) {
                                        err("get OA status failed");
                                        return SA_ERR_HPI_INTERNAL_ERROR;
                                }
                                /* If serail number is different
                                 * remove and add OA
                                 */
                                if (strcmp(oa_handler->oa_soap_resources.oa.
                                           serial_number[i - 1],
                                           info_response.serialNumber) != 0) {
                                        replace_resource = SAHPI_TRUE;
                                } else {
					/* Check the OA sensors state */
					rv = oa_soap_re_disc_oa_sen(
						oh_handler, con, i);
					if (rv != SA_OK) {
						err("Re-discover OA sensors "
						    " failed");
						return rv;
					}
                                        continue;
				}
                       } else
                                state = RES_PRESENT;
                }

                if (state == RES_ABSENT || replace_resource == SAHPI_TRUE) {
                        /* The OA is present according OA presence matrix, but
                         * OA is removed.  Remove the OA resource from RPTable.
                         */
                        rv = remove_oa(oh_handler, i);
                        if (rv != SA_OK) {
                                err("OA %d removal failed", i);
                                return rv;
                        } else
                                err("OA in slot %d is removed", i);
                }

                if (state == RES_PRESENT || replace_resource == SAHPI_TRUE) {
                        /* The OA is absent according OA presence matrix, but
                         * OA is present.  Add the OA resource to RPTable.
                         */
                        rv = add_oa(oh_handler, con, i);
                        if (rv != SA_OK) {
                                err("OA %d add failed", i);
                                return rv;
                        } else
                                err("OA in slot %d is added", i);

                        replace_resource = SAHPI_FALSE;
                }
        } /* End of for loop */
        return SA_OK;
}

/**
 * remove_oa
 *      @oh_handler: Pointer to openhpi handler
 *      @bay_number: Bay number of the extracted OA
 *
 * Purpose:
 *      Removes the OA information from the RPTable
 *      Updates the status of the OA in OA data structure as absent
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT remove_oa(struct oh_handler_state *oh_handler,
                   SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt = NULL;
        struct oh_event event;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Update the OA status to absent */
        switch (bay_number) {
                case 1:
                        g_mutex_lock(oa_handler->oa_1->mutex);
                        oa_handler->oa_1->oa_status = OA_ABSENT;
                        g_mutex_unlock(oa_handler->oa_1->mutex);
                        break;
                case 2:
                        g_mutex_lock(oa_handler->oa_2->mutex);
                        oa_handler->oa_2->oa_status = OA_ABSENT;
                        g_mutex_unlock(oa_handler->oa_2->mutex);
                        break;
                default:
                        err("Wrong OA bay number %d passed", bay_number);
                        return SA_ERR_HPI_INVALID_PARAMS;
        }

        update_hotswap_event(oh_handler, &event);
        resource_id =
           oa_handler->oa_soap_resources.oa.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;
	event.event.Severity = event.resource.ResourceSeverity;

        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;
        /* This state change happened due to surprise extraction */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_SURPRISE_EXTRACTION;

        /* Push the hotswap event to remove the resource from OpenHPI RPTable */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        /* Free the inventory info from inventory RDR */
        rv = free_inventory_info(oh_handler, rpt->ResourceId);
        if (rv != SA_OK) {
                err("Inventory cleanup failed for resource id %d",
                    rpt->ResourceId);
        }
        /* Remove the resource from plugin RPTable */
        rv = oh_remove_resource(oh_handler->rptcache,
                                event.resource.ResourceId);

        /* Reset resource_status structure to default values */
        oa_soap_update_resource_status(&oa_handler->oa_soap_resources.oa,
                                       bay_number, "",
                                       SAHPI_UNSPECIFIED_RESOURCE_ID,
                                       RES_ABSENT);
        return SA_OK;
}

/**
 * add_oa
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to the SOAP_CON structure
 *      @bay_number: Bay number of the extracted OA
 *
 * Purpose:
 *      Re-discover the newly added OA.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT add_oa(struct oh_handler_state *oh_handler,
                SOAP_CON *con,
                SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        struct getOaStatus status_request;
        struct oaStatus status_response;
        struct getOaInfo request;
        struct oaInfo response;
        struct getOaNetworkInfo network_info;
        struct oaNetworkInfo network_info_response;
        struct oa_soap_handler *oa_handler = NULL;
        struct oa_info *temp = NULL;
        SaHpiResourceIdT resource_id;
        struct oh_event event;
	GSList *asserted_sensors = NULL;
	SaHpiRptEntryT *rpt;

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Get the oa_info structure of the inserted OA */
        switch (bay_number) {
                case 1:
                        temp = oa_handler->oa_1;
                        break;
                case 2:
                        temp = oa_handler->oa_2;
                        break;
        }

        /* If the OA is removed during the first discovery, then
         * oa_soap_handler will have the information of the removed OA.
         * But, RPTable will not have the extracted OA information.
         * If the OA is inserted back and becomes active for any reason,
         * then we may end up trying to update the same SOAP_CON structure.
         *
         * To avoid this situation, check whether the event's SOAP_CON for
         * the inserted OA is same as the SOAP_CON passed to this function.
         * If yes, skip the SOAP_CON updating.
         */

        if (temp->event_con != con) {
                status_request.bayNumber = bay_number;
                rv = soap_getOaStatus(con, &status_request, &status_response);
                if (rv != SOAP_OK) {
                        err("get OA status failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* Update the OA status of the inserted OA */
                g_mutex_lock(temp->mutex);
                temp->oa_status = status_response.oaRole;
                g_mutex_unlock(temp->mutex);

                /* Get the IP address of the newly inserted OA */
                network_info.bayNumber = bay_number;
                rv = soap_getOaNetworkInfo(con, &network_info,
                                           &network_info_response);
                if (rv != SOAP_OK) {
                        err("Get OA network info failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* Copy the server IP address to oa_info structure*/
                g_mutex_lock(temp->mutex);
                memset(temp->server, 0, MAX_URL_LEN);
                strncpy(temp->server, network_info_response.ipAddress,
                        strlen(network_info_response.ipAddress));
                g_mutex_unlock(temp->mutex);
        }

        request.bayNumber = bay_number;
        rv = soap_getOaInfo(con, &request, &response);
        if (rv != SOAP_OK) {
                err("Get OA info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* If the OA is not yet stable, then getOaInfo response
         * structure will not have proper information. Abort the
         * re-discovery and let the OA to stabilize. The re-discovery will be
         * called again after some time which will allow OA to stabilize
         */
        if (response.serialNumber == NULL) {
                err("OA %d is not yet stabilized", bay_number);
                err("Re-discovery is aborted");
                err("Re-discovery will happen after sometime");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Build the rpt entry */
        rv = build_oa_rpt(oh_handler, bay_number, &resource_id);
        if (rv != SA_OK) {
                err("Failed to build OA RPT");
                return rv;
        }

        /* Update resource_status structure with resource_id, serial_number,
         * and presence status
         */
        oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.oa, bay_number,
                      response.serialNumber, resource_id, RES_PRESENT);

        /* Update the OA firmware version to RPT entry */
        rv = update_oa_info(oh_handler, &response, resource_id);
        if (rv != SA_OK) {
                err("Failed to update OA RPT");
                return rv;
        }

        /* Build the RDRs */
        rv = build_oa_rdr(oh_handler, con, bay_number, &response, resource_id);
        if (rv != SA_OK) {
                err("Failed to build OA RDR");
                /* Free the inventory info from inventory RDR */
                rv = free_inventory_info(oh_handler, resource_id);
                if (rv != SA_OK) {
                        err("Inventory cleanup failed for resource id %d",
                             resource_id);
                }
                oh_remove_resource(oh_handler->rptcache, resource_id);
                /* reset resource_status structure to default values */
                oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.oa, bay_number,
                              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                return rv;
        }

        rv = oa_soap_populate_event(oh_handler, resource_id, &event,
				    &asserted_sensors);
        if (rv != SA_OK) {
                err("Populating event struct failed");
                return rv;
        }

        event.event.EventType = SAHPI_ET_HOTSWAP;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        /* NOT_PRESENT to ACTIVE state change happened due to
         * operator action
         */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_OPERATOR_INIT;

        /* Push the hotswap event to add the resource to OpenHPI RPTable */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        oa_handler->oa_soap_resources.oa.presence[bay_number - 1] = RES_PRESENT;

	/* Raise the assert sensor events */
	if (asserted_sensors) {
	        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
		oa_soap_assert_sen_evt(oh_handler, rpt, asserted_sensors);
	}

        return SA_OK;
} /* add_oa */

/**
 * re_discover_blade
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to the SOAP_CON structure
 *
 * Purpose:
 *      Re-discover the Server Blades.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT re_discover_blade(struct oh_handler_state *oh_handler,
                           SOAP_CON *con)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        struct getBladeInfo request;
        struct bladeInfo response;
        SaHpiInt32T i;
        enum resource_presence_status state = RES_ABSENT;
        SaHpiBoolT replace_resource = SAHPI_FALSE;

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        for (i = 1; i <= oa_handler->oa_soap_resources.server.max_bays; i++) {
                request.bayNumber = i;
                rv = soap_getBladeInfo(con, &request, &response);
                if (rv != SOAP_OK) {
                        err("Get blade info failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                if (response.presence != PRESENT ) {
                        /* The blade is absent.  Is the blade absent in
                         * the presence matrix?
                         */
                        if (oa_handler->oa_soap_resources.server.presence[i - 1]
                            == RES_ABSENT)
                                continue;
                        else
                                state = RES_ABSENT;
                }
                /* The server blade is present.  Is the server present in
                 * the presence matrix?
                 */
                else if (oa_handler->oa_soap_resources.server.presence[i - 1]
                           == RES_PRESENT) {
                        /* If Serial number is different, remove and
                         * add the blade
                         */
                        if (strcmp(oa_handler->oa_soap_resources.server.
                                   serial_number[i - 1],
                                   response.serialNumber) != 0) {
                                replace_resource = SAHPI_TRUE;
                        } else {
                                /* Check and update the hotswap state
                                 * of the server blade
                                 */
                                if(response.bladeType == BLADE_TYPE_SERVER) {
                                    rv = update_server_hotswap_state(
                                             oh_handler, con, i);
                                    if (rv != SA_OK) {
                                        err("Update server hot swap"
                                            " state failed");
                                        return rv;
                                    }
                                }
				/* Check the server sensors state */
				rv = oa_soap_re_disc_server_sen(oh_handler, con,
								i);
				if (rv != SA_OK) {
					err("Re-discover server sensors "
					    "failed");
					return rv;
				}
                                continue;
                        }
                } else
                        state = RES_PRESENT;

                if (state == RES_ABSENT || replace_resource == SAHPI_TRUE) {
                        /* The server blade is present according OA presence
                         * matrix, but server is removed.  Remove the server
                         * resource from RPTable.
                         */
                        rv = remove_server_blade(oh_handler, i);
                        if (rv != SA_OK) {
                                err("Server blade %d removal failed", i);
                                return rv;
                        } else
                                err("Server in slot %d is removed", i);
                }

                if (state == RES_PRESENT || replace_resource == SAHPI_TRUE) {
                        /* The server blade is absent according OA presence
                         * matrix, but server is present.  Add the server
                         * resource to RPTable.
                         */
                        rv = add_server_blade(oh_handler, con, &response);
                        if (rv != SA_OK) {
                                err("Server blade %d add failed", i);
                                return rv;
                        } else
                                err("Server in slot %d is added", i);

                        replace_resource = SAHPI_FALSE;
                }
        } /* End of for loop */
        return SA_OK;
}

/**
 * update_server_hotswap_state
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to soap client handler
 *      @bay_number: Bay number of the removed blade
 *
 * Purpose:
 *      Updates the server blade hot swap state in RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT update_server_hotswap_state(struct oh_handler_state *oh_handler,
                                     SOAP_CON *con,
                                     SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt = NULL;
        struct oa_soap_hotswap_state *hotswap_state = NULL;
        struct oh_event event;
        SaHpiPowerStateT state;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler;

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.server.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(oh_handler->rptcache, rpt->ResourceId);
        if (hotswap_state == NULL) {
                err("Unable to get the resource private data");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        rv = get_server_power_state(con, bay_number, &state);
        if (rv != SA_OK) {
                err("Unable to get power state");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check whether current hotswap state of the server is same as
         * in hotswap structure in rpt entry
         */
        if ((state == SAHPI_POWER_ON &&
             hotswap_state->currentHsState == SAHPI_HS_STATE_ACTIVE) ||
            (state == SAHPI_POWER_OFF &&
             hotswap_state->currentHsState == SAHPI_HS_STATE_INACTIVE)) {
                return SA_OK;
        }

        /* Hotswap structure in rpt entry is not reflecting the current state
         * Update the hotswap structure and raise hotswap event
         */
        update_hotswap_event(oh_handler, &event);
        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;
        switch (state) {
                case SAHPI_POWER_ON:
                        /* Server got powered on.  Update the hotswap
                         * structure.
                         */
                        hotswap_state->currentHsState = SAHPI_HS_STATE_ACTIVE;
                        event.rdrs = NULL;
                        /* Raise the hotswap events */
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState = SAHPI_HS_STATE_INACTIVE;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_INSERTION_PENDING;
                        /* The cause of the state change is unknown */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_UNKNOWN;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));

                        event.rdrs = NULL;
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState =
                                SAHPI_HS_STATE_INSERTION_PENDING;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                                     SAHPI_HS_STATE_ACTIVE;
                        /* INSERTION_PENDING to ACTIVE state change happened
                         * due to auto policy of server blade
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));
                        break;

                case SAHPI_POWER_OFF:
                        /* Server got powered off.  Update the hotswap
                         * structure.
                         */
                        hotswap_state->currentHsState = SAHPI_HS_STATE_INACTIVE;
                        event.rdrs = NULL;
                        /* Raise the hotswap events */
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_EXTRACTION_PENDING;
                        /* ACTIVE to EXTRACTION_PENDING state change happened
                         * due power off event. The deactivation can not be
                         * stopped.
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange =
                                SAHPI_HS_CAUSE_UNEXPECTED_DEACTIVATION;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));

                        event.rdrs = NULL;
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState =
                                SAHPI_HS_STATE_EXTRACTION_PENDING;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                                     SAHPI_HS_STATE_INACTIVE;
                        /* EXTRACTION_PENDING to INACTIVE state change happened
                         * due to auto policy of server blade
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));
                        break;

                default:
                        err("unknown power status");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * remove_server_blade
 *      @oh_handler: Pointer to openhpi handler
 *      @bay_number: Bay number of the removed blade
 * Purpose:
 *      Remove the Server Blade from the RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT remove_server_blade(struct oh_handler_state *oh_handler,
                             SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        struct oa_soap_hotswap_state *hotswap_state;
        SaHpiRptEntryT *rpt = NULL;
        struct oh_event event;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        update_hotswap_event(oh_handler, &event);

        resource_id =
           oa_handler->oa_soap_resources.server.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;
	event.event.Severity = event.resource.ResourceSeverity;

        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
                /* Simple hotswap */
                event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                        SAHPI_HS_STATE_ACTIVE;
        } else {
                /* Managed hotswap */
                hotswap_state = (struct oa_soap_hotswap_state *)
                        oh_get_resource_data(oh_handler->rptcache,
                                             event.resource.ResourceId);
                if (hotswap_state == NULL) {
                        err("Failed to get hotswap state of server blade");
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState = SAHPI_HS_STATE_INACTIVE;
                } else {
                	event.event.EventDataUnion.HotSwapEvent.
				PreviousHotSwapState =
					hotswap_state->currentHsState;
		}
	}
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;

        if (event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState ==
            SAHPI_HS_STATE_INACTIVE) {
                /* INACTIVE to NOT_PRESENT state change happened due to
                 * operator action
                 */
                event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                        SAHPI_HS_CAUSE_OPERATOR_INIT;
        } else {
                /* This state change happened due to a surprise extraction */
                event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                        SAHPI_HS_CAUSE_SURPRISE_EXTRACTION;
        }

        /* Push the hotswap event to remove the resource from OpenHPI RPTable */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        /* Free the inventory info from inventory RDR */
        rv = free_inventory_info(oh_handler, rpt->ResourceId);
        if (rv != SA_OK) {
                err("Inventory cleanup failed for resource id %d",
                    rpt->ResourceId);
        }
        /* Remove the resource from plugin RPTable */
        rv = oh_remove_resource(oh_handler->rptcache,
                                event.resource.ResourceId);

        /* reset resource_status structure to default values */
        oa_soap_update_resource_status(
              &oa_handler->oa_soap_resources.server, bay_number,
              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);

        return SA_OK;
}

/**
 * add_server_blade
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to the SOAP_CON structure
 *      @info:       Pointer to the get blade info response structure
 *
 * Purpose:
 *      Remove the Server Blade from the RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT add_server_blade(struct oh_handler_state *oh_handler,
                          SOAP_CON *con,
                          struct bladeInfo *info)
{
        SaErrorT rv = SA_OK;
        struct oh_event event;
        SaHpiPowerStateT state;
        SaHpiInt32T bay_number;
        struct getBladeInfo request;
        struct bladeInfo response;
        struct oa_soap_handler *oa_handler;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt;
	GSList *asserted_sensors = NULL;
	char blade_name[MAX_NAME_LEN];

        if (oh_handler == NULL || info == NULL || con == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        update_hotswap_event(oh_handler, &event);
        bay_number = info->bayNumber;

        /* Get blade info to obtain serial_number */
        request.bayNumber = bay_number;
        rv = soap_getBladeInfo(con, &request, &response);
        if (rv != SOAP_OK) {
                err("Get blade info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

	/* Copy the blade name from response for future processing */
	convert_lower_to_upper(response.name, strlen(response.name),
			       blade_name, MAX_NAME_LEN);

        /* Build the server RPR entry */
        rv = build_discovered_server_rpt(oh_handler, con, info, &resource_id);
        if (rv != SA_OK) {
                err("build inserted server rpt failed");
                return rv;
        }

        /* Update resource_status structure with resource_id, serial_number,
         * and presence status
         */
        oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.server, bay_number,
                      response.serialNumber, resource_id, RES_PRESENT);

        /* Build the server RDR */
        rv = build_server_rdr(oh_handler, con, bay_number, resource_id, 
			      blade_name);
        if (rv != SA_OK) {
                err("build inserted server RDR failed");
                /* Free the inventory info from inventory RDR */
                rv = free_inventory_info(oh_handler, resource_id);
                if (rv != SA_OK) {
                        err("Inventory cleanup failed for resource id %d",
                             resource_id);
                }
                oh_remove_resource(oh_handler->rptcache, resource_id);
                /* reset resource_status structure to default values */
                oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.server, bay_number,
                              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                return rv;
        }

        rv = oa_soap_populate_event(oh_handler, resource_id, &event,
				    &asserted_sensors);
        if (rv != SA_OK) {
                err("Populating event struct failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rpt = oh_get_resource_by_id (oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        /* For blades that don't support  managed hotswap, send simple
           hotswap event  */
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
                event.event.EventType = SAHPI_ET_HOTSWAP;
                event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                        SAHPI_HS_STATE_NOT_PRESENT;
                event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                        SAHPI_HS_STATE_ACTIVE;
                event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                        SAHPI_HS_CAUSE_OPERATOR_INIT;
                oh_evt_queue_push(oh_handler->eventq,
                        copy_oa_soap_event(&event));

		/* Raise the assert sensor events */
		if (asserted_sensors)
			oa_soap_assert_sen_evt(oh_handler, rpt,
						     asserted_sensors);

                return(SA_OK);
        }

        /* Raise the hotswap event for the inserted server blade */
        event.event.EventType = SAHPI_ET_HOTSWAP;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_INSERTION_PENDING;
        /* NOT_PRESENT to INSERTION_PENDING state change happened due
         * to operator action
         */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_OPERATOR_INIT;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        event.rdrs = NULL;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_INSERTION_PENDING;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        /* INSERTION_PENDING to ACTIVE state change happened
         * due to auto policy of server blade
         */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_AUTO_POLICY;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        rv = get_server_power_state(con, bay_number, &state);
        if (rv != SA_OK) {
                err("Unable to get power status");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check the power state of the server.  If the power state is off,
         * may be server got powered off after inserting.  Inserting the
         * server makes to power on automatically.  Hence raise the hotswap
         * events for power off.
         */
        switch (state) {
                case SAHPI_POWER_ON:
                        break;

                case SAHPI_POWER_OFF:
                        event.rdrs = NULL;
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_EXTRACTION_PENDING;
                        /* ACTIVE to EXTRACTION_PENDING state change happened
                         * due power off event. The deactivation can not be
                         * stopped.
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange =
                                SAHPI_HS_CAUSE_UNEXPECTED_DEACTIVATION;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));

                        event.rdrs = NULL;
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState =
                                SAHPI_HS_STATE_EXTRACTION_PENDING;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_INACTIVE;
                        /* EXTRACTION_PENDING to INACTIVE state change happened
                         * due to auto policy of server blade
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));
                        break;

                default:
                        err("unknown power status");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

	/* Raise the assert sensor events */
	if (asserted_sensors)
		oa_soap_assert_sen_evt(oh_handler, rpt, asserted_sensors);

        return SA_OK;
}

/**
 * re_discover_interconnect
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to the SOAP_CON structure
 *
 * Purpose:
 *      Re-discover the interconnects.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT re_discover_interconnect(struct oh_handler_state *oh_handler,
                                  SOAP_CON *con)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        struct getInterconnectTrayStatus request;
        struct interconnectTrayStatus response;
        struct getInterconnectTrayInfo info_request;
        struct interconnectTrayInfo info_response;
        SaHpiInt32T i;
        enum resource_presence_status state = RES_ABSENT;
        SaHpiBoolT replace_resource = SAHPI_FALSE;

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        for (i = 1;
             i <= oa_handler->oa_soap_resources.interconnect.max_bays;
             i++) {
                request.bayNumber = i;
                rv = soap_getInterconnectTrayStatus(con, &request, &response);
                if (rv != SOAP_OK) {
                        err("Get interconnect tray status failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                if (response.presence != PRESENT) {
                        /* The interconnect is absent.  Is the interconnect
                         * absent in the presence matrix?
                         */
                        if (oa_handler->
                            oa_soap_resources.interconnect.presence[i - 1] ==
                            RES_ABSENT)
                                continue;
                        else
                                state = RES_ABSENT;

                }
                /* The interconnect is present.  Is the interconnect present
                 * in the presence matrix?
                 */
                else if (oa_handler->
                           oa_soap_resources.interconnect.presence[i - 1] ==
                           RES_PRESENT) {
                        info_request.bayNumber = i;
                        rv = soap_getInterconnectTrayInfo(con, &info_request,
                                                          &info_response);
                        if (rv != SOAP_OK) {
                                err("Get interconnect tray status failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        /* If serial number is different, remove and add the
                         * interconnect
                         */
                        if (strcmp(oa_handler->oa_soap_resources.interconnect.
                                   serial_number[i - 1],
                                   info_response.serialNumber) != 0) {
                                replace_resource = SAHPI_TRUE;
                        } else {
                                /* Check and update the hotswap state of the
                                 * server blade
                                 */
                                rv = update_interconnect_hotswap_state(
                                        oh_handler, con, i);
                                if (rv != SA_OK) {
                                        err("update interconnect hot swap"
                                            " state failed");
                                        return rv;
                                }
				/* Check the interconnect sensors state */
				rv = oa_soap_re_disc_interconct_sen(
							oh_handler, con, i);
				if (rv != SA_OK) {
					err("Re-discover interconnect sensors "
					    "failed");
					return rv;
				}
                                continue;
                        }
                } else
                        state = RES_PRESENT;

                if (state == RES_ABSENT || replace_resource == SAHPI_TRUE) {
                        /* The interconnect is present according OA presence
                         * matrix, but interconnect is removed.  Remove the
                         * interconnect resource from RPTable.
                         */
                        rv = remove_interconnect(oh_handler, i);
                        if (rv != SA_OK) {
                                err("Interconnect blade %d removal failed",
                                    i);
                                return rv;
                        } else
                                err("Interconnect blade %d removed", i);
                }

                if (state == RES_PRESENT || replace_resource == SAHPI_TRUE) {
                        /* The interconnect is absent according OA presence
                         * matrix, but interconnect is added.  Add the
                         * interconnect resource to RPTable.
                         */
                        rv = add_interconnect(oh_handler, con, i);
                        if (rv != SA_OK) {
                                err("Interconnect blade %d add failed", i);
                                return rv;
                        } else
                                err("Interconnect blade %d added", i);
                }
        }
        return SA_OK;
}

/**
 * update_interconnect_hotswap_state
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to the SOAP_CON structure
 *      @bay_number: Bay number of the removed blade
 *
 * Purpose:
 *      Updates the interconnect hot swap state in RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT update_interconnect_hotswap_state(struct oh_handler_state *oh_handler,
                                           SOAP_CON *con,
                                           SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt = NULL;
        struct oa_soap_hotswap_state *hotswap_state = NULL;
        struct oh_event event;
        SaHpiPowerStateT state;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler;

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->
                oa_soap_resources.interconnect.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(oh_handler->rptcache, rpt->ResourceId);
        if (hotswap_state == NULL) {
                err("Unable to get the resource private data");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        rv = get_interconnect_power_state(con, bay_number, &state);
        if (rv != SA_OK) {
                err("Unable to get power status");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check whether current hotswap state of the interconnect is same as
         * in hotswap structure in rpt entry
         */
        if ((state == SAHPI_POWER_ON &&
             hotswap_state->currentHsState == SAHPI_HS_STATE_ACTIVE) ||
            (state == SAHPI_POWER_OFF &&
             hotswap_state->currentHsState == SAHPI_HS_STATE_INACTIVE)) {
                return SA_OK;
        }

        update_hotswap_event(oh_handler, &event);
        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;
        switch (state) {
                case SAHPI_POWER_ON:
                        /* Interconnect got powered on.  Update the hotswap
                         * structure.
                         */
                        hotswap_state->currentHsState = SAHPI_HS_STATE_ACTIVE;
                        event.rdrs = NULL;
                        /* Raise the hotswap events */
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState = SAHPI_HS_STATE_INACTIVE;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_INSERTION_PENDING;
                        /* The cause of the state change is unknown */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_UNKNOWN;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));

                        event.rdrs = NULL;
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState =
                                SAHPI_HS_STATE_INSERTION_PENDING;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_ACTIVE;
                        /* INSERTION_PENDING to ACTIVE state change happened
                         * due to auto policy of server blade
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));
                        break;

                case SAHPI_POWER_OFF:
                        /* Interconnect got powered off.  Update the hotswap
                         * structure.
                         */
                        hotswap_state->currentHsState = SAHPI_HS_STATE_INACTIVE;
                        event.rdrs = NULL;
                        /* Raise the hotswap events */
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_EXTRACTION_PENDING;
                        /* ACTIVE to EXTRACTION_PENDING state change happened
                         * due power off event.
                         * The deactivation can not be stopped.
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange =
                                SAHPI_HS_CAUSE_UNEXPECTED_DEACTIVATION;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));

                        event.rdrs = NULL;
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState =
                                SAHPI_HS_STATE_EXTRACTION_PENDING;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_INACTIVE;
                        /* EXTRACTION_PENDING to INACTIVE state change happened
                         * due to auto policy of server blade
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));
                        break;

                default:
                        err("unknown power status");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * remove_interconnect
 *      @oh_handler: Pointer to openhpi handler
 *      @bay_number: Bay number of the removed interconnect
 *
 * Purpose:
 *      Removes the interconnect.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT remove_interconnect(struct oh_handler_state *oh_handler,
                             SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        struct oh_event event;
        struct oa_soap_hotswap_state *hotswap_state;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        update_hotswap_event(oh_handler, &event);

        resource_id = oa_handler->
                oa_soap_resources.interconnect.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;
	event.event.Severity = event.resource.ResourceSeverity;

        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(oh_handler->rptcache,
                                     event.resource.ResourceId);
        if (hotswap_state == NULL) {
                err("Failed to get hotswap state");
                event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                        SAHPI_HS_STATE_INACTIVE;
        }
        else
                event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                        hotswap_state->currentHsState;

        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;

        if (hotswap_state->currentHsState == SAHPI_HS_STATE_INACTIVE) {
                /* INACTIVE to NOT_PRESENT state change happened due to
                 * operator action
                 */
                event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                        SAHPI_HS_CAUSE_OPERATOR_INIT;
        } else {
                /* This state change happened due to surprise extraction */
                event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                        SAHPI_HS_CAUSE_SURPRISE_EXTRACTION;
        }

        /* Push the hotswap event to remove the resource from OpenHPI RPTable */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        /* Free the inventory info from inventory RDR */
        rv = free_inventory_info(oh_handler, event.resource.ResourceId);
        if (rv != SA_OK) {
                err("Inventory cleanup failed for resource id %d",
                     rpt->ResourceId);
        }
        /* Remove the resource from plugin RPTable */
        rv = oh_remove_resource(oh_handler->rptcache,
                                event.resource.ResourceId);

        /* reset resource_status structure to default values */
        oa_soap_update_resource_status(
              &oa_handler->oa_soap_resources.interconnect, bay_number,
              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
        return SA_OK;
}

/**
 * add_interconnect
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to the SOAP_CON structure
 *      @bay_number: Bay number of the removed interconnect
 *
 * Purpose:
 *      Adds the interconnect.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT add_interconnect(struct oh_handler_state *oh_handler,
                          SOAP_CON *con,
                          SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        struct getInterconnectTrayInfo info;
        struct interconnectTrayInfo response;
        struct oh_event event;
        SaHpiPowerStateT state;
        SaHpiResourceIdT resource_id;
	GSList *asserted_sensors = NULL;
	SaHpiRptEntryT *rpt;

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        info.bayNumber = bay_number;
        rv = soap_getInterconnectTrayInfo(con, &info, &response);
        if (rv != SOAP_OK) {
                err("Get Interconnect tray info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Build the rpt entry */
        rv = build_interconnect_rpt(oh_handler, con,
                                    response.name,
                                    bay_number, &resource_id, FALSE);
        if (rv != SA_OK) {
                err("Failed to get interconnect inventory RPT");
                return rv;
        }

        /* Update resource_status structure with resource_id, serial_number,
         * and presence status
         */
        oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.interconnect, bay_number,
                      response.serialNumber, resource_id, RES_PRESENT);

        /* Build the RDRs */
        rv = build_interconnect_rdr(oh_handler, con,
                                    bay_number, resource_id);
        if (rv != SA_OK) {
                err("Failed to get interconnect inventory RDR");
                /* Free the inventory info from inventory RDR */
                rv = free_inventory_info(oh_handler, resource_id);
                if (rv != SA_OK) {
                        err("Inventory cleanup failed for resource id %d",
                             resource_id);
                }
                oh_remove_resource(oh_handler->rptcache, resource_id);
                /* reset resource_status structure to default values */
                oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.interconnect,
                              bay_number,
                              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                return rv;
        }

        rv = oa_soap_populate_event(oh_handler, resource_id, &event,
				    &asserted_sensors);
        if (rv != SA_OK) {
                err("Populating event struct failed");
                return rv;
        }

        /* Raise the hotswap event for the inserted interconnect blade */
        event.event.EventType = SAHPI_ET_HOTSWAP;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_INSERTION_PENDING;
        /* NOT_PRESENT to INSERTION_PENDING state change happened due
         * to operator action
         */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_OPERATOR_INIT;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        event.rdrs = NULL;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_INSERTION_PENDING;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        /* INSERTION_PENDING to ACTIVE state change happened
         * due to auto policy of server blade
         */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_AUTO_POLICY;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        /* Check the power state of the interconnect.  If the power state is
         * off, may be interconnect got powered off after inserting.
         * Inserting the interconnect makes to power on automatically.
         * Hence raise the hotswap events for power off.
         */
        rv = get_interconnect_power_state(con, bay_number, &state);
        if (rv != SA_OK) {
                err("Unable to get power status");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        switch (state) {
                case SAHPI_POWER_ON:
                        break;

                case SAHPI_POWER_OFF:
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_EXTRACTION_PENDING;
                        /* ACTIVE to EXTRACTION_PENDING state change happened
                         * due power off event.
                         * The deactivation can not be stopped.
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange =
                                SAHPI_HS_CAUSE_UNEXPECTED_DEACTIVATION;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));

                        event.rdrs = NULL;
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState =
                                SAHPI_HS_STATE_EXTRACTION_PENDING;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_INACTIVE;
                        /* EXTRACTION_PENDING to INACTIVE state change happened
                         * due to auto policy of server blade
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));
                        break;

                default:
                        err("unknown power status");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

	/* Raise the assert sensor events */
	if (asserted_sensors) {
	        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
		oa_soap_assert_sen_evt(oh_handler, rpt, asserted_sensors);
	}

        return SA_OK;
}

/**
 * re_discover_fan
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to the SOAP_CON structure
 *
 * Purpose:
 *      Re-discover the Fans.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT re_discover_fan(struct oh_handler_state *oh_handler,
                         SOAP_CON *con)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        struct getFanInfo request;
        struct fanInfo response;
        SaHpiInt32T i;
        enum resource_presence_status state = RES_ABSENT;

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        for (i = 1; i <= oa_handler->oa_soap_resources.fan.max_bays; i++) {
                request.bayNumber = i;
                rv = soap_getFanInfo(con, &request, &response);
                if (rv != SOAP_OK) {
                        err("Get fan info failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                if (response.presence != PRESENT) {
                        /* The Fan is absent, check Fan is absent in presence
                         * matrix
                         */
                        if (oa_handler->oa_soap_resources.fan.presence[i - 1] ==
                            RES_ABSENT)
                                continue;
                        else
                                state = RES_ABSENT;
                } else {
                        /* The Fan is present, check Fan is present in presence
                         * matrix
                         */
                        if (oa_handler->oa_soap_resources.fan.presence[i - 1] ==
                            RES_PRESENT) {
				/* Check the fan sensors state */
				oa_soap_proc_fan_status(oh_handler, &response);
                                continue;
                        } else
                                state = RES_PRESENT;
                }

                if (state == RES_ABSENT) {
                        /* The Fan is present according to Fan presence matrix,
                         * but Fan is removed.  Remove the Fan resource from
                         * RPTable.
                         */
                        rv = remove_fan(oh_handler, i);
                        if (rv != SA_OK) {
                                err("Fan %d removal failed", i);
                                return rv;
                        } else
                                err("Fan %d removed", i);
                } else if (state == RES_PRESENT) {
                        /* The Fan is absent according Fan presence matrix,
                         * but Fan is present.  Add the Fan resource from
                         * RPTable.
                         */
                        rv = add_fan(oh_handler, con, &response);
                        if (rv != SA_OK) {
                                err("Fan %d add failed", i);
                                return rv;
                        } else
                                err("Fan %d added", i);
                }
        }
        return SA_OK;
}

/**
 * remove_fan
 *      @oh_handler: Pointer to openhpi handler
 *      @bay_number: Bay number of the fan
 *
 * Purpose:
 *      Remove the Fan from OpenHPI infrastructure.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT remove_fan(struct oh_handler_state *oh_handler,
                    SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        SaHpiRptEntryT *rpt = NULL;
        struct oh_event event;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        update_hotswap_event(oh_handler, &event);

        resource_id =
           oa_handler->oa_soap_resources.fan.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.rdrs = NULL;
        event.event.Source = event.resource.ResourceId;
	event.event.Severity = event.resource.ResourceSeverity;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;
        /* This state change happened due to surprise extraction */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_SURPRISE_EXTRACTION;

        /* Push the hotswap event to remove the resource from OpenHPI RPTable */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        /* Free the inventory info from inventory RDR */
        rv = free_inventory_info(oh_handler, event.resource.ResourceId);
        if (rv != SA_OK) {
                err("Inventory cleanup failed for resource id %d",
                     rpt->ResourceId);
        }
        /* Remove the resource from plugin RPTable */
        rv = oh_remove_resource(oh_handler->rptcache,
                                event.resource.ResourceId);

        /* reset resource_status structure to default values */
        oa_soap_update_resource_status(
              &oa_handler->oa_soap_resources.fan, bay_number,
              NULL, SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);

        return SA_OK;
}

/**
 * add_fan
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer to SOAP_CON structure
 *      @info:       Pointer to the get fan info response structure
 *
 * Purpose:
 *      Add the fan information to OpenHPI infrastructure.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT add_fan(struct oh_handler_state *oh_handler,
                 SOAP_CON *con,
                 struct fanInfo *info)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        struct oh_event event;
        SaHpiResourceIdT resource_id;
	GSList *asserted_sensors = NULL;
	SaHpiRptEntryT *rpt;

        if (oh_handler == NULL || con == NULL || info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Build the rpt entry */
        rv = oa_soap_build_fan_rpt(oh_handler, info->bayNumber, &resource_id);
        if (rv != SA_OK) {
                err("Failed to populate fan RPT");
                return rv;
        }

        /* Update resource_status structure with resource_id, serial_number,
         * and presence status.  Fan doesn't have serial number, so pass in
         * a null string.
         */
        oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.fan, info->bayNumber,
                      NULL, resource_id, RES_PRESENT);

        /* Build the RDRs */
        rv = oa_soap_build_fan_rdr(oh_handler, con, info, resource_id);
        if (rv != SA_OK) {
                err("Failed to populate fan RDR");
                /* Free the inventory info from inventory RDR */
                rv = free_inventory_info(oh_handler, resource_id);
                if (rv != SA_OK) {
                        err("Inventory cleanup failed for resource id %d",
                             resource_id);
                }
                oh_remove_resource(oh_handler->rptcache, resource_id);
                /* reset resource_status structure to default values */
                oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.fan,
                              info->bayNumber,
                              NULL, SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rv = oa_soap_populate_event(oh_handler, resource_id, &event,
				    &asserted_sensors);
        if (rv != SA_OK) {
                err("Populating event struct failed");
                return rv;
        }

        event.event.EventType = SAHPI_ET_HOTSWAP;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        /* NOT_PRESENT to ACTIVE state change happened due to operator action */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_OPERATOR_INIT;
        /* Push the hotswap event to add the resource to OpenHPI RPTable */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

	/* Raise the assert sensor events */
	if (asserted_sensors) {
	        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
		oa_soap_assert_sen_evt(oh_handler, rpt, asserted_sensors);
	}

        return SA_OK;
}

/**
 * re_discover_ps_unit
 *      @oh_handler: Pointer to openhpi handler
 *      @con: Pointer to the SOAP_CON structure
 *
 * Purpose:
 *      Re-discover the Power Supply Units.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT re_discover_ps_unit(struct oh_handler_state *oh_handler,
                             SOAP_CON *con)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        struct getPowerSupplyInfo request;
        struct powerSupplyInfo response;
        SaHpiInt32T i;
        enum resource_presence_status state = RES_ABSENT;
        SaHpiBoolT replace_resource = SAHPI_FALSE;

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        for (i = 1; i <= oa_handler->oa_soap_resources.ps_unit.max_bays; i++) {
                request.bayNumber = i;
                rv = soap_getPowerSupplyInfo(con, &request, &response);
                if (rv != SOAP_OK) {
                        err("Get power supply info failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* If the power supply unit does not have the power cord
                 * plugged in, then power supply unit will be in faulty
                 * condition. In this case, all the information in the
                 * response structure is NULL. Consider the faulty power supply
                 * unit as ABSENT
                 */
                if (response.presence != PRESENT ||
                    response.serialNumber == NULL) {
                        /* The power supply unit is absent.  Is the power
                         * supply unit absent in the presence matrix?
                         */
                        if (oa_handler->
                                oa_soap_resources.ps_unit.presence[i - 1] ==
                            RES_ABSENT)
                                continue;
                        else
                                state = RES_ABSENT;
                } else {
                        /* The power supply unit is present.  Is the power
                         * supply unit present in the presence matrix?
                         */
                        if (oa_handler->
                                oa_soap_resources.ps_unit.presence[i - 1] ==
                            RES_PRESENT) {

                                /* If serial number is diferent,
                                 * remove and add the power supply
                                 */
                                if (strcmp(oa_handler->oa_soap_resources.
                                           ps_unit.serial_number[i - 1],
                                           response.serialNumber) != 0) {
                                        replace_resource = SAHPI_TRUE;
                                 } else {
					/* Check the power supply sensors
					 * state
					 */
				       rv = oa_soap_re_disc_ps_sen(
							oh_handler, con, i);
					if (rv != SA_OK) {
						err("Re-discover power supply "
						    "sensors failed");
						return rv;
					}
                                        continue;
				}
                        } else
                                state = RES_PRESENT;
                }

                if (state == RES_ABSENT || replace_resource == SAHPI_TRUE) {
                        /* The power supply unit is present according power
                         * supply presence matrix, but power supply unit is
                         * removed.  Remove the power supply unit resource
                         * from RPTable.
                         */
                        rv = remove_ps_unit(oh_handler, i);
                        if (rv != SA_OK) {
                                err("Power Supply Unit %d removal failed", i);
                                return rv;
                        } else
                                err("Power Supply Unit %d removed", i);
                }

                if (state == RES_PRESENT || replace_resource == SAHPI_TRUE) {
                        /* The power supply unit is absent according power
                         * supply presence matrix, but power supply unit is
                         * added.  Add the power supply unit resource from
                         * RPTable.
                         */
                        rv = add_ps_unit(oh_handler, con, &response);
                        if (rv != SA_OK) {
                                err("Power Supply Unit %d add failed", i);
                                return rv;
                        } else
                                err("Power Supply Unit %d added", i);

                        replace_resource = SAHPI_FALSE;
                }
        } /* End of for loop */
        return SA_OK;
}

/**
 * remove_ps_unit
 *      @oh_handler: Pointer to openhpi handler
 *      @bay_number: Bay number of the fan
 *
 * Purpose:
 *      Remove the Power Supply Unit.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT remove_ps_unit(struct oh_handler_state *oh_handler,
                        SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        SaHpiRptEntryT *rpt = NULL;
        struct oh_event event;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        update_hotswap_event(oh_handler, &event);

        resource_id =
           oa_handler->oa_soap_resources.ps_unit.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;
	event.event.Severity = event.resource.ResourceSeverity;

        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;
        /* This state change happened due to surprise extraction */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_SURPRISE_EXTRACTION;
        /* Push the hotswap event to remove the resource from OpenHPI RPTable */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        /* Free the inventory info from inventory RDR */
        rv = free_inventory_info(oh_handler, event.resource.ResourceId);
        if (rv != SA_OK) {
                err("Inventory cleanup failed for resource id %d",
                     rpt->ResourceId);
        }
        /* Remove the resource from plugin RPTable */
        rv = oh_remove_resource(oh_handler->rptcache,
                                event.resource.ResourceId);

        /* reset resource_status structure to default values */
        oa_soap_update_resource_status(
              &oa_handler->oa_soap_resources.ps_unit, bay_number,
              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);

        return SA_OK;
}

/**
 * add_ps_unit
 *      @oh_handler: Pointer to openhpi handler
 *      @con:        Pointer SOAP_CON structure
 *      @info:       Pointer to the get power supply info response structure
 *
 * Purpose:
 *      Add the Power Supply Unit information to RPTable.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT add_ps_unit(struct oh_handler_state *oh_handler,
                     SOAP_CON *con,
                     struct powerSupplyInfo *info)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        char power_supply_disp[] = POWER_SUPPLY_NAME;
        struct oh_event event;
        SaHpiResourceIdT resource_id;
        struct getPowerSupplyInfo request;
        struct powerSupplyInfo response;
	GSList *asserted_sensors = NULL;
	SaHpiRptEntryT *rpt;

        if (oh_handler == NULL || con == NULL || info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        update_hotswap_event(oh_handler, &event);

        /* Get power supply info to obtain the serial number */
        request.bayNumber = info->bayNumber;
        rv = soap_getPowerSupplyInfo(con, &request, &response);
        if (rv != SOAP_OK) {
                err("Get power supply info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Build the rpt entry */
        rv = build_power_supply_rpt(oh_handler, power_supply_disp,
                                    info->bayNumber, &resource_id);
        if (rv != SA_OK) {
                err("build power supply rpt failed");
                return rv;
        }

        /* Update resource_status structure with resource_id, serial_number,
         * and presence status
         */
        oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.ps_unit, info->bayNumber,
                      response.serialNumber, resource_id, RES_PRESENT);

        /* Build the RDRs */
        rv = build_power_supply_rdr(oh_handler, con, info, resource_id);
        if (rv != SA_OK) {
                err("build power supply RDR failed");
                /* Free the inventory info from inventory RDR */
                rv = free_inventory_info(oh_handler, resource_id);
                if (rv != SA_OK) {
                        err("Inventory cleanup failed for resource id %d",
                             resource_id);
                }
                oh_remove_resource(oh_handler->rptcache, resource_id);
                /* reset resource_status structure to default values */
                oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.ps_unit,
                              info->bayNumber,
                              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rv = oa_soap_populate_event(oh_handler, resource_id, &event,
				    &asserted_sensors);
        if (rv != SA_OK) {
                err("Populating event struct failed");
                return rv;
        }

        event.event.EventType = SAHPI_ET_HOTSWAP;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        /* NOT_PRESENT to ACTIVE state change happened due to operator action */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_OPERATOR_INIT;
        /* Push the hotswap event to add the resource to OpenHPI RPTable */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

	/* Raise the assert sensor events */
	if (asserted_sensors) {
	        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
		oa_soap_assert_sen_evt(oh_handler, rpt, asserted_sensors);
	}

        return SA_OK;
}

/**
 * oa_soap_re_disc_oa_sen
 *      @oh_handler	: Pointer to openhpi handler
 *      @con		: Pointer SOAP_CON structure
 *      @bay_number	: OA bay nubmer
 *
 * Purpose:
 *	Re-discovers the OA sensor states
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_re_disc_oa_sen(struct oh_handler_state *oh_handler,
				       SOAP_CON *con,
				       SaHpiInt32T bay_number)
{
	SaErrorT rv = SA_OK;
	struct oa_soap_handler *oa_handler = NULL;
	struct getOaStatus request;
	struct oaStatus response;
	struct getOaNetworkInfo nw_info_request;
	struct oaNetworkInfo nw_info_response;

	if (oh_handler == NULL || con == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	request.bayNumber = bay_number;
	rv = soap_getOaStatus(con, &request, &response);
	if (rv != SOAP_OK) {
		err("Get OA status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check the OA sensor states */
	oa_soap_proc_oa_status(oh_handler, &response);

	nw_info_request.bayNumber = bay_number;
	rv = soap_getOaNetworkInfo(con, &nw_info_request, &nw_info_response);
	if (rv != SOAP_OK) {
		err("Get OA network info SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check the OA link status state */
	oa_soap_proc_oa_network_info(oh_handler, &nw_info_response);

	return SA_OK;
}

/**
 * oa_soap_re_disc_server_sen
 *      @oh_handler	: Pointer to openhpi handler
 *      @con		: Pointer SOAP_CON structure
 *      @bay_number	: Server bay nubmer
 *
 * Purpose:
 *	Re-discovers the server sensor states
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_re_disc_server_sen(struct oh_handler_state *oh_handler,
					   SOAP_CON *con,
					   SaHpiInt32T bay_number)
{
	SaErrorT rv = SA_OK;
	struct getBladeStatus request;
	struct bladeStatus response;

	if (oh_handler == NULL || con == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	request.bayNumber = bay_number;
	rv = soap_getBladeStatus(con, &request, &response);
	if (rv != SOAP_OK) {
		err("Get OA status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check the server sensor states */
	oa_soap_proc_server_status(oh_handler, con, &response);

	return SA_OK;
}

/**
 * oa_soap_re_disc_interconct_sen
 *      @oh_handler	: Pointer to openhpi handler
 *      @con		: Pointer SOAP_CON structure
 *      @bay_number	: Interconnect bay nubmer
 *
 * Purpose:
 *	Re-discovers the interconnect sensor states
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_re_disc_interconct_sen(struct oh_handler_state
							*oh_handler,
					      SOAP_CON *con,
					      SaHpiInt32T bay_number)
{
	SaErrorT rv = SA_OK;
	struct getInterconnectTrayStatus request;
	struct interconnectTrayStatus response;

	if (oh_handler == NULL || con == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	request.bayNumber = bay_number;
	rv = soap_getInterconnectTrayStatus(con, &request, &response);
	if (rv != SOAP_OK) {
		err("Get OA status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check the interconnect sensor states */
	oa_soap_proc_interconnect_status(oh_handler, &response);

	/* Check the interconnect thermal sensor state */
	oa_soap_proc_interconnect_thermal(oh_handler, con, &response);

	return SA_OK;
}

/**
 * oa_soap_re_disc_ps_sen
 *      @oh_handler	: Pointer to openhpi handler
 *      @con		: Pointer SOAP_CON structure
 *      @bay_number	: Power supply bay nubmer
 *
 * Purpose:
 *	Re-discovers the power supply sensor states
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_re_disc_ps_sen(struct oh_handler_state *oh_handler,
				       SOAP_CON *con,
				       SaHpiInt32T bay_number)
{
	SaErrorT rv = SA_OK;
	struct getPowerSupplyStatus request;
	struct powerSupplyStatus response;

	if (oh_handler == NULL || con == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	request.bayNumber = bay_number;
	rv = soap_getPowerSupplyStatus(con, &request, &response);
	if (rv != SOAP_OK) {
		err("Get OA status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check the power supply sensor states */
	oa_soap_proc_ps_status(oh_handler, &response);

	return SA_OK;
}

/**
 * oa_soap_re_disc_enc_sen
 *      @oh_handler	: Pointer to openhpi handler
 *      @con		: Pointer SOAP_CON structure
 *
 * Purpose:
 *	Re-discovers the enclosure sensor states
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_re_disc_enc_sen(struct oh_handler_state *oh_handler,
					SOAP_CON *con)
{
	SaErrorT rv = SA_OK;
	struct enclosureStatus response;

	if (oh_handler == NULL || con == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	rv = soap_getEnclosureStatus(con, &response);
	if (rv != SOAP_OK) {
		err("Get enclosure status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check the enclosure sensor states */
	oa_soap_proc_enc_status(oh_handler, &response);

	return SA_OK;
}

/**
 * oa_soap_re_disc_ps_subsys_sen
 *      @oh_handler	: Pointer to openhpi handler
 *      @con		: Pointer SOAP_CON structure
 *
 * Purpose:
 *	Re-discovers the power subsystem sensor states
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_re_disc_ps_subsys_sen(struct oh_handler_state
							*oh_handler,
					      SOAP_CON *con)
{
	SaErrorT rv = SA_OK;
	struct powerSubsystemInfo response;

	if (oh_handler == NULL || con == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	rv = soap_getPowerSubsystemInfo(con, &response);
	if (rv != SOAP_OK) {
		err("Get enclosure status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check the power subsystem sensor states */
	oa_soap_proc_ps_subsys_info(oh_handler, &response);

	return SA_OK;
}

/**
 * oa_soap_re_disc_lcd_sen
 *      @oh_handler	: Pointer to openhpi handler
 *      @con		: Pointer SOAP_CON structure
 *
 * Purpose:
 *	Re-discovers the LCD sensor states
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_re_disc_lcd_sen(struct oh_handler_state *oh_handler,
					SOAP_CON *con)
{
	SaErrorT rv = SA_OK;
	struct lcdStatus response;

	if (oh_handler == NULL || con == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	rv = soap_getLcdStatus(con, &response);
	if (rv != SOAP_OK) {
		err("Get LCD status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check the LCD sensor states */
	oa_soap_proc_lcd_status(oh_handler, &response);

	return SA_OK;
}

/**
 * oa_soap_re_disc_fz_sen
 *      @oh_handler	: Pointer to openhpi handler
 *      @con		: Pointer SOAP_CON structure
 *
 * Purpose:
 *	Re-discovers the fan_zone sensor states
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_re_disc_fz_sen(struct oh_handler_state *oh_handler,
				       SOAP_CON *con)
{
	SaErrorT rv = SA_OK;
	struct oa_soap_handler *oa_handler = NULL;
	struct getFanZoneArrayResponse response;
	struct fanZone fan_zone;
	SaHpiInt32T max_fz;

	if (oh_handler == NULL || con == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	max_fz = oa_handler->oa_soap_resources.fan_zone.max_bays;

	/* Get the Fan Zone array information */
	rv = oa_soap_get_fz_arr(oa_handler, max_fz, &response);
	if (rv != SOAP_OK) {
		err("Get fan zone array failed");
		return rv;
	}

	while (response.fanZoneArray) {
		soap_fanZone(response.fanZoneArray, &fan_zone);

		/* Check the fan zone sensor states */
		oa_soap_proc_fz_status(oh_handler, &fan_zone);

		response.fanZoneArray = soap_next_node(response.fanZoneArray);
	}

	return SA_OK;
}

/**
 * oa_soap_re_disc_therm_subsys_sen
 *      @oh_handler	: Pointer to openhpi handler
 *      @con		: Pointer SOAP_CON structure
 *
 * Purpose:
 *	Re-discovers the thermal subsystem sensor states
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_re_disc_therm_subsys_sen(struct oh_handler_state
							*oh_handler,
						SOAP_CON *con)
{
	SaErrorT rv = SA_OK;
	struct thermalSubsystemInfo response;

	if (oh_handler == NULL || con == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	rv = soap_getThermalSubsystemInfo(con, &response);
	if (rv != SOAP_OK) {
		err("Get thermal subsystem info SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check the thermal subsystem sensor states */
	oa_soap_proc_therm_subsys_info(oh_handler, &response);

	return SA_OK;
}
