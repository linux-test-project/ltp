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
 *      process_server_thermal_event()    - Processes the server thermal event
 *
 *      build_inserted_server_rpt()       - Builds the rpt entry for inserted
 *                                          server
 *
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
        SaHpiIdrIdT sen_rdr_num = OA_SOAP_RES_SEN_TEMP_NUM;

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

                        /* Check for any thermal sensor state
                         * and raise the deassert events
                         */
                        rv = check_and_deassert_event(
                                oh_handler, event->resource.ResourceId,
                                rdr, sensor_info);

                        if (rv != SA_OK) {
                                err("Deassert of sensor events failed");
                        }

                        /* Since the server got powered off, there will not be
                         * any active thermal events
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
        SaHpiInt32T bay_number;
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
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;

        switch (oa_event->eventData.bladeStatus.powered) {
                case (POWER_OFF):
                        rv = process_server_power_off_event(oh_handler, &event);
                        break;

                case (POWER_ON):
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
                                        struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;
        struct getBladeInfo info;
        struct bladeInfo response;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T bay_number;
        struct oh_event event;
        SaHpiRptEntryT rpt;

        if (oh_handler == NULL || con == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        update_hotswap_event(oh_handler, &event);
        bay_number = oa_event->eventData.bladeStatus.bayNumber;

        info.bayNumber = bay_number;
        rv = soap_getBladeInfo(con, &info, &response);
        if (rv != SOAP_OK) {
                err("Get blade info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

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
        rv = build_server_rdr(oh_handler, con, bay_number, rpt.ResourceId);
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

        rv = populate_event(oh_handler, rpt.ResourceId, &event);
        if (rv != SA_OK) {
                err("Populating event struct failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
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
        struct getBladeInfo request;
        struct bladeInfo response;
        struct oa_soap_handler *oa_handler = NULL;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        request.bayNumber = oa_event->eventData.bladeStatus.bayNumber;
        rv = soap_getBladeInfo(oa_handler->active_con, &request, &response);
        if (rv != SOAP_OK) {
            err("Get blade info failed");
            return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rv = remove_server_blade(oh_handler,
                                 oa_event->eventData.bladeStatus.bayNumber,
                                 response.bladeType);
        if (rv != SA_OK) {
                err("Removing server blade failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * process_server_thermal_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @oa_event:   Pointer to the OA event structure
 *
 * Purpose:
 *      Processes and creates server sensor thermal events
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_server_thermal_event(struct oh_handler_state *oh_handler,
                                      struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;
        SaHpiSensorReadingT current_reading;
        SaHpiSeverityT event_severity = SAHPI_OK;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T bay_number;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        bay_number = oa_event->eventData.bladeStatus.bayNumber;
        resource_id =
           oa_handler->oa_soap_resources.server.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rdr = oh_get_rdr_by_type(oh_handler->rptcache,
                                 rpt->ResourceId,
                                 SAHPI_SENSOR_RDR,
                                 OA_SOAP_RES_SEN_TEMP_NUM);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Get the thermal sensor information */
        sensor_info = (struct oa_soap_sensor_info*)
                oh_get_rdr_data(oh_handler->rptcache, rpt->ResourceId,
                                rdr->RecordId);
        if (sensor_info == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* If the thermal sensor is enabled and sensor event is enabled
         * Then, raise the thermal sensor events can be raised,
         * Else, ignore the thermal event from OA
         */
        if (sensor_info->sensor_enable == SAHPI_FALSE) {
                dbg("Sensor not enabled for reading");
                return SA_OK;
        }

        if (sensor_info->event_enable == SAHPI_FALSE) {
                dbg("Sensor not enabled for raising events");
                return SA_OK;
        }

        current_reading.IsSupported = SAHPI_TRUE;
        current_reading.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
        /* Put the current thermal temperature into current reading*/
        current_reading.Value.SensorFloat64 =
                oa_event->eventData.thermalInfo.temperatureC;

        /* Check the current state of the sensor */
        switch (oa_event->eventData.thermalInfo.sensorStatus) {
                case  SENSOR_STATUS_OK:
                        /* Sensor has changed the state from Major to OK
                         * Update the sensor structure
                         */
                        sensor_info->previous_state = SAHPI_ES_UPPER_MAJOR;
                        sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
                        event_severity = SAHPI_MAJOR;

                        /* Raise the thermal sensor deassert event */
                        rv = generate_sensor_deassert_thermal_event(oh_handler,
                                                      OA_SOAP_RES_SEN_TEMP_NUM,
                                                      rpt,
                                                      rdr,
                                                      current_reading,
                                                      event_severity,
                                                      sensor_info);
                        if (rv != SA_OK) {
                                err("Raising deassert thermal event failed");
                                return rv;
                        }
                        break;
                case SENSOR_STATUS_CAUTION:
                        /* Sensor has changed the state to Caution from
                         * Major or OK.  Update the sensor structure.
                         */
                        sensor_info->previous_state =
                                sensor_info->current_state;
                        sensor_info->current_state = SAHPI_ES_UPPER_MAJOR;

                        if (sensor_info->previous_state ==
                            SAHPI_ES_UNSPECIFIED) {
                                event_severity = SAHPI_MAJOR;
                                /* The previous sensor state is OK
                                 * Raise the thermal sensor assert event
                                 */
                                rv = generate_sensor_assert_thermal_event(
                                        oh_handler, OA_SOAP_RES_SEN_TEMP_NUM,
                                        rpt, rdr, current_reading,
                                        event_severity, sensor_info);
                                if (rv != SA_OK) {
                                        err("Raising assert thermal "
                                            "event failed");
                                        return rv;
                                }
                        } else {
                                event_severity = SAHPI_CRITICAL;
                                /* The previous sensor state is Major
                                 * Raise the thermal sensor deassert event
                                 */
                                rv = generate_sensor_deassert_thermal_event(
                                        oh_handler, OA_SOAP_RES_SEN_TEMP_NUM,
                                        rpt, rdr, current_reading,
                                        event_severity, sensor_info);
                                if (rv != SA_OK) {
                                        err("Raising assert thermal "
                                            "event failed");
                                        return rv;
                                }
                        }
                        break;
                case SENSOR_STATUS_CRITICAL:
                        /* Sensor has changed the state from Caution to
                         * Critical.  Update the sensor structure.
                         */
                        sensor_info->previous_state = SAHPI_ES_UPPER_MAJOR;
                        sensor_info->current_state = SAHPI_ES_UPPER_CRIT;
                        event_severity = SAHPI_CRITICAL;

                        /* Raise the thermal sensor assert event */
                        rv = generate_sensor_assert_thermal_event(oh_handler,
                                                    OA_SOAP_RES_SEN_TEMP_NUM,
                                                    rpt,
                                                    rdr,
                                                    current_reading,
                                                    event_severity,
                                                    sensor_info);
                        if (rv != SA_OK) {
                                err("Raising assert thermal event failed");
                                return rv;
                        }
                        break;
                default:
                        err("sensor status not handled");
                        return SA_OK;
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
