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
 *      Raghavendra M.S.  <raghavendra.ms@hp.com>
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
        SaHpiIdrIdT sen_rdr_num = OA_SOAP_SEN_TEMP_STATUS;
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
	GSList *asserted_sensors = NULL;
	SaHpiRptEntryT *rpt;

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

        rv = oa_soap_populate_event(oh_handler, resource_id, &event,
				    &asserted_sensors);
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

	/* Raise the assert sensor events */
	if (asserted_sensors) {
	        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
		oa_soap_assert_sen_evt(oh_handler, rpt, asserted_sensors);
	}

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
 *      Processes the interconnect status event and generates the sensor event
 *
 * Detailed Description:
 *      - The interconnect takes nearly 3 seconds to power on
 *        The interconnect status event which follows the insertion event
 *        indicates the power on of interconnect
 *      - Create the interconnect insertion pending to active hpi hotswap event
 *      - Processes the interconnect status event and generates the sensor event
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
void oa_soap_proc_interconnect_status(struct oh_handler_state *oh_handler,
				      struct interconnectTrayStatus *status)
{
        struct oa_soap_hotswap_state *hotswap_state;
        SaHpiRptEntryT *rpt = NULL;
        struct oh_event event;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler = NULL;
	enum oa_soap_extra_data_health health_status;
	SaErrorT rv;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

        if (oh_handler == NULL || status == NULL) {
                err("Invalid parameters");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        resource_id = oa_handler->oa_soap_resources.interconnect.
			resource_id[status->bayNumber - 1];
        /* Get the rpt entry of the server */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return;
        }

        hotswap_state = (struct oa_soap_hotswap_state *)
                oh_get_resource_data(oh_handler->rptcache, resource_id);
        if (hotswap_state == NULL) {
                err("Failed to get hotswap state of server blade");
                return;
        }

        /* Check whether blade is in the insertion pending state and it is
         * powered on
         */
        if (hotswap_state->currentHsState == SAHPI_HS_STATE_INSERTION_PENDING &&
	    status->powered == POWER_ON) {
                hotswap_state->currentHsState = SAHPI_HS_STATE_ACTIVE;

		update_hotswap_event(oh_handler, &event);
		memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
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

	/* Build operational status sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_OPER_STATUS,
				     status->operationalStatus, 0, 0)

	/* Build predictive failure sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_PRED_FAIL,
				     status->operationalStatus, 0, 0)

	/* Build interconnect CPU fault sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_CPU_FAULT,
				     status->cpuFault, 0, 0)

	/* Build interconnect health LED sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_HEALTH_LED,
				     status->healthLed, 0, 0)

	/* Build internal data error sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_INT_DATA_ERR,
				     status->diagnosticChecks.internalDataError,
				     0, 0)

	/* Build management processor error sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_MP_ERR,
				     status->diagnosticChecks.
					managementProcessorError, 0, 0)

	/* Build thermal waring sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_THERM_WARN,
				     status->diagnosticChecks.thermalWarning,
				     0, 0)

	/* Build thermal danger sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_THERM_DANGER,
				     status->diagnosticChecks.thermalDanger,
				     0, 0)

	/* Build IO configuration error sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_IO_CONFIG_ERR,
				     status->diagnosticChecks.
					ioConfigurationError, 0, 0)

	/* Build device power request error sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_PWR_REQ,
				     status->diagnosticChecks.
					devicePowerRequestError, 0, 0)

	/* Build device failure error sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_FAIL,
				     status->diagnosticChecks.deviceFailure,
				     0, 0)

	/* Build device degraded error sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_DEGRAD,
				     status->diagnosticChecks.deviceDegraded,
				     0, 0)

	oa_soap_parse_diag_ex(status->diagnosticChecksEx, diag_ex_status);

	/* Process device not supported sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_NOT_SUPPORT,
				     diag_ex_status[DIAG_EX_DEV_NOT_SUPPORT],
				     0, 0)

	/* Process Device informational sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_INFO,
				     diag_ex_status[DIAG_EX_DEV_INFO], 0, 0)

	/* Process Storage device missing sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_STORAGE_DEV_MISS,
				     diag_ex_status[DIAG_EX_STORAGE_DEV_MISS],
				     0, 0)

	/* Process Duplicate management IP address sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DUP_MGMT_IP_ADDR,
				     diag_ex_status[DIAG_EX_DUP_MGMT_IP_ADDR],
				     0, 0)

	/* Get the healthStatus enum from extraData structure */
	oa_soap_get_health_val(status->extraData, &health_status);

	/* Build health status operational sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_HEALTH_OPER,
				     health_status, 0, 0)

	/* Build health status predictive failure sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_HEALTH_PRED_FAIL,
				     health_status, 0, 0)
        return;
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
void oa_soap_proc_interconnect_thermal(struct oh_handler_state *oh_handler,
				       SOAP_CON *con,
				       struct interconnectTrayStatus *response)
{
        SaErrorT rv = SA_OK;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T bay_number;
	SaHpiFloat64T trigger_reading;
	SaHpiFloat64T trigger_threshold;
	struct getThermalInfo thermal_request;
	struct thermalInfo thermal_response;
	struct oa_soap_sensor_info *sensor_info;
	SaHpiRdrT *rdr;

        if (oh_handler == NULL || con== NULL || response == NULL) {
                err("Invalid parameters");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        bay_number = response->bayNumber;
        resource_id = oa_handler->
                oa_soap_resources.interconnect.resource_id[bay_number - 1];

	rdr = oh_get_rdr_by_type(oh_handler->rptcache, resource_id,
				 SAHPI_SENSOR_RDR, OA_SOAP_SEN_TEMP_STATUS);
	sensor_info = (struct oa_soap_sensor_info *)
			oh_get_rdr_data(oh_handler->rptcache, resource_id,
					rdr->RecordId);

        /* The payload of the thermal event received from OA contains
	 * sensor status. Based on the sensor status, 
	 * determine the threshold which triggered the thermal event from OA.
         * Event with SENSOR_STATUS_CAUTION or SENSOR_STATUS_OK is
         * generated only if CAUTION threshold is crossed.
         * Event with SENSOR_STATUS_CRITICAL is generated only when CRITICAL
         * threshold is crossed.
         * Sensor current reading and trigger threshold are required for event
         * generation. Sensor current reading is not provided by the event,
         * hence make soap call to get the reading
         */
	thermal_request.bayNumber = bay_number;
	thermal_request.sensorType = SENSOR_TYPE_INTERCONNECT;

	rv = soap_getThermalInfo(con, &thermal_request, &thermal_response);
	if (rv != SOAP_OK) {
		err("soap_getThermalInfo soap call returns error");
		return;
	}

        trigger_reading = (SaHpiInt32T)thermal_response.temperatureC;

        if ((response->thermal == SENSOR_STATUS_CAUTION &&
	     sensor_info->current_state != SAHPI_ES_UPPER_MAJOR) || 
	    (response->thermal == SENSOR_STATUS_OK &&
	      sensor_info->current_state !=  SAHPI_ES_UNSPECIFIED)) {
		/* Trigger for this event is caution threshold */
                trigger_threshold = thermal_response.cautionThreshold;
        } else if (response->thermal == SENSOR_STATUS_CRITICAL  &&
		   sensor_info->current_state != SAHPI_ES_UPPER_CRIT) {
		/* Trigger for this event is critical threshold */
                trigger_threshold = thermal_response.criticalThreshold;
        } else {
		dbg("Ignore the event. There is no change in the sensor state");
		return;
	}

        /* Process the thermal event from OA and generate appropriate HPI event
         */
        OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_TEMP_STATUS,
                                response->thermal,
                                trigger_reading,trigger_threshold)

        return;
}
