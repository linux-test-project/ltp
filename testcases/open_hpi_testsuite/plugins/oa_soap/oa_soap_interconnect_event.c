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
 * This file has the interconnect blade related events handling
 *
 *      process_interconnect_reset_event()      - Process the interconeect
 *                                                reset event
 *
 *      process_interconnect_power_event()      - Process the interconeect
 *                                                power event
 *
 *      process_interconnect_insertion_event()  - Process the interconeect
 *                                                insertion event
 *
 *      process_interconnect_extraction_event() - Process the interconeect
 *                                                extraction event
 *
 *      process_interconnect_status_event()     - Process the interconeect
 *                                                status event
 *
 *      process_interconnect_thermal_event()    - Process the interconeect
 *                                                thermal event
 *
 */

#include "oa_soap_interconnect_event.h"

/**
 * process_interconnect_reset_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @oa_event:   Pointer to OA event structure
 *
 * Purpose:
 *      Creates the interconnect reset hpi hotswap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_interconnect_reset_event(struct oh_handler_state *oh_handler,
                                          struct eventInfo *oa_event)
{
        struct oa_soap_hotswap_state *hotswap_state = NULL;
        struct oh_event event;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T bay_number;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        update_hotswap_event(oh_handler, &event);

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        bay_number = oa_event->eventData.interconnectTrayStatus.bayNumber;
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
        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(oh_handler->rptcache,
                                     event.resource.ResourceId);
        if (hotswap_state == NULL) {
                err("blade private info is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        hotswap_state->currentHsState = SAHPI_HS_STATE_ACTIVE;
        event.resource.ResourceSeverity = SAHPI_OK;

        /* On reset of interconnect, it has powered off and powered on
         * Raise 2 hoswap events for power off
         * ACTIVE -> EXTRACTION_PENDING and EXTRACTION_PENDING -> INACTIVE
         * Then, raise 2 hoswap events for power on
         * INACTIVE -> INSERTION_PENDING and INSERTION_PENDING -> ACTIVE
         */
        event.rdrs = NULL;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_EXTRACTION_PENDING;
        /* ACTIVE to EXTRACTION_PENDING state change happened due power off
         *  event. The deactivation can not be stopped.
         */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_UNEXPECTED_DEACTIVATION;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        event.rdrs = NULL;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_EXTRACTION_PENDING;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_INACTIVE;
        /* EXTRACTION_PENDING to INACTIVE state change happened due
         * to Auto policy of the server blade
         */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_AUTO_POLICY;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        event.rdrs = NULL;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_INACTIVE;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_INSERTION_PENDING;
        /* The cause of the state change is unknown */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_UNKNOWN;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        event.rdrs = NULL;
        event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                SAHPI_HS_STATE_INSERTION_PENDING;
        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                SAHPI_HS_STATE_ACTIVE;
        /* INSERTION_PENDING to ACTIVE state change happened due
         * to auto policy of server blade
         */
        event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                SAHPI_HS_CAUSE_AUTO_POLICY;
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

        return SA_OK;
}

/**
 * process_interconnect_power_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @oa_event:   Pointer to OA event structure
 *
 * Purpose:
 *      Creates the interconnect power hpi hotswap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_interconnect_power_event(struct oh_handler_state *oh_handler,
                                          struct eventInfo *oa_event)
{
        struct oa_soap_hotswap_state *hotswap_state = NULL;
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt = NULL;
        struct oh_event event;
        struct oa_soap_sensor_info *sensor_info=NULL;
        SaHpiRdrT *rdr = NULL;
        SaHpiIdrIdT sen_rdr_num = OA_SOAP_RES_SEN_TEMP_NUM;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T bay_number;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        update_hotswap_event(oh_handler, &event);

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        bay_number = oa_event->eventData.interconnectTrayStatus.bayNumber;
        resource_id = oa_handler->
                oa_soap_resources.interconnect.resource_id[bay_number - 1];
        /* Get the rpt entry of the server */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;
        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(oh_handler->rptcache,
                                     event.resource.ResourceId);
        if (hotswap_state == NULL) {
                err("blade private info is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        switch (oa_event->eventData.interconnectTrayStatus.powered) {
                case (POWER_OFF):
                        event.resource.ResourceSeverity = SAHPI_CRITICAL;
                        /* Update the current hotswap state to INACTIVE */
                        hotswap_state->currentHsState = SAHPI_HS_STATE_INACTIVE;
                        if (rv != SA_OK) {
                                err("add rpt entry failed");
                                return rv;
                        }

                        /* Raise the power off hotswap event*/
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
                        /* EXTRACTION_PENDING to INACTIVE state change happens
                         * due to auto policy of server blade
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));
                        break;

                case (POWER_ON):
                        event.resource.ResourceSeverity = SAHPI_OK;
                        /* Update the current hot swap state to ACTIVE */
                        hotswap_state->currentHsState = SAHPI_HS_STATE_ACTIVE;

                        rdr = oh_get_rdr_by_type(oh_handler->rptcache,
                                                 event.resource.ResourceId,
                                                 SAHPI_SENSOR_RDR,
                                                 sen_rdr_num);

                        if (rdr == NULL) {
                                err("RDR not present");
                                return SA_ERR_HPI_NOT_PRESENT;
                        }

                        /* Get the thermal sensor information of the server */
                        sensor_info = (struct oa_soap_sensor_info*)
                                oh_get_rdr_data(oh_handler->rptcache,
                                                event.resource.ResourceId,
                                                rdr->RecordId);
                        if (sensor_info == NULL) {
                                err("No sensor data. Sensor=%s",
                                    rdr->IdString.Data);
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        /* Check for any thermal sensor state
                         * and raise the deassert events
                         */
                        rv = check_and_deassert_event(oh_handler,
                                                      event.resource.ResourceId,
                                                      rdr,
                                                      sensor_info);

                        if (rv != SA_OK) {
                                err("Deassert of sensor events failed");
                        }

                        /* Since the interconnect got powered off, there will
                         * not be any active thermal events.  Clear the thermal
                         * sensor states
                         */
                        sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
                        sensor_info->previous_state = SAHPI_ES_UNSPECIFIED;

                        /* Raise the power on hotswap event*/
                        event.rdrs = NULL;
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
                         * to Auto policy of server blade
                         */
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_AUTO_POLICY;
                        oh_evt_queue_push(oh_handler->eventq,
                                          copy_oa_soap_event(&event));
                        break;

                default:
                        err("Wrong power state %d",
                            oa_event->eventData.bladeStatus.powered);
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * process_interconnect_insertion_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @con:        Pointer to the SOAP_CON structure
 *      @oa_event:   Pointer to OA event structure
 *
 * Purpose:
 *      Creates the interconnect insertion hpi hotswap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_interconnect_insertion_event(struct oh_handler_state
                                              *oh_handler,
                                              SOAP_CON *con,
                                              struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        struct getInterconnectTrayInfo info;
        struct interconnectTrayInfo response;
        struct oh_event event;
        SaHpiInt32T bay_number;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL || oa_event == NULL || con == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        bay_number = oa_event->eventData.interconnectTrayStatus.bayNumber;
        update_hotswap_event(oh_handler, &event);

        info.bayNumber = bay_number;
        rv = soap_getInterconnectTrayInfo(con, &info, &response);
        if (rv != SOAP_OK) {
                err("Get interconnect tray info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Build the inserted interconnect RPT entry */
        rv = build_interconnect_rpt(oh_handler, con, response.name,
                                    bay_number, &resource_id, TRUE);
        if (rv != SA_OK) {
                err("Failed to build the interconnect RPT");
                return rv;
        }

        /* Update resource_status structure with resource_id,
         * serial_number, and presence status
         */
        oa_soap_update_resource_status(
                 &oa_handler->oa_soap_resources.interconnect, bay_number,
                 response.serialNumber, resource_id, RES_PRESENT);

        /* Build the inserted interconnect RDRs */
        rv = build_interconnect_rdr(oh_handler, con,
                                    bay_number, resource_id);
        if (rv != SA_OK) {
                err("Failed to build the interconnect RDR");
                rv = oh_remove_resource(oh_handler->rptcache,
                                        event.resource.ResourceId);
                /* reset resource_status structure to default values */
                oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.interconnect, bay_number,
                      "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                return rv;
        }

        rv = populate_event(oh_handler, resource_id, &event);
        if (rv != SA_OK) {
                err("Creating hotswap event failed");
                return rv;
        }

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
 * process_interconnect_extraction_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @oa_event:   Pointer to OA event structure
 *
 * Purpose:
 *      Creates the interconnect extraction hpi hotswap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_interconnect_extraction_event(struct oh_handler_state
                                               *oh_handler,
                                               struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rv = remove_interconnect(oh_handler,
                 oa_event->eventData.interconnectTrayStatus.bayNumber);
        if (rv != SA_OK) {
                err("Encoding entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * process_interconnect_status_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @oa_event:   Pointer to OA event structure
 *
 * Purpose:
 *      Creates the interconnect insertion pending to active hpi hotswap event
 *
 * Detailed Description:
 *      - The interconnect takes nearly 3 seconds to power on
 *        The interconnect status event which follows the insertion event
 *        indicates the power on of interconnect
 *      - Create the interconnect insertion pending to active hpi hotswap event
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_interconnect_status_event(struct oh_handler_state *oh_handler,
                                           struct eventInfo *oa_event)
{
        struct oa_soap_hotswap_state *hotswap_state;
        SaHpiRptEntryT *rpt = NULL;
        struct oh_event event;
        SaHpiInt32T bay_number;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler = NULL;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        bay_number = oa_event->eventData.interconnectTrayStatus.bayNumber;
        update_hotswap_event(oh_handler, &event);

        if (oa_event->eventData.interconnectTrayStatus.powered != POWER_ON) {
                return SA_OK;
        }

        resource_id = oa_handler->
                oa_soap_resources.interconnect.resource_id[bay_number - 1];
        /* Get the rpt entry of the server */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;

        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(oh_handler->rptcache,
                                     event.resource.ResourceId);
        if (hotswap_state == NULL) {
                err("Failed to get hotswap state of server blade");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check whether blade is in the insertion pending state and it is
         * powered on
         */
        if (hotswap_state->currentHsState == SAHPI_HS_STATE_INSERTION_PENDING) {
                hotswap_state->currentHsState = SAHPI_HS_STATE_ACTIVE;
                event.event.Source = event.resource.ResourceId;

                event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
                        SAHPI_HS_STATE_INSERTION_PENDING;
                event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                        SAHPI_HS_STATE_ACTIVE;
                /* INSERTION_PENDING to ACTIVE state change happened de
                 * to Auto policy of server blade
                 */
                event.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
                        SAHPI_HS_CAUSE_AUTO_POLICY;
                oh_evt_queue_push(oh_handler->eventq,
                                  copy_oa_soap_event(&event));
        }

        return SA_OK;
}

/**
 * process_interconnect_thermal_event
 *      @oh_handler:  Pointer to openhpi handler structure
 *      @oa_event:    Pointer to the OA event structure
 *
 * Purpose:
 *      Processes and creates interconnect sensor thermal events
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_interconnect_thermal_event(struct oh_handler_state *oh_handler,
                                            struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;
        SaHpiSensorReadingT current_reading;
        SaHpiSeverityT event_severity;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T bay_number;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        bay_number = oa_event->eventData.interconnectTrayStatus.bayNumber;
        resource_id = oa_handler->
                oa_soap_resources.interconnect.resource_id[bay_number - 1];
        /* Get the rpt entry of the server */
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

        current_reading.IsSupported = SAHPI_FALSE;

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
                        /* Sensor has changed the state to Caution from Major
                         * or OK.  Update the sensor structure.
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
                                        oh_handler,
                                        OA_SOAP_RES_SEN_TEMP_NUM,
                                        rpt, rdr, current_reading,
                                        event_severity, sensor_info);
                                if (rv != SA_OK) {
                                        err("Raising assert thermal event "
                                            "failed");
                                        return rv;
                                }
                        } else {
                                event_severity = SAHPI_CRITICAL;
                                /* The previous sensor state is Major
                                 * Raise the thermal sensor deassert event
                                 */
                                rv = generate_sensor_deassert_thermal_event(
                                        oh_handler,
                                        OA_SOAP_RES_SEN_TEMP_NUM,
                                        rpt, rdr, current_reading,
                                        event_severity, sensor_info);
                                if (rv != SA_OK) {
                                        err("Raising assert thermal event "
                                            "failed");
                                        return rv;
                                }
                        }
                        break;
                case SENSOR_STATUS_CRITICAL:
                        /* Sensor has changed the state from Caution to Critical
                         * Update the sensor structure
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
                        err("Sensor status not handled");
                        return SA_OK;
        }
        return SA_OK;
}

