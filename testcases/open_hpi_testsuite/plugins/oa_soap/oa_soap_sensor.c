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
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *
 * This file supports the functions related to HPI Sensor.
 * The file covers three general classes of function: Sensor ABI functions,
 * Build functions for creating sensor RDRs for resources and sensor specific
 * functions for generating sensor enable and thermal events
 *
 * Sensor ABI functions:
 *
 *      oa_soap_get_sensor_reading()    - Gets the sensor reading of resource
 *
 *      oa_soap_get_sensor_thresholds() - Retreives sensor's threshold values,
 *                                        if defined
 *
 *      oa_soap_get_sensor_enable()     - Retrieves sensor's boolean enablement
 *                                        status
 *
 *      oa_soap_set_sensor_enable()     - Sets sensor's boolean enablement
 *                                        status
 *
 *      oa_soap_get_sensor_event_enable()- Retrieves sensor's boolean event
 *                                         enablement status
 *
 *      oa_soap_set_sensor_event_enable()- Sets sensor's boolean event
 *                                         enablement status
 *
 *      oa_soap_get_sensor_event_masks()- Retrieves sensor's assert and
 *                                        deassert event masks
 *
 *      oa_soap_set_sensor_event_masks()- Sets sensor's assert and deassert
 *                                        event masks
 * Build functions:
 *      update_sensor_rdr()             - Returns current status of the sensor
 *                                        RDR from resource
 *
 *      generate_sensor_assert_thermal_event()-  Builds and generates the sensor
 *                                               assert thermal event
 *
 *      generate_sensor_deassert_thermal_event()- Builds and generates the
 *                                                sensor deassert thermal event
 *
 *      check_and_deassert_event()      - Checks and deasserts the pending
 *                                        events on resource
 *
 *	oa_soap_build_sen_rdr()		- Builds the sensor
 *
 *	oa_soap_map_sen_value()		- Maps the OA sensor value to HPI sensor
 *					  value
 *
 *	oa_soap_gen_sen_evt()		- Generates the HPI sensor event
 *
 *	oa_soap_gen_res_evt() 		- Generates the HPI resource event
 *
 *	oa_soap_proc_sen_evt()		- Processes the sensor event
 *
 * 	oa_soap_map_thresh_resp()	- Maps the OA threshold sensor value to
 * 					  HPI sensor states
 *
 * 	oa_soap_assert_sen_evt()	- Generates the assert sensor event
 *
 *	oa_soap_get_bld_thrm_sen_data	- Retrieves the matching 
 * 					  bladeThermalInfo structure instance
 *					  from bladeThermalInfoArrayResponse
 *					  response
 */

#include "oa_soap_sensor.h"
#include "oa_soap_resources.h"

/* Forward declarations of static functions */
static SaErrorT oa_soap_gen_sen_evt(struct oh_handler_state *oh_handler,
				    SaHpiRptEntryT *rpt,
				    SaHpiRdrT *rdr,
				    SaHpiInt32T sensor_status,
				    SaHpiFloat64T trigger_reading,
				    SaHpiFloat64T trigger_threshold);

static void oa_soap_gen_res_evt(struct oh_handler_state *oh_handler,
				SaHpiRptEntryT *rpt,
				SaHpiInt32T sensor_status);

/**
 * oa_soap_get_sensor_reading
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Sensor rdr number
 *      @data: Structure for receiving sensor reading values
 *      @state: Structure for receiving sensor event states
 *
 * Purpose:
 *      Gets the sensor reading
 *
 * Detailed Description:
 *      - Fetches sensor info structure from the private data area of
 *        sensor rdr of specified rdr number
 *      - If the sensor is enabled for reading, then current reading is
 *        retrieved from the resource directly through soap call
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR
 *      SA_ERR_HPI_NOT_PRESENT - RDR is not present
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_INVALID_REQUEST - Resource has the sensor disabled
 *      SA_ERR_HPI_UNKNOWN - Invalid entity type.
 **/
SaErrorT oa_soap_get_sensor_reading(void *oh_handler,
                                    SaHpiResourceIdT resource_id,
                                    SaHpiSensorNumT rdr_num,
                                    SaHpiSensorReadingT *data,
                                    SaHpiEventStateT    *state)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler = NULL;
        struct oa_soap_handler *oa_handler = NULL;
        struct oa_soap_sensor_reading_data sensor_data;
        struct oa_soap_sensor_info *sensor_info = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;

        if (oh_handler == NULL || state == NULL || data == NULL) {
                err("Invalid paramters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;
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

        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Retrieve sensor rdr from framework of specified rdr number */
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_SENSOR_RDR,
                                 rdr_num);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Retrieve sensor_info structure from the private area of rdr */
        sensor_info = (struct oa_soap_sensor_info*)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (sensor_info == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check whether sensor is enabled */
        if (sensor_info->sensor_enable == SAHPI_FALSE) {
                err("Sensor not enabled");
                return(SA_ERR_HPI_INVALID_REQUEST);
        }

	/* Check whether the reading is supported or not */
	if (rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported ==
	    SAHPI_FALSE) {
		data->IsSupported = SAHPI_FALSE;
		*state = sensor_info->current_state;
		dbg("sensor reading is not supported");
		return SA_OK;
	}

        /* Fetch current reading of the sensor from the resource */
        rv = update_sensor_rdr(handler, resource_id,
                               rdr_num, rpt, &sensor_data);
        if (rv != SA_OK) {
                return rv;
        }

        /* Populate the return data with current sensor reading */
        data->IsSupported = sensor_data.data.IsSupported;
        data->Type = sensor_data.data.Type;
        data->Value = sensor_data.data.Value;
        *state = sensor_info->current_state;
        return SA_OK;
}

/**
 * oa_soap_get_sensor_thresholds
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Sensor rdr number
 *      @threshold: Location to store sensor's threshold values
 *
 * Purpose:
 *      Retrieves sensor's threshold values, if defined
 *
 * Detailed Description:
 *      - Fetches sensor info structure from the private data area of
 *        sensor rdr of specified rdr number
 *      - Threshold details are returned if the event category of the sensor
 *        is set to threshold type
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR
 *      SA_ERR_HPI_NOT_PRESENT - RDR is not present
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 *      SA_ERR_HPI_INVALID_CMD - Sensor not of threshold type, or is not enabled
 *                               for reading
 **/
SaErrorT oa_soap_get_sensor_thresholds(void *oh_handler,
                                      SaHpiResourceIdT resource_id,
                                      SaHpiSensorNumT rdr_num,
                                      SaHpiSensorThresholdsT *threshold)
{
        struct oh_handler_state *handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;

        if (oh_handler == NULL || threshold == NULL) {
                err("Invalid paramters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Retrieve sensor rdr from framework of specified rdr number */
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_SENSOR_RDR,
                                 rdr_num);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Retrieve sensor_info structure from the private area of rdr */
        sensor_info = (struct oa_soap_sensor_info*)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (sensor_info == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Sensor supporting threshold shall have their event category set to
         * threshold type. Threshold of a sensor is fetched only if the
         * sensor event category value = SAHPI_EC_THRESHOLD
         */
        if (rdr->RdrTypeUnion.SensorRec.Category != SAHPI_EC_THRESHOLD ||
            rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible ==
                SAHPI_FALSE ||
            rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold == 0) {
                err("Invalid command");
                return SA_ERR_HPI_INVALID_CMD;
        }

        /* setting the return value with the threshold value from the
         * sensor info strucutre
         */
        *threshold = sensor_info->threshold;
        return SA_OK;
}

/**
 * oa_soap_set_sensor_thresholds
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Sensor rdr number
 *      @threshold: Location to store sensor's threshold values
 *
 * Purpose:
 *      Sets sensor's threshold values
 *
 * Detailed Description:
 *      - The threshold values supported by HP BladeSystem cClass for different
 *        resource such as thermal limits, fan speed limits are not enabled for
 *        modifications
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - oa_soap plugin does not support this API
 **/
SaErrorT oa_soap_set_sensor_thresholds(void *oh_handler,
                                      SaHpiResourceIdT resource_id,
                                      SaHpiSensorNumT rdr_num,
                                      const SaHpiSensorThresholdsT *threshold)
{
        err("oa_soap set sensor thresholds not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_get_sensor_enable
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Sensor rdr number
 *      @enable: Location to store sensor's enablement boolean
 *
 * Purpose:
 *      Retrieves a sensor's boolean enablement status
 *
 * Detailed Description:
 *
 *      - Fetches sensor info structure from the private data area of
 *        sensor rdr of specified rdr number
 *      - Sensor enable status is returned from sensor info structure
 *        Sensor enable status determines whether the sensor is enabled for
 *        reading is retrieved from the sensor info structure
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR
 *      SA_ERR_HPI_NOT_PRESENT - RDR is not present
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT oa_soap_get_sensor_enable(void *oh_handler,
                                  SaHpiResourceIdT resource_id,
                                  SaHpiSensorNumT rdr_num,
                                  SaHpiBoolT *enable)
{
        struct oh_handler_state *handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;

        if (oh_handler == NULL || enable == NULL) {
                err("Invalid paramters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Retrieve sensor rdr from framework of specified rdr number */
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_SENSOR_RDR,
                                 rdr_num);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Retrieve sensor_info structure from the private area of rdr */
        sensor_info = (struct oa_soap_sensor_info*)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (sensor_info == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* setting the return value with the sensor enable status
         * from the sensor info strucutre
         */
        *enable = sensor_info->sensor_enable;
        return SA_OK;
}

/**
 * oa_soap_set_sensor_enable
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Sensor rdr number
 *      @enable: Location to store sensor's enablement boolean
 *
 * Purpose:
 *      Sets a sensor's boolean enablement status
 *
 * Detailed Description:
 *      - Fetches sensor info structure from the private data area of
 *        sensor rdr of specified rdr number
 *      - Sensor enable status in sensor info structure is updated
 *        with enable parameter value if it is different.
 *        Sensor enable status determines whether the sensor is enabled for
 *        reading
 *      - If there is a change in sensor enable status value, then
 *        "Sensor enable change" event is generated to report the change to
 *        framework
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR
 *      SA_ERR_HPI_NOT_PRESENT - RDR is not present
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_INVALID_STATE - The blade is in invalid state
 **/
SaErrorT oa_soap_set_sensor_enable(void *oh_handler,
                                  SaHpiResourceIdT resource_id,
                                  SaHpiSensorNumT rdr_num,
                                  SaHpiBoolT enable)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;

        if (oh_handler == NULL) {
                err("Invalid paramters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Retrieve sensor rdr from framework of specified rdr number */
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_SENSOR_RDR,
                                 rdr_num);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (rdr->RdrTypeUnion.SensorRec.EnableCtrl == SAHPI_TRUE) {
                /* Retrieve sensor_info structure from the private area
                 * of rdr
                 */
                sensor_info = (struct oa_soap_sensor_info*)
                        oh_get_rdr_data(handler->rptcache, resource_id,
                                        rdr->RecordId);
                if (sensor_info == NULL) {
                        err("No sensor data. Sensor=%s", rdr->IdString.Data);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

		/* The thermal sensors of the blade resource should not be 
		 * enabled if the resource is in power off/degraded state.
		 * TODO: Since the "EnableCtrl" field in rdr cannot be changed
		 * after the discovery of the blade, provided below is
		 * workaround logic until OpenHPI migrates to HPI-B.03.01 
		 * specification
		 */
		if ((rdr->Entity.Entry[0].EntityType == 
						SAHPI_ENT_SYSTEM_BLADE) ||
		    (rdr->Entity.Entry[0].EntityType ==
						SAHPI_ENT_IO_BLADE) ||
		    (rdr->Entity.Entry[0].EntityType ==
						SAHPI_ENT_DISK_BLADE)) {
			if ((rdr_num == OA_SOAP_SEN_TEMP_STATUS) || 
			    ((rdr_num >= OA_SOAP_BLD_THRM_SEN_START) && 
			    (rdr_num <= OA_SOAP_BLD_THRM_SEN_END))) {
				if (oa_soap_bay_pwr_status[
					rpt->ResourceEntity.Entry[0].
						EntityLocation -1] != 
							SAHPI_POWER_ON) {
					err("Sensor enable operation cannot"
					    " be performed");
					return SA_ERR_HPI_INVALID_STATE;
				}
			}
		}

                if (sensor_info->sensor_enable != enable) {
                        /* Update the sensor enable status with new value and
                         * report the change to the framework through the
                         * sensor enable event
                         */
                        sensor_info->sensor_enable = enable;
                        rv =  generate_sensor_enable_event(oh_handler, rdr_num,
                                                           rpt, rdr,
                                                           sensor_info);
                        if (rv != SA_OK) {
                                err("Event generation failed");
                                return rv;
                        }
                }
        } else {
                err("Sensor does not support changing the enable status");
                return SA_ERR_HPI_READ_ONLY;
        }
        return SA_OK;
}

/**
 * oa_soap_get_sensor_event_enable
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Sensor rdr number
 *      @enable: Location to store sensor's enablement boolean
 *
 * Purpose:
 *      Retrieves a sensor's boolean event enablement status
 *
 * Detailed Description:
 *      - Fetches sensor info structure from the private data area of
 *        sensor rdr of specified rdr number
 *      - Sensor event enable status is returned from sensor info structure.
 *        Sensor event enable status determines whether the sensor is enabled
 *        for raising events is retrieved from sensor info structure
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR
 *      SA_ERR_HPI_NOT_PRESENT - RDR is not present
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 **/
SaErrorT oa_soap_get_sensor_event_enable(void *oh_handler,
                                         SaHpiResourceIdT resource_id,
                                         SaHpiSensorNumT rdr_num,
                                         SaHpiBoolT *enable)
{
        struct oh_handler_state *handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;

        if (oh_handler == NULL || enable == NULL) {
                err("Invalid paramters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Retrieve sensor rdr from framework of specified rdr number */
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_SENSOR_RDR,
                                 rdr_num);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Retrieve sensor_info structure from the private area of rdr */
        sensor_info = (struct oa_soap_sensor_info*)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (sensor_info == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        *enable = sensor_info->event_enable;
        return SA_OK;
}

/**
 * oa_soap_set_sensor_event_enable
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Sensor rdr number
 *      @enable: Location to store sensor's enablement boolean
 *
 * Purpose:
 *      Sets a sensor's boolean event enablement status
 *
 * Detailed Description:
 *      - Fetches sensor info structure from the private data area of
 *        sensor rdr of specified rdr number
 *      - Sensor event enable status in sensor info structure is updated
 *        with enable parameter value if it is different
 *      - Sensor event enable status determines whether the sensor is enabled
 *        for raising events
 *      - If there is a change in sensor event enable status value, then
 *        "Sensor enable change" event is generated to report the change to
 *        framework
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR
 *      SA_ERR_HPI_NOT_PRESENT - RDR is not present
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 **/
SaErrorT oa_soap_set_sensor_event_enable(void *oh_handler,
                                         SaHpiResourceIdT resource_id,
                                         SaHpiSensorNumT rdr_num,
                                         const SaHpiBoolT enable)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;

        if (oh_handler == NULL) {
                err("Invalid paramters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Retrieve sensor rdr from framework of specified rdr number */
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_SENSOR_RDR,
                                 rdr_num);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (rdr->RdrTypeUnion.SensorRec.EventCtrl == SAHPI_SEC_READ_ONLY) {
                err("Sensor does not support changing the event enable status");
                return SA_ERR_HPI_READ_ONLY;
        }

        /* Retrieve sensor_info structure from the private area of rdr */
        sensor_info = (struct oa_soap_sensor_info*)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (sensor_info == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        if (sensor_info->event_enable != enable) {
                /* Update the sensor event enable status with new value
                 * and report the change to the framework through the
                 * sensor enable event
                 */
                sensor_info->event_enable = enable;
                rv =  generate_sensor_enable_event(oh_handler, rdr_num,
                                                   rpt, rdr, sensor_info);
                if (rv != SA_OK) {
                        err("Event generation failed");
                        return rv;
                }
        }
        return SA_OK;
}

/**
 * oa_soap_get_sensor_event_masks
 *      @oh_handler: Handler data pointer.
 *      @resource_id: Resource id
 *      @sid: Sensor rdr number
 *      @assert: Location to store sensor's assert event mask.
 *      @deassert: Location to store sensor's deassert event mask.
 *
 * Purpose:
 *      Retrieves a sensor's assert and deassert event masks.
 *
 * Detailed Description:
 *      - Fetches sensor info structure from the private data area of
 *        sensor rdr of specified rdr number
 *      - Sensor event mask value is returned from sensor info structure.
 *        sensor event mask determines whether the sensor is enabled for
 *        raising events if the threshold values are crossed.
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR
 *      SA_ERR_HPI_NOT_PRESENT - RDR is not present
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 **/
SaErrorT oa_soap_get_sensor_event_masks(void *oh_handler,
                                       SaHpiResourceIdT resource_id,
                                       SaHpiSensorNumT rdr_num,
                                       SaHpiEventStateT *assert,
                                       SaHpiEventStateT *deassert)
{
        struct oh_handler_state *handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;

        if (oh_handler == NULL || assert == NULL || deassert == NULL) {
                err("Invalid paramters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Retrieve sensor rdr from framework of specified rdr number */
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_SENSOR_RDR,
                                 rdr_num);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }
        /* Retrieve sensor_info structure from the private area of rdr */
        sensor_info = (struct oa_soap_sensor_info*)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (sensor_info == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        *assert = sensor_info->assert_mask;

        if (rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS) {
                *deassert = sensor_info->assert_mask;
        } else {
                *deassert = sensor_info->deassert_mask;
        }
        return SA_OK;
}

/**
 * oa_soap_set_sensor_event_masks
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource id
 *      @sid: Sensor rdr number
 *      @assert: Location to store sensor's assert event mask
 *      @deassert: Location to store sensor's deassert event mask
 *
 * Purpose:
 *      Sets a sensor's assert and deassert event masks
 *
 * Detailed Description:
 *      - Fetches sensor info structure from the private data area of
 *        sensor rdr of specified rdr number
 *      - Sensor event mask value in sensor info structure is updated
 *        with assert/deassert parameter values if it is different
 *      - Sensor event mask determines whether the sensor is enabled for
 *        raising events if the threshold values are crossed
 *      - If there is a change in sensor event mask values, then
 *        "Sensor enable change" event is generated to report the change to
 *        framework
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR
 *      SA_ERR_HPI_NOT_PRESENT - RDR is not present
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_INVALID_DATA - Invalid assert/deassert mask
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 **/
SaErrorT oa_soap_set_sensor_event_masks(void *oh_handler,
                                       SaHpiResourceIdT resource_id,
                                       SaHpiSensorNumT rdr_num,
                                       SaHpiSensorEventMaskActionT action,
                                       SaHpiEventStateT assert,
                                       SaHpiEventStateT deassert)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;
        SaHpiEventStateT orig_assert_mask = 0;
        SaHpiEventStateT orig_deassert_mask = 0;
	SaHpiEventStateT check_mask;

        if (oh_handler == NULL) {
                err("Invalid paramters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if ((assert == 0) && (deassert == 0)) {
                err("Invalid masks");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (oh_lookup_sensoreventmaskaction(action) == NULL) {
                err("Invalid action");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Retrieve sensor rdr from framework of specified rdr number */
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_SENSOR_RDR,
                                 rdr_num);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (rdr->RdrTypeUnion.SensorRec.EventCtrl != SAHPI_SEC_PER_EVENT) {
                err("Sensor do no support setting event masks");
                return SA_ERR_HPI_READ_ONLY;
        }

	/* On adding new sensors with different event category or on supporting
	 * new masks for the existing sensors, update below swtich statement
 	 */
	switch (rdr->RdrTypeUnion.SensorRec.Category) {
		case SAHPI_EC_THRESHOLD:
			check_mask = OA_SOAP_STM_VALID_MASK;
			break;
		case SAHPI_EC_PRED_FAIL:
			check_mask = SAHPI_ES_PRED_FAILURE_DEASSERT |
					SAHPI_ES_PRED_FAILURE_ASSERT;
			break;
		case SAHPI_EC_ENABLE:
			check_mask = SAHPI_ES_DISABLED | SAHPI_ES_ENABLED;
			break;
		case SAHPI_EC_REDUNDANCY:
			check_mask = SAHPI_ES_FULLY_REDUNDANT |
					SAHPI_ES_REDUNDANCY_LOST;
			break;
		default :
			err("Un-supported event category %d detected ",
			     rdr->RdrTypeUnion.SensorRec.Category);
			return SA_ERR_HPI_INTERNAL_ERROR;
	}

	if (assert !=0 && (assert & ~(check_mask))) {
		err("Assert mask is not valid");
		return SA_ERR_HPI_INVALID_DATA;
	}
	if (deassert != 0 && (deassert & ~(check_mask))) {
		err("Deassert mask is not valid");
		return SA_ERR_HPI_INVALID_DATA;
	}

	/* Retrieve sensor_info structure from the private area of rdr */
	sensor_info = (struct oa_soap_sensor_info*)
		oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
	if (sensor_info == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	orig_assert_mask = sensor_info->assert_mask;
	orig_deassert_mask = sensor_info->deassert_mask;

	/* Based on the action type, the bits in assert/deassert mask are set
	 * or cleared
	 */
	if (action == SAHPI_SENS_ADD_EVENTS_TO_MASKS) {
		sensor_info->assert_mask = sensor_info->assert_mask | assert;

		if (rpt->ResourceCapabilities &
		    SAHPI_CAPABILITY_EVT_DEASSERTS) {
			sensor_info->deassert_mask = sensor_info->assert_mask;
		} else {
			sensor_info->deassert_mask =
				sensor_info->deassert_mask | deassert;
		}
	} else if (assert != 0 &&
		   action == SAHPI_SENS_REMOVE_EVENTS_FROM_MASKS) {
		sensor_info->assert_mask = sensor_info->assert_mask & ~(assert);
		if (rpt->ResourceCapabilities &
		    SAHPI_CAPABILITY_EVT_DEASSERTS) {
			sensor_info->deassert_mask = sensor_info->assert_mask;
		} else if (deassert != 0) {
			sensor_info->deassert_mask =
				sensor_info->deassert_mask & ~(deassert);
		}
	}

	if ((sensor_info->assert_mask != orig_assert_mask) ||
	    (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS) &&
		sensor_info->deassert_mask != orig_deassert_mask)) {
		/* If the assert or deassert mask has change, raise a
		 * "sensor enable event"
		 */
		rv =  generate_sensor_enable_event(oh_handler, rdr_num, rpt,
						   rdr, sensor_info);
		if (rv != SA_OK) {
			err("Event generation failed");
			return rv;
		}
	}

        return SA_OK;
}

/**
 * update_sensor_rdr
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Sensor rdr number
 *      @rpt: Pointer to rpt Structure
 *      @sensor_data: Pointer to sensor reading data
 *
 * Purpose:
 *      Returns current status of the sensor RDR from resource
 *
 * Detailed Description:
 *      - Fetches current reading of the sensor from the resource
 *        by soap call and returns reading in sensor data
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_UNKNOWN - Invalid entity type
 **/
SaErrorT update_sensor_rdr(struct oh_handler_state *oh_handler,
                           SaHpiResourceIdT resource_id,
                           SaHpiSensorNumT rdr_num,
                           SaHpiRptEntryT *rpt,
                           struct oa_soap_sensor_reading_data *sensor_data)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler = NULL;
        struct oa_soap_handler *oa_handler = NULL;

        struct getThermalInfo thermal_request;
        struct thermalInfo thermal_response;
	struct getBladeThermalInfoArray blade_thermal_request;
	struct bladeThermalInfoArrayResponse blade_thermal_response;
	struct bladeThermalInfo blade_thermal_info;
        struct getBladeStatus server_status_request;
        struct bladeStatus server_status_response;
        struct getFanInfo fan_request;
        struct fanInfo fan_response;
        struct getPowerSupplyInfo power_supply_request;
        struct powerSupplyInfo power_supply_response;
        struct powerSubsystemInfo ps_response;
        SaHpiInt32T location = -1;

        if (oh_handler == NULL || rpt == NULL || sensor_data == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;
        location = rpt->ResourceEntity.Entry[0].EntityLocation;
        thermal_request.bayNumber = 
	server_status_request.bayNumber = 
        fan_request.bayNumber = 
	power_supply_request.bayNumber = 
	blade_thermal_request.bayNumber = location;

        /* Getting the current reading of the sensor directly from resource
         * using a soap call
         */
        switch (rpt->ResourceEntity.Entry[0].EntityType) {
                case (SAHPI_ENT_SYSTEM_BLADE):
                case (SAHPI_ENT_IO_BLADE):
                case (SAHPI_ENT_DISK_BLADE):
                        if ((rdr_num == OA_SOAP_SEN_TEMP_STATUS) || 
			    ((rdr_num >= OA_SOAP_BLD_THRM_SEN_START) &&
			     (rdr_num <= OA_SOAP_BLD_THRM_SEN_END))){
                                /* Fetching current thermal reading of the
                                 * server blade in the specified bay number
				 * NOTE: If the blade is in POWER OFF state or
				 * in unstable state, the control should not 
				 * reach this place
                                 */
				rv = soap_getBladeThermalInfoArray(
							oa_handler->active_con, 
							&blade_thermal_request, 
						       &blade_thermal_response);
                                if (rv != SOAP_OK) {
					err("Get blade's thermal info failed");
                                        return SA_ERR_HPI_INTERNAL_ERROR;
                                }
				
				/* Traverse the soap response and fetch the
				 * current sensor reading
				 */
				rv = oa_soap_get_bld_thrm_sen_data(rdr_num,
							blade_thermal_response,
							&blade_thermal_info);
				if (rv != SA_OK) {
					err("Could not find the matching"
					    " sensors info from blade");
					return rv;
				}

                                sensor_data->data.IsSupported = SAHPI_TRUE;
                                sensor_data->data.Type =
                                        SAHPI_SENSOR_READING_TYPE_FLOAT64;
                                sensor_data->data.Value.SensorFloat64 =
                                        blade_thermal_info.temperatureC;
                        }
                        else if (rdr_num == OA_SOAP_SEN_PWR_STATUS) {

                                /* Fetching current power status of the
                                 * server blade in the specified bay number
                                 */
                                rv = soap_getBladeStatus(
                                        oa_handler->active_con,
                                        &server_status_request,
                                        &server_status_response);
                                if (rv != SOAP_OK) {
                                        return SA_ERR_HPI_INTERNAL_ERROR;
                                }
                                sensor_data->data.IsSupported = SAHPI_TRUE;
                                sensor_data->data.Type =
                                        SAHPI_SENSOR_READING_TYPE_FLOAT64;
                                sensor_data->data.Value.SensorFloat64 =
                                        server_status_response.powerConsumed;
                        }
                        break;
                case (SAHPI_ENT_SWITCH_BLADE):
                        thermal_request.sensorType = SENSOR_TYPE_INTERCONNECT;

                        /* Fetching current thermal reading of the switch blade
                         * in the specified bay number
                         */
                        rv = soap_getThermalInfo(oa_handler->active_con,
                                                 &thermal_request,
                                                 &thermal_response);
                        if (rv != SOAP_OK) {
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        sensor_data->data.IsSupported = SAHPI_TRUE;
                        sensor_data->data.Type =
                                SAHPI_SENSOR_READING_TYPE_FLOAT64;
                        sensor_data->data.Value.SensorFloat64 =
                                thermal_response.temperatureC;
                        break;
                case (SAHPI_ENT_SYS_MGMNT_MODULE):
                        thermal_request.sensorType = SENSOR_TYPE_OA;

                        /* Fetching current thermal readng of the OA
                         * in the specified bay number
                         */
                        rv = soap_getThermalInfo(oa_handler->active_con,
                                                 &thermal_request,
                                                 &thermal_response);
                        if (rv != SOAP_OK) {
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        sensor_data->data.IsSupported = SAHPI_TRUE;
                        sensor_data->data.Type =
                                SAHPI_SENSOR_READING_TYPE_FLOAT64;
                        sensor_data->data.Value.SensorFloat64 =
                                thermal_response.temperatureC;
                        break;
                case (SAHPI_ENT_SYSTEM_CHASSIS):
                        thermal_request.sensorType = SENSOR_TYPE_ENC;

                        /* Fetching current thermal readng of the Enclosure
                         * in the specified bay number
                         */
                        rv = soap_getThermalInfo(oa_handler->active_con,
                                                 &thermal_request,
                                                 &thermal_response);
                        if (rv != SOAP_OK) {
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        sensor_data->data.IsSupported = SAHPI_TRUE;
                        sensor_data->data.Type =
                                SAHPI_SENSOR_READING_TYPE_FLOAT64;
                        sensor_data->data.Value.SensorFloat64 =
                                thermal_response.temperatureC;
                        break;
                case (SAHPI_ENT_FAN):

                        /* Fetching current speed and power consumption info
                         * of fan in the specified bay number
                         */
                        rv = soap_getFanInfo(oa_handler->active_con,
                                             &fan_request, &fan_response);
                        if (rv != SOAP_OK) {
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        sensor_data->data.IsSupported = SAHPI_TRUE;
                        sensor_data->data.Type =
                                SAHPI_SENSOR_READING_TYPE_FLOAT64;
                        if (rdr_num == OA_SOAP_SEN_FAN_SPEED) {
                                sensor_data->data.Value.SensorFloat64 =
                                        fan_response.maxFanSpeed;
                        } else if (rdr_num == OA_SOAP_SEN_PWR_STATUS) {
                                sensor_data->data.Value.SensorFloat64 =
                                        fan_response.powerConsumed;
                        }
                        break;
                case (SAHPI_ENT_POWER_MGMNT):

                        /* Fetching current power info of power subsystem */
                        rv = soap_getPowerSubsystemInfo(oa_handler->active_con,
                                                        &ps_response);
                        if (rv != SOAP_OK) {
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        sensor_data->data.IsSupported = SAHPI_TRUE;
                        sensor_data->data.Type =
                                SAHPI_SENSOR_READING_TYPE_FLOAT64;
                        if (rdr_num == OA_SOAP_SEN_IN_PWR) {
                                sensor_data->data.Value.SensorFloat64 =
                                        ps_response.inputPowerVa;
                        }
                        if (rdr_num == OA_SOAP_SEN_OUT_PWR) {
                                sensor_data->data.Value.SensorFloat64 =
                                        ps_response.outputPower;
                        }
                        if (rdr_num == OA_SOAP_SEN_PWR_STATUS) {
                                sensor_data->data.Value.SensorFloat64 =
                                        ps_response.powerConsumed;
                        }
                        if (rdr_num == OA_SOAP_SEN_PWR_CAPACITY) {
                                sensor_data->data.Value.SensorFloat64 =
                                        ps_response.capacity;
                        }
                        break;
                case (SAHPI_ENT_POWER_SUPPLY):
                        /* Fetching current actual power output info of
                         * power supply in the specified bay number
                         */
                        rv = soap_getPowerSupplyInfo(oa_handler->active_con,
                                                     &power_supply_request,
                                                     &power_supply_response);
                        if (rv != SOAP_OK) {
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        sensor_data->data.IsSupported = SAHPI_TRUE;
                        sensor_data->data.Type =
                                SAHPI_SENSOR_READING_TYPE_FLOAT64;
                        sensor_data->data.Value.SensorFloat64 =
                                power_supply_response.actualOutput;
                        break;
                default:
                        err("Wrong resource type");
                        return SA_ERR_HPI_UNKNOWN;
        }
        return SA_OK;
}

/**
 * generate_sensor_enable_event
 *      @oh_handler: Handler data pointer.
 *      @rdr_num: Sensor rdr number.
 *      @rpt: Pointer to rpt structure.
 *      @rdr: Pointer to rdr structure.
 *      @sensor_info: Pointer to sensor information structure
 *
 * Purpose:
 *      Builds and generates the sensor enable event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 **/
SaErrorT generate_sensor_enable_event(void *oh_handler,
                                      SaHpiSensorNumT rdr_num,
                                      SaHpiRptEntryT *rpt,
                                      SaHpiRdrT *rdr,
                                      struct oa_soap_sensor_info *sensor_info)
{
        struct oh_handler_state *handler = NULL;
        struct oh_event event;

        if (oh_handler == NULL || sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        memset(&event, 0, sizeof(struct oh_event));
        event.hid = handler->hid;
        event.event.EventType = SAHPI_ET_SENSOR_ENABLE_CHANGE;
        /* TODO: map the timestamp of the OA generated event */
        oh_gettimeofday(&(event.event.Timestamp));
        event.event.Severity = SAHPI_INFORMATIONAL;
        memcpy(&event.resource, rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = rpt->ResourceId;
        event.event.EventDataUnion.SensorEnableChangeEvent.SensorNum = rdr_num;
        event.event.EventDataUnion.SensorEnableChangeEvent.SensorType =
                rdr->RdrTypeUnion.SensorRec.Type;
        event.event.EventDataUnion.SensorEnableChangeEvent.EventCategory =
                rdr->RdrTypeUnion.SensorRec.Category;
        event.event.EventDataUnion.SensorEnableChangeEvent.SensorEnable =
                sensor_info->sensor_enable;
        event.event.EventDataUnion.SensorEnableChangeEvent.SensorEventEnable =
                sensor_info->event_enable;
        event.event.EventDataUnion.SensorEnableChangeEvent.AssertEventMask =
                sensor_info->assert_mask;
        event.event.EventDataUnion.SensorEnableChangeEvent.DeassertEventMask =
                sensor_info->deassert_mask;
        event.rdrs = g_slist_append(event.rdrs,
                                    g_memdup(rdr, sizeof(SaHpiRdrT)));

        event.event.EventDataUnion.SensorEnableChangeEvent.OptionalDataPresent =
                 SAHPI_SEOD_CURRENT_STATE;

        /* If the current state is SAHPI_ES_UPPER_CRIT then the current
         * asserted event states are SAHPI_ES_UPPER_CRIT and
         * SAHPI_ES_UPPER_MAJOR.
         */
        if (rdr->RdrTypeUnion.SensorRec.Category == SAHPI_EC_THRESHOLD &&
	     sensor_info->current_state == SAHPI_ES_UPPER_CRIT) {
                event.event.EventDataUnion.SensorEnableChangeEvent.
                        CurrentState = SAHPI_ES_UPPER_CRIT |
                                       SAHPI_ES_UPPER_MAJOR;
        } else {
                event.event.EventDataUnion.SensorEnableChangeEvent.
                        CurrentState = sensor_info->current_state;
        }

        oh_evt_queue_push(handler->eventq, copy_oa_soap_event(&event));
        return SA_OK;
}

/**
 * generate_sensor_assert_thermal_event
 *      @oh_handler: Handler data pointer
 *      @rdr_num: Sensor rdr number
 *      @rpt: Pointer to rpt structure
 *      @rdr: Pointer to rdr structure
 *      @current_reading: Current reading of sensor
 *      @event_severity: Severity of thermal event
 *      @sensor_info: Pointer to sensor information structure
 *
 * Purpose:
 *      Builds and generates the sensor assert thermal event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT generate_sensor_assert_thermal_event(void *oh_handler,
                                              SaHpiSensorNumT rdr_num,
                                              SaHpiRptEntryT *rpt,
                                              SaHpiRdrT *rdr,
                                              SaHpiSensorReadingT
                                                      current_reading,
                                              SaHpiSeverityT event_severity,
                                              struct oa_soap_sensor_info
                                                      *sensor_info)
{
        struct oh_handler_state *handler = NULL;
        struct oh_event event;

        if (oh_handler == NULL || sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        memset(&event, 0, sizeof(struct oh_event));

        /* Update the event data structure with default values and current
         * state of the thermal reading
         */
        event.hid = handler->hid;
        event.event.EventType = SAHPI_ET_SENSOR;
        oh_gettimeofday(&(event.event.Timestamp));
        event.event.Severity = event_severity;
        memcpy(&event.resource, rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = rpt->ResourceId;
        event.event.EventDataUnion.SensorEvent.SensorNum = rdr_num;
        event.event.EventDataUnion.SensorEvent.SensorType = SAHPI_TEMPERATURE;
        event.event.EventDataUnion.SensorEvent.EventCategory =
                SAHPI_EC_THRESHOLD;
        event.event.EventDataUnion.SensorEvent.Assertion = SAHPI_TRUE;
        event.event.EventDataUnion.SensorEvent.EventState =
                sensor_info->current_state;

        if (current_reading.IsSupported == SAHPI_TRUE) {
                event.event.EventDataUnion.SensorEvent.OptionalDataPresent =
                        SAHPI_SOD_TRIGGER_READING |
                        SAHPI_SOD_TRIGGER_THRESHOLD |
                        SAHPI_SOD_PREVIOUS_STATE |
                        SAHPI_SOD_CURRENT_STATE;
                event.event.EventDataUnion.SensorEvent.TriggerReading =
                        current_reading;
        } else {
                event.event.EventDataUnion.SensorEvent.OptionalDataPresent =
                        SAHPI_SOD_TRIGGER_THRESHOLD |
                        SAHPI_SOD_PREVIOUS_STATE |
                        SAHPI_SOD_CURRENT_STATE;
        }

        switch (sensor_info->current_state) {
                case (SAHPI_ES_UNSPECIFIED):
                        err("There is no event to assert");
                        return SA_OK;
                case (SAHPI_ES_UPPER_MAJOR):
                        /* Raise an assert event if the thermal reading crosses
                         * the major threshold and assert mask for event is
                         * enabled
                         */
                        if (! (sensor_info->assert_mask & SAHPI_STM_UP_MAJOR)) {
                                err("Assert mask for major threshold is "
                                    "not set");
                                return SA_OK;
                        }

                        if (sensor_info->previous_state ==
                            SAHPI_ES_UNSPECIFIED) {
                                event.event.EventDataUnion.SensorEvent.
                                        TriggerThreshold =
                                        sensor_info->threshold.UpMajor;
                        } else {
                                err("There is no event to assert");
                                return SA_OK;
                        }
                        break;
                case (SAHPI_ES_UPPER_CRIT):
                        /* Raise an assert event if the thermal reading crosses
                         * the critical threshold and assert mask for event is
                         * enabled
                         */
                        if (! (sensor_info->assert_mask & SAHPI_STM_UP_CRIT)) {
                                err("Assert mask for critical threshold is "
                                    "not set");
                                return SA_OK;
                        }
                        event.event.EventDataUnion.SensorEvent.
                                TriggerThreshold =
                                sensor_info->threshold.UpCritical;
                        break;
                default:
                        err("Invalid current state for asserting the event");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Update the event structure with previous and current state after
         * a threshold has been crossed
         */
        event.event.EventDataUnion.SensorEvent.PreviousState =
                sensor_info->previous_state;
        /* If the current state is SAHPI_ES_UPPER_CRIT the current asserted
         * event states are SAHPI_ES_UPPER_CRIT and SAHPI_ES_UPPER_MAJOR
         */
        if (sensor_info->current_state == SAHPI_ES_UPPER_CRIT) {
                event.event.EventDataUnion.SensorEvent.CurrentState =
                        SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR;
        } else {
                event.event.EventDataUnion.SensorEvent.CurrentState =
                        sensor_info->current_state;
        }

        event.rdrs = g_slist_append(event.rdrs,
                                    g_memdup(rdr, sizeof(SaHpiRdrT)));

        oh_evt_queue_push(handler->eventq, copy_oa_soap_event(&event));
        return SA_OK;
}

/**
 * generate_sensor_deassert_thermal_event
 *      @oh_handler: Handler data pointer.
 *      @rdr_num: Sensor rdr number.
 *      @rpt: Pointer to rpt structure.
 *      @rdr: Pointer to rdr structure
 *      @current_reading: Current reading of sensor
 *      @sensor_info: Pointer to sensor information structure
 *
 * Purpose:
 *      Builds and generates the sensor deassert thermal event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT generate_sensor_deassert_thermal_event(void *oh_handler,
                                                SaHpiSensorNumT rdr_num,
                                                SaHpiRptEntryT *rpt,
                                                SaHpiRdrT *rdr,
                                                SaHpiSensorReadingT
                                                        current_reading,
                                                SaHpiSeverityT event_severity,
                                                struct oa_soap_sensor_info
                                                        *sensor_info)
{
        struct oh_handler_state *handler = NULL;
        struct oh_event event;

        if (oh_handler == NULL || sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        memset(&event, 0, sizeof(struct oh_event));

        /* Update the event data structure with default values and current
         * state of the thermal reading
         */
        event.hid = handler->hid;
        event.event.EventType = SAHPI_ET_SENSOR;
        oh_gettimeofday(&(event.event.Timestamp));
        event.event.Severity = event_severity;
        memcpy(&event.resource, rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = rpt->ResourceId;
        event.event.EventDataUnion.SensorEvent.SensorNum = rdr_num;
        event.event.EventDataUnion.SensorEvent.SensorType = SAHPI_TEMPERATURE;
        event.event.EventDataUnion.SensorEvent.EventCategory =
                SAHPI_EC_THRESHOLD;
        event.event.EventDataUnion.SensorEvent.Assertion = SAHPI_FALSE;
        event.event.EventDataUnion.SensorEvent.EventState =
                sensor_info->previous_state;

        if (current_reading.IsSupported == SAHPI_TRUE) {
                event.event.EventDataUnion.SensorEvent.OptionalDataPresent =
                        SAHPI_SOD_TRIGGER_READING |
                        SAHPI_SOD_TRIGGER_THRESHOLD |
                        SAHPI_SOD_PREVIOUS_STATE |
                        SAHPI_SOD_CURRENT_STATE;
                event.event.EventDataUnion.SensorEvent.TriggerReading =
                        current_reading;
        } else {
                event.event.EventDataUnion.SensorEvent.OptionalDataPresent =
                        SAHPI_SOD_TRIGGER_THRESHOLD |
                        SAHPI_SOD_PREVIOUS_STATE |
                        SAHPI_SOD_CURRENT_STATE;
        }

        switch (sensor_info->current_state) {
                case (SAHPI_ES_UNSPECIFIED):
                        /* Raise an deassert event if the thermal reading drops
                         * below the major threshold and deassert mask for event
                         * is enabled
                         */
                        if (! (sensor_info->deassert_mask &
                               SAHPI_STM_UP_MAJOR)) {
                                err("Event deassert mask for major threshold "
                                    "is not set");
                                return SA_OK;
                        }
                        if (sensor_info->previous_state == SAHPI_STM_UP_MAJOR) {
                                event.event.EventDataUnion.SensorEvent.
                                        TriggerThreshold =
                                        sensor_info->threshold.UpMajor;
                        } else {
                                err("There is no event to deassert");
                                return SA_OK;
                        }
                        break;
                case (SAHPI_ES_UPPER_MAJOR):
                        /* Raise an deassert event if the thermal reading drops
                         * below the critical threshold and deassert mask for
                         * event is enabled
                         */
                        if (sensor_info->previous_state ==
                            SAHPI_ES_UPPER_CRIT) {
                                if (! (sensor_info->deassert_mask &
                                       SAHPI_STM_UP_CRIT)) {
                                        err("Event deassert mask for critical "
                                            "threshold is not set");
                                        return SA_OK;
                                }
                                event.event.EventDataUnion.SensorEvent.
                                        TriggerThreshold =
                                        sensor_info->threshold.UpCritical;
                        } else {
                                err("There is no event to deassert");
                                return SA_OK;
                        }
                        break;
                case (SAHPI_ES_UPPER_CRIT):
                        err("There is no event to deassert");
                        return SA_OK;
                        break;
                default:
                        err("Invalid current state");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Update the event structure with previous and current state after
         * a threshold has been crossed.
         * If the previous state is SAHPI_ES_UPPER_CRIT the previous asserted
         * event states are SAHPI_ES_UPPER_CRIT and SAHPI_ES_UPPER_MAJOR
         */
        if (sensor_info->previous_state == SAHPI_ES_UPPER_CRIT) {
                event.event.EventDataUnion.SensorEvent.PreviousState =
                        SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR;
        } else {
                event.event.EventDataUnion.SensorEvent.PreviousState =
                        sensor_info->previous_state;
        }
        event.event.EventDataUnion.SensorEvent.CurrentState =
                sensor_info->current_state;

        event.rdrs = g_slist_append(event.rdrs,
                                    g_memdup(rdr, sizeof(SaHpiRdrT)));

        oh_evt_queue_push(handler->eventq, copy_oa_soap_event(&event));
        return SA_OK;
}

/**
 * check_and_deassert_event
 *      @oh_handler: Pointer to openhpi handler
 *      @resource_id: Resource id
 *      @rdr: Pointer to rdr structure
 *      @sensor_info: Pointer to the sensor information
 *
 * Purpose:
 *      Check and deassert the pending events on resource
 *
 * Detailed Description:
 *      - This is the module is called to check what thermal event to deassert
 *        based on the current state
 *
 * Return values:
 *      SA_OK  - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT check_and_deassert_event(struct oh_handler_state *oh_handler,
                                  SaHpiResourceIdT resource_id,
                                  SaHpiRdrT *rdr,
                                  struct oa_soap_sensor_info *sensor_info)
{
        SaErrorT rv = SA_OK;
        SaHpiSeverityT event_severity;
        SaHpiSensorReadingT current_reading;
        SaHpiRptEntryT *rpt = NULL;

        current_reading.IsSupported = SAHPI_FALSE;

        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        if (sensor_info->current_state == SAHPI_ES_UPPER_CRIT) {
                /* if the current state is CRITICAL, then this module would
                 * have been called when the thermal reading would have dropped
                 * below critical threshold.
                 * Hence thermal event raised for crossing critical threshold
                 * will be deasserted
                 */
                sensor_info->previous_state = SAHPI_ES_UPPER_CRIT;
                sensor_info->current_state = SAHPI_ES_UPPER_MAJOR;
                event_severity = SAHPI_CRITICAL;

                rv = generate_sensor_deassert_thermal_event(oh_handler,
                                              OA_SOAP_SEN_TEMP_STATUS,
                                              rpt,
                                              rdr,
                                              current_reading,
                                              event_severity,
                                              sensor_info);
                if (rv != SA_OK) {
                        err("Raising critical deassert thermal event failed");
                }
        }
        if (sensor_info->current_state == SAHPI_ES_UPPER_MAJOR) {
                /* if the current state is MAJOR, then this module would
                 * have been called when the thermal reading would have dropped
                 * below major threshold.
                 * Hence thermal event raised for crossing major threshold
                 * will be deasserted
                 */
                sensor_info->previous_state = SAHPI_ES_UPPER_MAJOR;
                sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
                event_severity = SAHPI_MAJOR;

                rv = generate_sensor_deassert_thermal_event(oh_handler,
                                              OA_SOAP_SEN_TEMP_STATUS,
                                              rpt,
                                              rdr,
                                              current_reading,
                                              event_severity,
                                              sensor_info);
                if (rv != SA_OK) {
                        err("Raising major deassert thermal event failed");
                }
        }
        return SA_OK;
}

/**
 * oa_soap_build_sen_rdr
 *      @oh_handler: Pointer to openhpi handler
 *      @resource_id: Resource id
 *      @rdr: Pointer to rdr structure
 *      @sensor_info: Pointer to the sensor information
 *	@sensor_num: Sensor number
 *
 * Purpose:
 *      Build the sensor RDR
 *
 * Detailed Description:
 *	- Allocates the memory for sensor info
 *	- Copies the sensor RDR from the global sensor array
 *
 * Return values:
 *      SA_OK  - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT oa_soap_build_sen_rdr(struct oh_handler_state *oh_handler,
			       SaHpiResourceIdT resource_id,
			       SaHpiRdrT *rdr,
			       struct oa_soap_sensor_info **sensor_info,
			       SaHpiSensorNumT sensor_num)
{
        SaHpiRptEntryT *rpt = NULL;

	if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	/* Get the rpt entry of the resource */
	rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
	if (rpt == NULL) {
		err("resource RPT is NULL");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Sensor specific information is stored in this structure */
	*sensor_info =
		 g_memdup(&(oa_soap_sen_arr[sensor_num].sensor_info),
			  sizeof(struct oa_soap_sensor_info));
	if (*sensor_info == NULL) {
		err("oa_soap out of memory");
		return SA_ERR_HPI_OUT_OF_MEMORY;
	}

	/* Populate the sensor rdr with default value */
	rdr->Entity = rpt->ResourceEntity;
	rdr->RdrType = SAHPI_SENSOR_RDR;
	rdr->RdrTypeUnion.SensorRec = oa_soap_sen_arr[sensor_num].sensor;

	oh_init_textbuffer(&(rdr->IdString));
	oh_append_textbuffer(&(rdr->IdString),
			     oa_soap_sen_arr[sensor_num].comment);

	return SA_OK;
}

/**
 * oa_soap_map_sen_val
 *      @sensor_info: Pointer to the sensor information structure
 *      @sensor_num: Sensor number
 *      @sensor_value: Value of the sensor
 *      @sensor_status: Pointer to the sensor status
 *
 * Purpose:
 *      Maps the sensor value got from SOAP call to HPI sensor state, if it is
 *	changed
 *
 * Detailed Description:
 *       NA
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - One seeing the unsupported sensor value
 **/
SaErrorT oa_soap_map_sen_val(struct oa_soap_sensor_info *sensor_info,
			     SaHpiSensorNumT sensor_num,
			     SaHpiInt32T sensor_value,
			     SaHpiInt32T *sensor_status)
{
	SaHpiInt32T sensor_class;

	if (sensor_info == NULL || sensor_status == NULL) {
		err("Invalid parameter.");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	/* Get the sensor value */
	sensor_class = oa_soap_sen_arr[sensor_num].sensor_class;

	/* Check whether the sensor value is supported or not */
	if (oa_soap_sen_val_map_arr[sensor_class][sensor_value] == -1) {
		err("Not supported sensor value %d detected.", sensor_value);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Check whether HPI sensor value has changed or not*/
	if (sensor_info->current_state !=
	    oa_soap_sen_val_map_arr[sensor_class][sensor_value]) {
		/* Update the current sensor state */
		sensor_info->current_state =
			oa_soap_sen_val_map_arr[sensor_class][sensor_value];
		/* Get the assert state of the sensor */	
		*sensor_status =
			 oa_soap_sen_assert_map_arr[sensor_class][sensor_value];
	} else {
		/* Sensor value has not changed */  
		*sensor_status = OA_SOAP_SEN_NO_CHANGE;
	}

	return SA_OK;
}

/**
 * oa_soap_gen_res_evt
 *      @oh_handler: Pointer to openhpi handler
 *      @rpt: Pointer to the rpt structure
 *      @sensor_status: sensor status
 *
 * Purpose:
 *      Generates the HPI reource event
 *	changed
 *
 * Detailed Description:
 *	- If the operational status sensor state is asserted, then resource
 *	  event type is set to FAILURE and ResourceFailed field is set to TRUE.
 *	- If the operational status sensor state is deasserted, then resource
 *	  event type is set to RESTORED and ResourceFailed field is set to
 *	  FALSE.
 *	- Pushes the modified RPT entry to plugin rptcache.
 *	- Raises the HPI resource event.
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - One seeing the unsupported sensor value
 **/
static void oa_soap_gen_res_evt(struct oh_handler_state *oh_handler,
				SaHpiRptEntryT *rpt,
				SaHpiInt32T sensor_status)
{
	SaErrorT rv;
	struct oh_event event;
	struct oa_soap_hotswap_state *hotswap_state = NULL;

	if (oh_handler == NULL || rpt == NULL) {
		err("Invalid parameters");
		return;
	}

        memset(&event, 0, sizeof(struct oh_event));

	if (sensor_status == OA_SOAP_SEN_ASSERT_TRUE &&
	    rpt->ResourceFailed != SAHPI_TRUE) {
		/* Resource failed */
		event.event.EventDataUnion.ResourceEvent.
			ResourceEventType = SAHPI_RESE_RESOURCE_FAILURE;
		rpt->ResourceFailed = SAHPI_TRUE;
	} else if (sensor_status == OA_SOAP_SEN_ASSERT_FALSE &&
		   rpt->ResourceFailed != SAHPI_FALSE) {
		/* Resource restored */
		event.event.EventDataUnion.ResourceEvent.
			ResourceEventType = SAHPI_RESE_RESOURCE_RESTORED;
		rpt->ResourceFailed = SAHPI_FALSE;
	} else {
		/* Do not generate resource event as there is no change */
		return;
	}

        /* Update the event structure */
        event.hid = oh_handler->hid;
        oh_gettimeofday(&(event.event.Timestamp));
        event.event.Severity = SAHPI_CRITICAL;
        event.event.Source = rpt->ResourceId;
	event.event.EventType = SAHPI_ET_RESOURCE;

	/* Get the hotswap structure */
	if (rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
		hotswap_state = (struct oa_soap_hotswap_state *)
                        oh_get_resource_data(oh_handler->rptcache,
                                             rpt->ResourceId);
	}

	/* Update the RPT entry */
        rv = oh_add_resource(oh_handler->rptcache, rpt, hotswap_state, 0);
        if (rv != SA_OK) {
                err("Adding resource failed");
                return;
        }

        memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));
}

/**
 * oa_soap_gen_sen_evt
 *      @oh_handler: Pointer to openhpi handler
 *      @rpt: Pointer to the rpt structure
 *      @rdr: Pointer to rdr structure
 *      @sensor_status: Status of the sensor
 *
 * Purpose:
 *      Generates the HPI sensor event
 *
 * Detailed Description:
 * 	- Obtains the sensor event structure from the global sensor array
 *	- If the trigger reading is greater than zero, then sets the trigger
 *	  reading value in the sensor event structure.
 *	- If the trigger threshold is greater than zero, then sets the trigger
 *	  threshold value in the sensor event structure.
 *	- Appends the sensor RDR to the event structure
 *	- Raises the HPI sensor event
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 **/
static SaErrorT oa_soap_gen_sen_evt(struct oh_handler_state *oh_handler,
				    SaHpiRptEntryT *rpt,
	      			    SaHpiRdrT *rdr,
		      		    SaHpiInt32T sensor_status,
				    SaHpiFloat64T trigger_reading,
				    SaHpiFloat64T trigger_threshold)
{
	struct oh_event event;
	SaHpiSensorNumT sensor_num;

	if (oh_handler == NULL || rpt == NULL || rdr == NULL) {
		err("Invalid parameter.");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	sensor_num = rdr->RdrTypeUnion.SensorRec.Num;

	/* Update the event structure */
        memset(&event, 0, sizeof(struct oh_event));
	/* Get the event structure from gloabl array */
	event.event = oa_soap_sen_arr[sensor_num].sen_evt[sensor_status];
	if (trigger_reading > 0) {
		event.event.EventDataUnion.SensorEvent.TriggerReading.Value.
		SensorFloat64
			 = trigger_reading;
	}
	if (trigger_threshold > 0) {
		event.event.EventDataUnion.SensorEvent.TriggerThreshold.Value.
		SensorFloat64
			= trigger_threshold;
	}

	memcpy(&(event.resource), rpt, sizeof(SaHpiRptEntryT));
        event.event.Source = event.resource.ResourceId;
        event.hid = oh_handler->hid;
        oh_gettimeofday(&(event.event.Timestamp));
	event.rdrs = g_slist_append(event.rdrs,
                                    g_memdup(rdr, sizeof(SaHpiRdrT)));
	/* Raise the HPI sensor event */
        oh_evt_queue_push(oh_handler->eventq, copy_oa_soap_event(&event));

	return SA_OK;
}

/**
 * oa_soap_proc_sen_evt
 *      @oh_handler: Pointer to openhpi handler
 *      @resource_id: Resource Id
 *      @sensor_num: Sensor number
 *      @sensor_value: Sensor value
 *
 * Purpose:
 *	Processes and raises the sensor event
 *
 * Detailed Description:
 *	- Raises the sensor event on changes in the state of the sensors with
 *	  sensor class OPER, PRED_FAIL, REDUND, DIAG, ENC_AGR_OPER,
 *	  ENC_AGR_PRED_FAIL, BOOL, BOOL_RVRS, HEALTH_OPER, HEALTH_PRED_FAIL.
 *	- Raises the resource failed or restored event if there is a change in
 *	  the operational status sensor state.
 *	- Raises the sensor event on changes in the state of the sensors with
 *	  sensor class TEMP
 *
 * Return values:
 *      SA_OK  - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT oa_soap_proc_sen_evt(struct oh_handler_state *oh_handler,
			      SaHpiResourceIdT resource_id,
			      SaHpiSensorNumT sensor_num,
			      SaHpiInt32T sensor_value,
			      SaHpiFloat64T trigger_reading,
			      SaHpiFloat64T trigger_threshold)
{
        SaErrorT rv = SA_OK;
	SaHpiInt32T sensor_status;
	SaHpiInt32T sensor_class;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_sensor_info *sensor_info;
	SaHpiInt32T event_index = -1;

        if (oh_handler == NULL) {
                err("wrong parameters passed");
		return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Get the enable sensor RDR */
        rdr = oh_get_rdr_by_type(oh_handler->rptcache, rpt->ResourceId,
                                 SAHPI_SENSOR_RDR, sensor_num);
        if (rdr == NULL) {
                err("RDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Get the sensor information */
        sensor_info = (struct oa_soap_sensor_info*)
		oh_get_rdr_data(oh_handler->rptcache, rpt->ResourceId,
			        rdr->RecordId);
        if (sensor_info == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

	sensor_class = oa_soap_sen_arr[sensor_num].sensor_class;

	switch (sensor_class) {
		case OA_SOAP_OPER_CLASS:
		case OA_SOAP_PRED_FAIL_CLASS:
		case OA_SOAP_REDUND_CLASS:
		case OA_SOAP_DIAG_CLASS:
		case OA_SOAP_ENC_AGR_OPER_CLASS:
		case OA_SOAP_ENC_AGR_PRED_FAIL_CLASS:
		case OA_SOAP_BOOL_CLASS:
		case OA_SOAP_BOOL_RVRS_CLASS:
		case OA_SOAP_HEALTH_OPER_CLASS:
		case OA_SOAP_HEALTH_PRED_FAIL_CLASS:
			rv = oa_soap_map_sen_val(sensor_info, sensor_num,
						 sensor_value, &sensor_status);
			if (rv != SA_OK) {
				err("Setting sensor value has failed");
				return rv;
			}

			/* If there is no change in the sensor value, ignore the
			 * OA event
			 */
			if (sensor_status == OA_SOAP_SEN_NO_CHANGE)
				return SA_OK;

			/* Ignore the sensor event if the sensor is disabled or
			 * sensor event is disabled
			 */
			if (sensor_info->sensor_enable == SAHPI_FALSE ||
			    sensor_info->event_enable == SAHPI_FALSE) {
				dbg ("Sensor is disabled or sensor event is "
				     "disabled");
			} else {
				/* Generate the sensor event */
				oa_soap_gen_sen_evt(oh_handler, rpt, rdr,
						    sensor_status, 0, 0);
			}

			/* Generate resource failed/restored event based on the
			 * operational status sensor state
			 */
			if (sensor_num == OA_SOAP_SEN_OPER_STATUS) {
				oa_soap_gen_res_evt(oh_handler, rpt,
						    sensor_status);
			}
			break;
		case OA_SOAP_TEMP_CLASS:
			/* If the thermal sensor is enabled and sensor
			 * event is enabled
			 * Then raise the thermal sensor events,
			 * Else, ignore the thermal event from OA
			 */
			if (sensor_info->sensor_enable == SAHPI_FALSE ||
			    sensor_info->event_enable == SAHPI_FALSE) {
				dbg ("Sensor or sensor event is disabled");
				return SA_OK;
			}

			/* Check the current state of the sensor and determine
			 * the index of event in event array of the sensor
			 * required for raising the event
			 */
			switch (sensor_value) {
				case  SENSOR_STATUS_OK:
					/* Sensor has changed the state from
					 * CAUTION to OK. 
					 * Update the event structure.
					 */
					sensor_info->previous_state = 
						SAHPI_ES_UPPER_MAJOR; 
					sensor_info->current_state =
						SAHPI_ES_UNSPECIFIED;
					event_index = OA_SOAP_TEMP_CAUT_OK;
					break;
				case SENSOR_STATUS_CAUTION:
					/* Sensor has changed the state to
					 * CAUTION from CRITICAL or OK.
					 * Update the event structure.
					 */
					sensor_info->previous_state =
						sensor_info->current_state;
					sensor_info->current_state =
						SAHPI_ES_UPPER_MAJOR;

					if (sensor_info->previous_state ==
					    SAHPI_ES_UNSPECIFIED) {
						event_index =
							OA_SOAP_TEMP_OK_CAUT;
					} else {
						event_index =
							OA_SOAP_TEMP_CRIT_CAUT;
					}
					break;
				case SENSOR_STATUS_CRITICAL:
					/* Sensor has changed the state from
					 * CAUTION to CRITICAL.
					 * Update the event structure.
					 */
					sensor_info->previous_state =
						SAHPI_ES_UPPER_MAJOR;
					sensor_info->current_state =
						SAHPI_ES_UPPER_CRIT;
					event_index = OA_SOAP_TEMP_CAUT_CRIT;
					break;
				default:
					err("Event not supported for the \
					     specified sensor status");
					return SA_ERR_HPI_INTERNAL_ERROR;
			}
			/* Raise the event */
			rv = oa_soap_gen_sen_evt(oh_handler, rpt, rdr,
						 event_index, trigger_reading,
						 trigger_threshold);
			if (rv != SA_OK) {
				err("Error in generating sensor event");
				return rv; 
			}
			break;
		default:
			err("No event support for specified class");
	}
	return SA_OK;
}
 
/**
 * oa_soap_map_thresh_resp
 *      @rdr: Pointer to the sensor rdr
 *      @oa_soap_threshold_sensor: Structure containing the threshold reading
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Updates the rdr structure with the threshold values retrieved from OA
 *	For Thermal sensors, current state in sensor_info is updated based on
 *	current reading
 *
 * Detailed Description:
 *       NA
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - One seeing the unsupported sensor value
 **/
SaErrorT oa_soap_map_thresh_resp(SaHpiRdrT *rdr,
				 void *response,
				 SaHpiBoolT event_support,
				 struct oa_soap_sensor_info *sensor_info)
{
	SaHpiSensorRecT *sensor = NULL;
	SaHpiUint32T current_reading = 0;
	SaHpiInt32T sensor_class;
	struct thermalInfo *thermal_response;
	struct bladeThermalInfo *blade_thermal_response;
	struct fanInfo *fan_info;

	if (rdr == NULL || sensor_info == NULL) {
		err("Invalid parameter.");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	sensor = &(rdr->RdrTypeUnion.SensorRec);
	sensor_class = oa_soap_sen_arr[sensor->Num].sensor_class;

	switch (sensor_class) {
		case OA_SOAP_TEMP_CLASS:
		case OA_SOAP_BLADE_THERMAL_CLASS:
			/* Ambient Zone thermal sensor is present for most of
			 * resource. Ambient Zone threshold info for Blade is
			 * available from bladeThermalInfo response, where as
			 * for other resources it is available it is available
			 * from thermalInfo response.
			 * Hence for Blade Ambient zone threshold info
			 * set sensor class as OA_SOAP_BLADE_THERMAL_CLASS to
			 * retrieve threshold values from correct response.
			 */

			if ((rdr->Entity.Entry[0].EntityType == 
						SAHPI_ENT_SYSTEM_BLADE) ||
			    (rdr->Entity.Entry[0].EntityType ==
						SAHPI_ENT_IO_BLADE) ||
			    (rdr->Entity.Entry[0].EntityType ==
						SAHPI_ENT_DISK_BLADE)) {
				sensor_class = OA_SOAP_BLADE_THERMAL_CLASS;
			}
			
			if (sensor_class == OA_SOAP_TEMP_CLASS) {
				/* Cast the response structure to thermal info
				 * response
				 */
				thermal_response = 
					(struct thermalInfo *)response;
				/* Updating the rdr with actual upper critical
				 * threshold value provided by OA
				 */
				sensor->DataFormat.Range.Max.Value.
								SensorFloat64 =
				sensor_info->threshold.UpCritical.Value.
								SensorFloat64 =
					thermal_response->criticalThreshold;

				sensor->DataFormat.Range.NormalMax.Value.
								SensorFloat64 =
				sensor_info->threshold.UpMajor.Value.
								SensorFloat64 =
					thermal_response->cautionThreshold;
				current_reading = 
				(SaHpiUint32T)thermal_response->temperatureC;
			} else if (sensor_class == 
						OA_SOAP_BLADE_THERMAL_CLASS) {
				/* Cast the response structure to blade thermal
				 * info response
				 */
				blade_thermal_response = 
					(struct bladeThermalInfo *)response;
				/* Updating the rdr with actual upper critical
				 * threshold value provided by OA
				 */
				sensor->DataFormat.Range.Max.Value.
								SensorFloat64 =
				sensor_info->threshold.UpCritical.Value.
								SensorFloat64 =
				blade_thermal_response->criticalThreshold;

				sensor->DataFormat.Range.NormalMax.Value.
								SensorFloat64 =
				sensor_info->threshold.UpMajor.Value.
								SensorFloat64 =
				blade_thermal_response->cautionThreshold;
				current_reading = 
				(SaHpiUint32T)blade_thermal_response->
								temperatureC;

			}

			/* Update the sensor info with current reading, this
			 * reading will be utilized sensor event assetion post
			 * discovery.
			 */
			if ((current_reading >=
			     sensor->DataFormat.Range.NormalMax.Value.
				SensorFloat64) &&
			    (current_reading <
			     sensor->DataFormat.Range.Max.Value.
				SensorFloat64)) {
				sensor_info->current_state =
					SAHPI_ES_UPPER_MAJOR;
			} else if (current_reading >
				   sensor->DataFormat.Range.Max.Value.
					SensorFloat64) {
					sensor_info->current_state =
						SAHPI_ES_UPPER_CRIT;
			}
			/* Update the sensor info with current reading, this
			 * reading will be utilized for sensor event assertion 
			 * post discovery.
			 */
			sensor_info->sensor_reading.Value.SensorFloat64 = 
				current_reading;

			if (event_support == SAHPI_TRUE) {
				/* Set the event support to TRUE. 
				 * By default, RDR will have event support set
				 * to FALSE
				 */
				sensor->EventCtrl = SAHPI_SEC_PER_EVENT;
				sensor->Events = SAHPI_ES_UPPER_CRIT | 
						 SAHPI_ES_UPPER_MAJOR;
				sensor_info->event_enable = SAHPI_TRUE;

				/* Thermal events are supported by OA only for
				 * crossing CAUTION threshold and CRITICAL
				 * THRESHOLD.
				 * Hence appropriate bit masks needs to set for
				 * assert and deassert mask
				 */
				sensor_info->assert_mask = 
					SAHPI_ES_UPPER_CRIT |
					SAHPI_ES_UPPER_MAJOR;

				sensor_info->deassert_mask =
					SAHPI_ES_UPPER_CRIT |
					SAHPI_ES_UPPER_MAJOR;
			}
			break;
		case OA_SOAP_FAN_SPEED_CLASS:
			/* Cast the response structure to fan info response
			 */
			fan_info = (struct fanInfo*) response;

			/* Updating the rdr with actual critical threshold
			 * value provided by OA
			 */
			sensor->DataFormat.Range.Max.Value.SensorFloat64 =
			fan_info->maxFanSpeed;

			/* Updating the rdr with actual critical threshold
			 * value provided by OA
			 */
			sensor->DataFormat.Range.Max.Value.SensorFloat64 =
			fan_info->lowLimitFanSpeed;

			/* Currently OA does not have event support for SPEED
			 * sensor, hence ignoring the event_support flag
			 */
			break;
		default:
			err("Sensor class not supported");
			return SA_ERR_HPI_INTERNAL_ERROR;
	}

	return SA_OK;
}

/**
 * oa_soap_assert_sen_evt
 *      @oh_handler		: Pointer to openhpi handler
 *      @rpt			: Pointer to the RPT entry
 *      @assert_sensor_list	: Pointer to the assert sensor list
 *
 * Purpose:
 *	Generates the sensor events for asserted sensors.
 *
 * Detailed Description:
 *	- Extracts the sensor RDR from the assert sensor list
 *	- Obtains the sensor number and sensor class.
 *	- Gets the appropriate sensor assert event structure from the global
 *	  sensor array
 *	- Raises the sensor assert event
 *	- If the operational status of the resource has asserted, then raises
 *	  the resource failed event
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 **/
SaErrorT oa_soap_assert_sen_evt(struct oh_handler_state *oh_handler,
				SaHpiRptEntryT *rpt,
				GSList *assert_sensor_list)
{
	GSList *node = NULL;
	SaHpiRdrT *rdr;
	SaHpiSensorNumT sensor_num;
	SaHpiInt32T sensor_class, assert_state;
	struct oa_soap_sensor_info *sensor_info;
	SaHpiFloat64T trigger_reading, trigger_threshold;

	if (oh_handler == NULL || rpt == NULL || assert_sensor_list == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	for (node = assert_sensor_list; node; node = node->next) {
		rdr = (SaHpiRdrT *)node->data;
		sensor_num = rdr->RdrTypeUnion.SensorRec.Num;
		/* Get the sensor information */
		sensor_info = (struct oa_soap_sensor_info*)
			oh_get_rdr_data(oh_handler->rptcache, rpt->ResourceId,
					rdr->RecordId);

		sensor_class = oa_soap_sen_arr[sensor_num].sensor_class;
		switch (sensor_class) {
			case OA_SOAP_OPER_CLASS:
			case OA_SOAP_PRED_FAIL_CLASS:
			case OA_SOAP_REDUND_CLASS:
			case OA_SOAP_DIAG_CLASS:
			case OA_SOAP_ENC_AGR_OPER_CLASS:
			case OA_SOAP_ENC_AGR_PRED_FAIL_CLASS:
			case OA_SOAP_BOOL_CLASS:
			case OA_SOAP_BOOL_RVRS_CLASS:
			case OA_SOAP_HEALTH_OPER_CLASS:
			case OA_SOAP_HEALTH_PRED_FAIL_CLASS:
				trigger_reading = 0;
				trigger_threshold = 0;
				assert_state = OA_SOAP_SEN_ASSERT_TRUE;

				/* If the blade type is IO_BLADE or DISK BLADE
				 * and if predictive failure sensor is 
				 * asserted, then change the power state of 
				 * partner blade in oa_soap_bay_pwr_status
				 * to SAHPI_POWER_OFF
				 */
				if ((rpt->ResourceEntity.Entry[0].EntityType == 
				     SAHPI_ENT_IO_BLADE) || 
				    (rpt->ResourceEntity.Entry[0].EntityType == 
				     SAHPI_ENT_DISK_BLADE)) {
					if (sensor_num ==
					    OA_SOAP_SEN_PRED_FAIL) {
						oa_soap_bay_pwr_status[rpt->
							ResourceEntity.Entry[0].
							EntityLocation -1] =
								SAHPI_POWER_OFF;
					}
				}
				break;
			case OA_SOAP_TEMP_CLASS:
				trigger_reading =
					sensor_info->sensor_reading.Value.
						SensorFloat64;
				trigger_threshold =
					sensor_info->threshold.UpMajor.Value.
						SensorFloat64;
				assert_state = OA_SOAP_TEMP_OK_CAUT;
				if (sensor_info->current_state ==
						SAHPI_ES_UPPER_CRIT) {
					/* Generate OK to CAUTION thermal
                                         * event
					 */
					oa_soap_gen_sen_evt(oh_handler, rpt,
							    rdr, assert_state,
							    trigger_reading,
							    trigger_threshold);
					/* Update the assert state and trigger
					 * threshold values
					 */
					assert_state =
						OA_SOAP_TEMP_CAUT_CRIT;
					trigger_threshold =
						sensor_info->threshold.UpMajor.
							Value.SensorFloat64;
				}
				break;
			default:
				err("Unrecognized sensor class %d "
				    "is detected", sensor_class);
				/* Release the node->data */
				g_free(node->data);
				continue;
		 }

		/* Generate the sensor event */
		oa_soap_gen_sen_evt(oh_handler, rpt, rdr, assert_state,
				    trigger_reading, trigger_threshold);

		/* If the operational status has failed, raise the resource
		 * failed event
		 */
		if (sensor_num == OA_SOAP_SEN_OPER_STATUS)
			oa_soap_gen_res_evt(oh_handler, rpt,
					    OA_SOAP_SEN_ASSERT_TRUE);
		/* Release the node->data */
		g_free(node->data);
	} /* End of while loop */

	/* Release the assert_sensor_list */
	g_slist_free(assert_sensor_list);

	return SA_OK;
}

/**
 * oa_soap_get_bld_thrm_sen_data
 *      @oh_handler	: Pointer to openhpi handler
 *      @response	: bladeThermalInfoArrayResponse response
 *      @bld_thrm_info	: pointer to the bladeThermalInfo
 *
 * Purpose:
 *	Retrieve the correct instance of bladeThermalInfo structure instance 
 *      from bladeThermalInfoArrayResponse response
 *
 * Detailed Description:
 *       NA
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 **/
SaErrorT oa_soap_get_bld_thrm_sen_data(SaHpiSensorNumT sen_num,
				       struct bladeThermalInfoArrayResponse 
								response,
				       struct bladeThermalInfo *bld_thrm_info)
{
	SaHpiInt32T sen_delta_num = 0;
	struct bladeThermalInfo blade_thermal_info;
	SaHpiInt32T index = -1, i;

	if (bld_thrm_info == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	/* "getBladeThermalInfoArray" response contains multiple instances
	 * of bladeThermalInfo. It is required to map the correct instance
	 * of the getBladeThermalInfo structure to the sensor number whose 
	 * reading is requested. 
	 * This mapping is achieved as follows:
	 * Based on the sensor number, sensor base number and difference of the
	 * sensor number w.r.t. sensor base number is determined(sen_delta_num).
	 * This sen_delta_num helps determine the location of bladeThermalInfo
	 * structure instance in the response required for sensor data.
	 */
	if (sen_num != OA_SOAP_SEN_TEMP_STATUS) {
		sen_delta_num = sen_num - oa_soap_bld_thrm_sen_base_arr
					[sen_num - OA_SOAP_BLD_THRM_SEN_START];
	}

	/* As per discovery, mapping between the bladeThermalInfo response and 
	 * the sensor number can be be achieved based on the description string
	 * in response and comment field in the sensor rdr.
	 * Sometimes the comment field in sensor rdr may not have matching 
	 * substring in soap response. 
	 * Hence map the sensor rdr comment field to the standard string listed
	 * in oa_soap_thermal_sensor_string array. as it is assumed that the 
	 * strings list in this array will match the description in response
	 * For example:
	 * The comment field of the system zone sensor
	 * is "System Zone thermal status" and if the description field of 
	 * bladeThermalInfo structure is "System Zone", then it is possible to 
	 * achieve mapping between the response and sensor rdr.
	 * But if the description field of bladeThermalInfo contains 
	 * "System Chassis", then it is difficult to achieve the mapping
	 * between bladeThermalInfo structure instance to any particular sensor.
	 */
	for (i = 0; i <OA_SOAP_MAX_THRM_SEN; i++) {
		if ((strstr(oa_soap_sen_arr[sen_num].comment,
			    oa_soap_thermal_sensor_string[i]))) {
			index = i;
			break;
		}
	}

	while (response.bladeThermalInfoArray) {
		soap_bladeThermalInfo(response.bladeThermalInfoArray,
				      &blade_thermal_info);
		if (strstr(blade_thermal_info.description,
			   oa_soap_thermal_sensor_string[index])) {
			/* Return the matching bladeThermalInfo structure
			 * instance
			 */
			if (sen_delta_num == 0) {
				memcpy(bld_thrm_info, &blade_thermal_info,
				       sizeof(struct bladeThermalInfo));
				break;
			}
			sen_delta_num--;
		}
		response.bladeThermalInfoArray = soap_next_node(
							response.
							bladeThermalInfoArray);
	}
	return SA_OK;
}

void * oh_get_sensor_reading (void *, SaHpiResourceIdT,
                              SaHpiSensorNumT,
                              SaHpiSensorReadingT *,
                              SaHpiEventStateT    *)
                __attribute__ ((weak, alias("oa_soap_get_sensor_reading")));

void * oh_get_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("oa_soap_get_sensor_thresholds")));

void * oh_set_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 const SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("oa_soap_set_sensor_thresholds")));

void * oh_get_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT *)
                __attribute__ ((weak, alias("oa_soap_get_sensor_enable")));

void * oh_set_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT)
                __attribute__ ((weak, alias("oa_soap_set_sensor_enable")));

void * oh_get_sensor_event_enables (void *, SaHpiResourceIdT,
                                    SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak,
                                alias("oa_soap_get_sensor_event_enable")));

void * oh_set_sensor_event_enables (void *,
                                    SaHpiResourceIdT id,
                                    SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak,
                                alias("oa_soap_set_sensor_event_enable")));

void * oh_get_sensor_event_masks (void *,
                                  SaHpiResourceIdT,
                                  SaHpiSensorNumT,
                                  SaHpiEventStateT *,
                                  SaHpiEventStateT *)
                __attribute__ ((weak, alias("oa_soap_get_sensor_event_masks")));

void * oh_set_sensor_event_masks (void *,
                                  SaHpiResourceIdT,
                                  SaHpiSensorNumT,
                                  SaHpiSensorEventMaskActionT,
                                  SaHpiEventStateT,
                                  SaHpiEventStateT)
                __attribute__ ((weak, alias("oa_soap_set_sensor_event_masks")));
