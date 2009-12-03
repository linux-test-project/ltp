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
 *      Shuah Khan <shuah.khan@hp.com>
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Mohan Devarajulu <mohan@fc.hp.com>
 *
 * This file has the server blade related events handling
 *
 *      process_server_power_off_event()  - Processes the server power off event
 *
 *      process_server_power_on_event()   - Processes the server power on event
 *
 *      process_server_power_event()      - Processes the server power event
 *
 *      process_server_insertion_event()  - Processes the server insertion event
 *
 *      process_server_extraction_event() - Processes the server extraction
 *                                          event
 *
 *      build_inserted_server_rpt()       - Builds the rpt entry for inserted
 *                                          server
 *
 *	oa_soap_proc_server_status() 	  - Processes the server status event
 *
 *	oa_soap_serv_post_comp ()	  - Processes the blade post complete 
 *					    event
 *
 *	oa_soap_set_thermal_sensor ()	  - Enables or Disables the thermal
 *					    sensors associated with blade
 */	

#include "oa_soap_server_event.h"
#include "oa_soap_discover.h"           /* for build_server_rpt() prototype */

/**
 * process_server_power_off_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @event:      Pointer to the openhpi event structure
 *
 * Purpose:
 *      Creates the server power off hotswap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_server_power_off_event(struct oh_handler_state *oh_handler,
                                       struct oh_event *event)
{
        struct oa_soap_hotswap_state *hotswap_state = NULL;

        if (oh_handler == NULL || event == NULL) {
                err("wrong parameters passed");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(oh_handler->rptcache,
                                     event->resource.ResourceId);
        if (hotswap_state == NULL) {
                err("Failed to get server hotswap state");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Sometimes OA sends duplicate power off event
         * Check whether hotswap state is not in ACTIVE
         * If yes, then ignore power off event
         */
        if (hotswap_state->currentHsState != SAHPI_HS_STATE_ACTIVE) {
                dbg("blade is not in proper state");
                dbg("ignoring the power off event");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Raise the server power off hotswap event */
        event->rdrs = NULL;
        event->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        event->event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_EXTRACTION_PENDING;
        /* ACTIVE to EXTRACTION_PENDING state change can not be stopped.
         * Hence, this is unexpected deactivation
         */
        event->event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_UNEXPECTED_DEACTIVATION;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(event));

        event->rdrs = NULL;
        event->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_EXTRACTION_PENDING;
        event->event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_INACTIVE;
        /* EXTRACTION_PENDING to INACTIVE state change happens due to auto
         * policy of server blade
         */
        event->event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_AUTO_POLICY;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(event));

        event->resource.ResourceSeverity = SAHPI_CRITICAL;
        hotswap_state->currentHsState = SAHPI_HS_STATE_INACTIVE;

        return SA_OK;
}

/**
 * process_server_power_on_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @con:        Pointer to SOAP_CON structure
 *      @bay_number: Bay number of the server blade
 *      @event:      Pointer to the openhpi event structure
 *
 * Purpose:
 *      Creates the server power on hotswap event.  If the sever blade
 *      was powered on after insertion, then the INSERTION_PENDING to
 *      ACTIVE hot swap event is generated.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_server_power_on_event(struct oh_handler_state *oh_handler,
                                       SOAP_CON *con,
                                       struct oh_event *event,
                                       SaHpiInt32T bay_number)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
        struct oa_soap_hotswap_state *hotswap_state = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;
        SaHpiRdrT *rdr = NULL;
        SaHpiIdrIdT sen_rdr_num = OA_SOAP_SEN_TEMP_STATUS;

        if (oh_handler == NULL || con == NULL || event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(oh_handler->rptcache,
                                     event->resource.ResourceId);
        if (hotswap_state == NULL) {
                err("Failed to get hotswap state of server blade");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        event->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                                hotswap_state->currentHsState;
        /* Check whether blade is inserted and then powered on */
        switch (hotswap_state->currentHsState) {
                case (SAHPI_HS_STATE_INSERTION_PENDING):
                        hotswap_state->currentHsState = SAHPI_HS_STATE_ACTIVE;
                        event->event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_ACTIVE;
                        /* INSERTION_PENDING to ACTIVE state change happens due
                         * to auto policy of server blade
                         */
                        event->event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(event));
                        break;

                case (SAHPI_HS_STATE_INACTIVE):
                        event->resource.ResourceSeverity = SAHPI_OK;
                        /* The previous state of the server was power off
                         * Update the current hotswap state to ACTIVE
                         */
                        hotswap_state->currentHsState = SAHPI_HS_STATE_ACTIVE;
                        if (rv != SA_OK) {
                                err("Failed to add hot swap state");
                        }

                        rdr = oh_get_rdr_by_type(oh_handler->rptcache,
                                                 event->resource.ResourceId,
                                                 SAHPI_SENSOR_RDR,
                                                 sen_rdr_num);

                        if (rdr == NULL) {
                                err("RDR not present");
                                return SA_ERR_HPI_NOT_PRESENT;
                        }

                        /* Get the thermal sensor information of the server */
                        sensor_info = (struct oa_soap_sensor_info*)
                                oh_get_rdr_data(oh_handler->rptcache,
                                                event->resource.ResourceId,
                                                rdr->RecordId);
                        if (sensor_info == NULL) {
                                err("No sensor data. Sensor=%s",
                                    rdr->IdString.Data);
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }

                        /* Since the server got powered off
                         * Clear the thermal sensor states
                         */
                        sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
                        sensor_info->previous_state = SAHPI_ES_UNSPECIFIED;

                        /* Raise the server power on hotswap event */
                        event->rdrs = NULL;
                        event->event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_INSERTION_PENDING;
                        /* The cause of the state change is unknown */
                        event->event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_UNKNOWN;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(event));

                        event->rdrs = NULL;
                        event->event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState =
                                SAHPI_HS_STATE_INSERTION_PENDING;
                        event->event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_ACTIVE;
                        /* INSERTION_PENDING to ACTIVE state change happens due
                         * to Auto policy of server blade
                         */
                        event->event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(event));
                        break;

                default:
                        err("wrong state detected");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * process_server_power_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @con:        Pointer to SOAP_CON structure
 *      @oa_event:   Pointer to OA event structure
 *
 * Purpose:
 *      Creates the server power hpi hotswap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_server_power_event(struct oh_handler_state *oh_handler,
                                    SOAP_CON *con,
                                    struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt = NULL;
        struct oa_soap_handler *oa_handler = NULL;
        struct oa_soap_hotswap_state hotswap_state;
        SaHpiInt32T bay_number, loc=1;
        struct oh_event event;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL || con == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        update_hotswap_event(oh_handler, &event);

        bay_number = oa_event->eventData.bladeStatus.bayNumber;
        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.server.resource_id[bay_number - 1];

        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                /* rpt does not exist When EVENT_BLADE_POWER_STATE comes 
                   before EVENT_BLADE_INSERT_COMPLETED event. But some times 
                   (<5%) OA sends these two events out of order.
                   EVENT_BLADE_INSERT_COMPLETED creates the rpt entry. 
                   EVENT_BLADE_POWER_STATE with POWER_ON comes only if the
                   "Automatically power on server" is set to yes for that
                   blade in the iLO, otherwise it does not.

                   This workaround fixes the problem by doing opposite of 
                   what OA is doing, when it sends the events out of order.

                   a. When the EVENT_BLADE_POWER_STATE comes when the RPT is 
                   empty for that blade, then assume that we missed the 
                   EVENT_BLADE_INSERT_COMPLETED event and execute that code. 

                   b. Avoid calling EVENT_BLADE_POWER_STATE code by knowing 
                   where it is called from and whether the POWER_ON state is 
                   set or not. POWER_ON is set in EVENT_BLADE_INSERT_COMPLETED 
                   event, if it arrives later.

                   c. When the EVENT_BLADE_INSERT_COMPLETED eventually comes 
                   with the POWER_ON state call the EVENT_BLADE_POWER_STATE 
                   code to set the active state.

                   d. When OA fixes their code, this workaround code will not 
                   get executed at all. */

                dbg("resource RPT is NULL, starting Workaround");
                rv = process_server_insertion_event(oh_handler, con, oa_event, loc); 
                return rv;
        }

        /* For blades that do not support managed hotswap, ignore power event */
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
		dbg("Ignoring the power event for blade %d", bay_number);
		return SA_OK;
	}

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;

        switch (oa_event->eventData.bladeStatus.powered) {
                case (POWER_OFF):
                        rv = process_server_power_off_event(oh_handler, &event);

			/* Walk through the rdr list of the resource and 
			 * disables thermal sensors associated with server, as it
			 * cannot perform the sensor monitoring
			 * For disabling the thermal sensor, 
			 * Response structure pointer is passed as NULL, since
			 * it not utilized for disable operation.
			 */
			rv = oa_soap_set_thermal_sensor (oh_handler, rpt, NULL,
							 SAHPI_FALSE);
			if (rv != SA_OK) {
				err("Failure in disabling thermal sensors");
				oa_soap_bay_pwr_status[bay_number -1] = SAHPI_POWER_OFF;
				return rv;
			}
			oa_soap_bay_pwr_status[bay_number -1] = SAHPI_POWER_OFF;
                        break;

                case (POWER_ON):
			oa_soap_bay_pwr_status[bay_number -1] = SAHPI_POWER_ON;
                        rv = process_server_power_on_event(oh_handler, con,
                                                           &event, bay_number);
                        break;

                /* Currently, OA is not sending the REBOOT event*/
                case (POWER_REBOOT):
                        hotswap_state.currentHsState = SAHPI_HS_STATE_ACTIVE;
                        event.event.EventDataUnion.HotSwapEvent.
                                PreviousHotSwapState =
                                SAHPI_HS_STATE_INSERTION_PENDING;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_ACTIVE;
                        /* INSERTION_PENDING to ACTIVE state change happens due
                         * to Auto policy of server blade
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));

                        break;

                default:
                        err("Wrong power state");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * process_server_insertion_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @con:        Pointer to SOAP_CON structure
 *      @oa_event:   Pointer to the OA event structure
 *      @loc:        Location, 0 default, 1 Workaround 
 *
 * Purpose:
 *      Creates the server insertion hpi hotswap event
 *
 * Detailed Description:
 *      - The inserted server blade will not have all the
 *        information with the insertion event.
 *        Build the bare minimum inventory RDR
 *      - Raise the NOT_PRESENT to INSERTION_PENDING hotswap event
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_server_insertion_event(struct oh_handler_state *oh_handler,
                                        SOAP_CON *con,
                                        struct eventInfo *oa_event,
                                        SaHpiInt32T loc)
{
        SaErrorT rv = SA_OK;
        struct getBladeInfo info;
        struct bladeInfo response;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T bay_number;
        struct oh_event event;
        SaHpiRptEntryT rpt;
	GSList *asserted_sensors = NULL;
	char blade_name[MAX_NAME_LEN];

        if (oh_handler == NULL || con == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        update_hotswap_event(oh_handler, &event);
        bay_number = oa_event->eventData.bladeStatus.bayNumber;

        if ((oa_event->eventData.bladeStatus.powered == POWER_ON) &&
            (loc == 0)) {
                /* Usually power ON event comes after insertion complete, but 5% of the
                   time it comes first. So out of order events are processed out of order.
                   The power_on event code calls this function with loc=1 to avoid 
                   recursion */
                rv = process_server_power_event(oh_handler, con, oa_event);
                return rv;
        }

        info.bayNumber = bay_number;
        rv = soap_getBladeInfo(con, &info, &response);
        if (rv != SOAP_OK) {
                err("Get blade info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

	/* Copy the blade name from response for future processing */ 
	convert_lower_to_upper(response.name, strlen(response.name), 
			       blade_name, MAX_NAME_LEN);

        /* Build the server RPT entry */
        rv = build_inserted_server_rpt(oh_handler, &response, &rpt);
        if (rv != SA_OK) {
                err("build inserted server rpt failed");
                return rv;
        }

        /* Update resource_status structure with resource_id, serial_number,
         * and presence status
         */
        oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.server, bay_number,
                      response.serialNumber, rpt.ResourceId, RES_PRESENT);

        /* Build the server RDR */
        rv = build_server_rdr(oh_handler, con, bay_number, rpt.ResourceId,
			      blade_name);
        if (rv != SA_OK) {
                err("build inserted server RDR failed");
                /* Free the inventory info from inventory RDR */
                rv = free_inventory_info(oh_handler, rpt.ResourceId);
                if (rv != SA_OK) {
                        err("Inventory cleanup failed for resource id %d",
                             rpt.ResourceId);
                }
                oh_remove_resource(oh_handler->rptcache, rpt.ResourceId);
                /* reset resource_status structure to default values */
                oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.server, bay_number,
                              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                return rv;
        }

        rv = oa_soap_populate_event(oh_handler, rpt.ResourceId, &event,
				   &asserted_sensors);
        if (rv != SA_OK) {
                err("Populating event struct failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        event.event.EventType = SAHPI_ET_HOTSWAP;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_NOT_PRESENT;
        /* For blades that do not support managed hotswap, current hotswap state
	 * is ACTIVE
	 */
        if (!(rpt.ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
        	event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                	SAHPI_HS_STATE_ACTIVE;
	} else {
        	event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                	SAHPI_HS_STATE_INSERTION_PENDING;
	}
        /* NOT_PRESENT to INSERTION_PENDING/ACTIVE state change happened due
         * to operator action of blade insertion
         */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_OPERATOR_INIT;
        /* Raise the hotswap event for the inserted server blade */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

	/* Raise the assert sensor events */
	if (asserted_sensors)
		oa_soap_assert_sen_evt(oh_handler, &rpt, asserted_sensors);

        return SA_OK;
}

/**
 * process_server_extraction_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @oa_event:   Pointer to the OA event structure
 *
 * Purpose:
 *      Creates the server extraction hpi hotswap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_server_extraction_event(struct oh_handler_state *oh_handler,
                                         struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rv = remove_server_blade(oh_handler,
                                 oa_event->eventData.bladeStatus.bayNumber);
        if (rv != SA_OK) {
                err("Removing server blade failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * build_inserted_server_rpt
 *      @oh_handler: Pointer to openhpi handler
 *      @response:   Pointer to the bladeInfo structure
 *      @rpt:        Pointer to the rpt entry
 *
 * Purpose:
 *      Populate the server blade RPT with aid of build_server_rpt() and add
 *      hotswap state information to the returned rpt.
 *      Pushes the RPT entry to infrastructure.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY  - on malloc failure
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/

SaErrorT build_inserted_server_rpt(struct oh_handler_state *oh_handler,
                                   struct bladeInfo *response,
                                   SaHpiRptEntryT *rpt)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_hotswap_state *hotswap_state = NULL;

        if (oh_handler == NULL || response == NULL || rpt == NULL) {
                err("invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (build_server_rpt(oh_handler, response, rpt) != SA_OK) {
                err("Building Server RPT failed for an inserted blade");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        if (rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
                hotswap_state = (struct oa_soap_hotswap_state *)
                        g_malloc0(sizeof(struct oa_soap_hotswap_state));
                if (hotswap_state == NULL) {
                        err("Out of memory");
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
                /* Inserted server needs some time to stabilize.  Put the
                 * server hotswap state to INSERTION_PENDING.  Once the
                 * server stabilizes, put the hotswap state to ACTIVE
                 * (handled in power on event).
                 */
                hotswap_state->currentHsState =
                        SAHPI_HS_STATE_INSERTION_PENDING;
        }

        rv = oh_add_resource(oh_handler->rptcache, rpt, hotswap_state, 0);
        if (rv != SA_OK) {
                err("Failed to add Server rpt");
                if (hotswap_state != NULL)
                        g_free(hotswap_state);
                return rv;
        }

        return SA_OK;
}

/**
 * oa_soap_proc_server_status
 *      @oh_handler	: Pointer to openhpi handler structure
 *      @con		: Pointer to soap con
 *      @status		: Pointer to blade status structure
 *
 * Purpose:
 *      Processes the server status event
 *
 * Detailed Description: NA
 *
 * Return values:
 *	NONE
 **/
void oa_soap_proc_server_status(struct oh_handler_state *oh_handler,
				    SOAP_CON *con,
				    struct bladeStatus *status)
{
	SaErrorT rv = SA_OK;
	SaHpiRptEntryT *rpt = NULL;
	struct oa_soap_handler *oa_handler = NULL;
	SaHpiResourceIdT resource_id;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];
	struct getBladeThermalInfoArray thermal_request;
	struct bladeThermalInfoArrayResponse thermal_response;

	if (oh_handler == NULL || status == NULL) {
		err("Invalid parameters");
		return;
	}

	oa_handler = (struct oa_soap_handler *) oh_handler->data;
	resource_id = oa_handler->oa_soap_resources.server.
			resource_id[status->bayNumber - 1];
	/* Get the rpt entry of the resource */
	rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
	if (rpt == NULL) {
		err("resource RPT is NULL");
		return;
	}

	/* Process operational status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_OPER_STATUS,
				     status->operationalStatus, 0, 0)

	/* Process predictive failure sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_PRED_FAIL,
				     status->operationalStatus, 0, 0)

	/* Process internal data error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_INT_DATA_ERR,
				     status->diagnosticChecks.internalDataError,
				     0, 0)

	/* Process management processor error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_MP_ERR,
				     status->diagnosticChecks.
					managementProcessorError, 0, 0)

	/* Process thermal waring sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_THERM_WARN,
				     status->diagnosticChecks.thermalWarning,
				     0, 0)

	/* Process thermal danger sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_THERM_DANGER,
				     status->diagnosticChecks.thermalDanger,
				     0, 0)

	/* Process IO configuration error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_IO_CONFIG_ERR,
				     status->diagnosticChecks.
					ioConfigurationError, 0, 0)

	/* Process device power request error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_PWR_REQ,
				     status->diagnosticChecks.
					devicePowerRequestError, 0, 0)

	/* Process insufficient coolling sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_INSUF_COOL,
				     status->diagnosticChecks.
					insufficientCooling, 0, 0)

	/* Process device location error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_LOC_ERR,
				     status->diagnosticChecks.
					deviceLocationError, 0, 0)

	/* Process device failure error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_FAIL,
				     status->diagnosticChecks.deviceFailure,
				     0, 0)

	/* Process device degraded error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_DEGRAD,
				     status->diagnosticChecks.deviceDegraded,
				     0, 0)

	oa_soap_parse_diag_ex(status->diagnosticChecksEx, diag_ex_status);

	/* Process device missing sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_MISS,
				     diag_ex_status[DIAG_EX_DEV_MISS], 0, 0)

	/* Process device bonding sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_BOND,
				     diag_ex_status[DIAG_EX_DEV_BOND], 0, 0)

	/* Process device power sequence sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_PWR_SEQ,
				     diag_ex_status[DIAG_EX_DEV_PWR_SEQ], 0, 0)

	/* Process network configuration sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_NET_CONFIG,
				     diag_ex_status[DIAG_EX_NET_CONFIG], 0, 0)

	/* Process profile unassigned error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_PROF_UNASSIGN_ERR,
				     diag_ex_status[DIAG_EX_PROF_UNASSIGN_ERR],
				     0, 0)

	/* Process Device not supported sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_NOT_SUPPORT,
				     diag_ex_status[DIAG_EX_DEV_NOT_SUPPORT],
				     0, 0)

	/* Process Too low power request sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_TOO_LOW_PWR_REQ,
				     diag_ex_status[DIAG_EX_TOO_LOW_PWR_REQ],
				     0, 0)

	/* Process Call HP sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_CALL_HP,
				     diag_ex_status[DIAG_EX_CALL_HP], 0, 0)

	/* Process Storage device missing sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_STORAGE_DEV_MISS,
				     diag_ex_status[DIAG_EX_STORAGE_DEV_MISS],
				     0, 0)

	/* Process Power capping error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_GRPCAP_ERR,
				     diag_ex_status[DIAG_EX_GRPCAP_ERR],
				     0, 0)

	/* Process IML recorded errors sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_IML_ERR,
				     diag_ex_status[DIAG_EX_IML_ERR], 0, 0)

	/* Process Duplicate management IP address sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DUP_MGMT_IP_ADDR,
				     diag_ex_status[DIAG_EX_DUP_MGMT_IP_ADDR],
				     0, 0)

	/* Partner blades such as IO BLADE or STORAGE BLADE raises a 
	 * blade status event when it recovers from degraded state.
	 * The received event is checked for the power status of the blade.
	 * If the power status is "POWER_ON", 
	 * then the sensors associated with blade is enabled based on
	 * getBladeThermalInfoArray thermal response from the blade.
	 */

	if ((rpt->ResourceEntity.Entry[0].EntityType == SAHPI_ENT_IO_BLADE) ||
	    (rpt->ResourceEntity.Entry[0].EntityType == SAHPI_ENT_DISK_BLADE)) {

		/* Sometimes the DISK_BLADE does the power toggle upon the
		 * reset of adjacent SYSTEM_BLADE. In effect to this, blade
		 * status event is raised. Ignore the event, as the partner
		 * blade is expected to get back to POWER ON state immediately
		 */
		if (oa_soap_bay_pwr_status[rpt->ResourceEntity.Entry[0].
							EntityLocation -1] == 
							SAHPI_POWER_ON) {
			dbg("Ignore the blade status event from the partner"
			    " blade %d which is in POWER ON state",
			    status->bayNumber);
			return;
		}
				
		if (status->powered == POWER_ON) {
			dbg("The blade has deasserted degraded state,"
			    " enable thermal sensors");

			/* Make getBladeThermalInfoArray soap call */ 
			thermal_request.bayNumber = status->bayNumber;
			rv = soap_getBladeThermalInfoArray(con, 
						&thermal_request, 
						&thermal_response);

			/* In addition to verifying return value from the soap
			 * call, check whether the thermal response is NULL,
			 * partner blade resource might have transitioned to 
			 * degraded state
			 */
			if ((rv != SA_OK) ||
			    (thermal_response.bladeThermalInfoArray == NULL)) {
				err("getBladeThermalInfo failed for blade or"
				    "the blade %d is not in stable state",
				    status->bayNumber);
				return;
			}

			/* Walk through the rdr list of the resource and enable
			 * only those sensor which have the "SensorPresent"
			 * value as "true" in getBladeThermalInfoArray response.
			 * Rest of the statically modeled sensors remain in
			 *  disabled state.
			 */
			rv = oa_soap_set_thermal_sensor(oh_handler, rpt, 
							&thermal_response, 
							SAHPI_TRUE);
			if (rv != SA_OK) {
				err("Failed to enable the thermal sensor");
				return;;
			}
			
			/* Set the power status of the partner blade as 
			 * POWER ON since the partner blade has recovered from
			 * degraded state. After this event, the partner blade
			 * power status should never change in 
			 * oa_soap_bay_pwr_status array
			 */
			oa_soap_bay_pwr_status[rpt->ResourceEntity.Entry[0].
							EntityLocation -1] == 
							SAHPI_POWER_ON;
		} else if (status->powered == POWER_OFF) {
			dbg("thermal sensors of blade already in disable state,"
			    " no action required");
		}
	}
	return;
}

/**
 * oa_soap_serv_post_comp
 *      @oh_handler	: Pointer to openhpi handler structure
 *	@con		: Pointer to soap con structure
 *      @bay_number	: Bay number of the resource
 *
 * Purpose:
 *      Processes the blade post complete event
 *
 * Detailed Description: NA
 *
 * Return values:
 *	NONE
 **/
void oa_soap_serv_post_comp(struct oh_handler_state
						*oh_handler,
				      SOAP_CON *con,
				      SaHpiInt32T bay_number)
{
	SaErrorT rv = SA_OK;
	SaHpiRptEntryT *rpt = NULL;
	struct getBladeThermalInfoArray thermal_request;
	struct bladeThermalInfoArrayResponse thermal_response;
	struct oa_soap_handler *oa_handler = NULL;
	SaHpiResourceIdT resource_id = -1;
	
	if (oh_handler == NULL) {
		err("Invalid parameters");
		return;
	}

	oa_handler = (struct oa_soap_handler *) oh_handler->data;
	
	resource_id = 
		oa_handler->oa_soap_resources.server.resource_id[bay_number -1];
	
	rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
	if (rpt == NULL) {
		err("resource RPT is NULL");
		return;
	}

	/* Make getBladeThermalInfoArray soap call */ 
	thermal_request.bayNumber = bay_number;
	rv = soap_getBladeThermalInfoArray(con, &thermal_request,
					   &thermal_response);

	/* In addition to verifying return value from the soap call, 
	 * Check whether the thermal response is NULL,
	 * blade resource might have transitioned to POWER-OFF state
	 * during the processing of this event hence resulting in 
	 * a NULL response
	 */
	if ((rv != SA_OK) || (thermal_response.bladeThermalInfoArray == NULL)) {
		err("getBladeThermalInfo failed for blade or"
		    "the blade is not in stable state");
		return;
	}

	/* Walk through the rdr list of the resource and enable only those 
	 * sensor which have the "SensorPresent" value as "true" in 
	 * getBladeThermalInfoArray response. Rest of the statically modeled
	 * sensors remain in disabled state.
	 */
	rv = oa_soap_set_thermal_sensor(oh_handler, rpt, &thermal_response, 
					SAHPI_TRUE);
	if (rv != SA_OK) {
		err("Failed to enable the thermal sensor");
		return;
	}
	
	return;
}

/**
 * oa_soap_set_thermal_sensor
 *      @oh_handler	: Pointer to openhpi handler structure
 *      @rpt		: Pointer to rpt structure
 *      @thermal_response: Pointer to bladeThermalInfoArray response structure
 *	@enable_flag	: Sensor Enable Flag
 *
 * Purpose:
 *      Enables or Disables the thermal sensors associated with server blade
 *
 * Detailed Description:
 *	- For Disable request of thermal sensor, the function walks through 
 *	  the rdr list of the blade resource and disables only the thermal
 *	  sensors
 *	- Also disable the user control to enable the thermal sensor.
 *	- For Enable request of thermal sensors, following steps are done:
 *	  	1. Make soap getBladeThermalInfoArray soap call to the 
 *		blade resource.
 *		2.  Walk through the rdr list of the resource and enable
 *	  	only those thermal sensor which have the "SensorPresent" value
 *		as "true" in getBladeThermalInfoArray response.
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT oa_soap_set_thermal_sensor(struct oh_handler_state *oh_handler,
				    SaHpiRptEntryT *rpt,
				    struct bladeThermalInfoArrayResponse
					*thermal_response,
				    SaHpiBoolT enable_flag)
{
	SaErrorT rv = SA_OK;
        SaHpiRdrT *rdr = NULL;
	struct bladeThermalInfo bld_thrm_info;
	struct extraDataInfo extra_data;

	if (oh_handler == NULL || rpt == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	rdr = oh_get_rdr_next(oh_handler->rptcache, rpt->ResourceId,
			      SAHPI_FIRST_ENTRY);
	while (rdr) {
		if (rdr->RdrType == SAHPI_SENSOR_RDR) {
			if ((rdr->RdrTypeUnion.SensorRec.Num == 
					OA_SOAP_SEN_TEMP_STATUS) ||
			    ((rdr->RdrTypeUnion.SensorRec.Num >=
					OA_SOAP_BLD_THRM_SEN_START) && 
			    (rdr->RdrTypeUnion.SensorRec.Num <= 
						OA_SOAP_BLD_THRM_SEN_END))) {
				if (enable_flag == SAHPI_TRUE) {
					/* Verify with of the thermal
					 * response to enable the sensor
					 */
					if (thermal_response == NULL) {
						err ("Valid thermal response"
						     " required for processing"
						     " sensor enable operation");
						return 
						SA_ERR_HPI_INTERNAL_ERROR;
					}

					/* Fetch the mapping bladeThermalInfo 
					 * structure instance from response for
					 * for thermal sensor number to 
					 * whether the sensor can be enabled
					 */
					rv = oa_soap_get_bld_thrm_sen_data(
						rdr->RdrTypeUnion.SensorRec.Num,
						*thermal_response,
						&bld_thrm_info);

					if (rv != SA_OK) {
						err("Could not find the"
						    " matching sensor");
						return 
						SA_ERR_HPI_INTERNAL_ERROR;
					}

			                soap_getExtraData(bld_thrm_info.
								extraData,
                                                	  &extra_data);
                			if ((extra_data.value != NULL) &&
					    (!(strcasecmp(extra_data.value, 
							  "false")))) {
						dbg("sensor can not be enabled");
						rdr = oh_get_rdr_next(
							oh_handler->rptcache, 
							rpt->ResourceId, 
							rdr->RecordId);
						continue;
					}
				}
				rv = oa_soap_set_sensor_enable(oh_handler, 
						rpt->ResourceId, 
						rdr->RdrTypeUnion.SensorRec.Num,
						enable_flag);
				if (rv != SA_OK) {
					err("Sensor set failed");
					return rv;
				}
			}
		}
		rdr = oh_get_rdr_next(oh_handler->rptcache, rpt->ResourceId,
				      rdr->RecordId);
	}
	return SA_OK;
}
