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
 *
 *      build_enclosure_thermal_sensor_rdr()- Gets the thermal information of
 *                                            enclosure and creates
 *                                            sensor rdr
 *
 *      build_oa_thermal_sensor_rdr()   - Gets the thermal information of
 *                                        Onboard Administrator and creates
 *                                        sensor rdr
 *
 *      build_server_thermal_sensor_rdr()- Gets the thermal information of
 *                                         server and creates sensor rdr
 *
 *      build_server_power_sensor_rdr() - Gets the power information of server
 *                                        and creates sensor rdr
 *
 *      build_interconnect_thermal_sensor_rdr()- Gets the thermal information
 *                                               of interconnect and creates
 *                                               sensor rdr
 *
 *      build_fan_speed_sensor()        - Gets the speed of the fan
 *                                        and creates sensor rdr
 *
 *      build_fan_power_sensor_rdr()    - Gets the power information of fan and
 *                                        creates sensor rdr
 *
 *      build_ps_power_sensor_rdr()     - Gets the power information of
 *                                        power supply and creates sensor rdr
 *
 *      build_ps_subsystem_in_power_sensor_rdr()- Gets the input power
 *                                                information of power subsystem
 *                                                and creates sensor rdr
 *
 *      build_ps_subsystem_out_power_sensor_rdr()- Gets the output power
 *                                                 information of power
 *                                                 subsystem and creates
 *                                                 sensor rdr
 *
 *      build_ps_subsystem_power_consumed_sensor_rdr()- Gets the consumed power
 *                                                      information of power
 *                                                      subsystem and creates
 *                                                      sensor rdr
 *
 *      build_ps_subsystem_power_capacity_sensor_rdr()- Gets the power capacity
 *                                                      information of power
 *                                                      subsystem and creates
 *                                                      sensor rdr
 *
 *      build_thermal_sensor_info()     - Builds the data information for the
 *                                        thermal sensor rdr
 *
 *      build_fan_speed_sensor_info()   - Builds the data information for the
 *                                        fan speed sensor rdr
 *
 *      build_ps_sensor_info()          - Builds the data information for the
 *                                        powersupply power sensor rdr
 *
 *      build_power_sensor_info()       - Builds the data information for the
 *                                        non thermal sensor rdr
 *
 *      update_sensor_rdr()             - Returns current status of the sensor
 *                                        RDR from resource
 *
 *      update_ps_subsystem_sensor_rdr()- Builds and generates the sensor enable
 *                                        event
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
 */

#include "oa_soap_sensor.h"

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

        /* Check whether sensor is enabled for reading */
        if (sensor_info->sensor_enable == SAHPI_FALSE) {
                err("Sensor not enabled for reading");
                return(SA_ERR_HPI_INVALID_REQUEST);
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
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
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

        if (oh_handler == NULL) {
                err("Invalid paramters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (oh_lookup_sensoreventmaskaction(action) == NULL) {
                err("Invalid action");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if ((assert != 0) && (deassert != 0)) {
                if (assert & ~(OA_SOAP_STM_VALID_MASK)) {
                        err("Assert mask is not valid");
                        return SA_ERR_HPI_INVALID_DATA;
                }
                if (deassert & ~(OA_SOAP_STM_VALID_MASK)) {
                        err("Deassert mask is not valid");
                        return SA_ERR_HPI_INVALID_DATA;
                }
        } else {
                err("Assert/Deassert mask is not valid");
                return SA_ERR_HPI_INVALID_DATA;
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
        if (rdr->RdrTypeUnion.SensorRec.EventCtrl == SAHPI_SEC_PER_EVENT) {
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

                orig_assert_mask = sensor_info->assert_mask;
                orig_deassert_mask = sensor_info->deassert_mask;

                /* Based on the action type, the bits in assert/deassert mask
                 * are set or cleared
                 */
                if (action == SAHPI_SENS_ADD_EVENTS_TO_MASKS) {
                        sensor_info->assert_mask = sensor_info->assert_mask |
                                                   assert;

                        if (rpt->ResourceCapabilities &
                            SAHPI_CAPABILITY_EVT_DEASSERTS) {
                                sensor_info->deassert_mask =
                                        sensor_info->assert_mask;
                        } else {
                                sensor_info->deassert_mask =
                                        sensor_info->deassert_mask | deassert;
                        }
                } else if (action == SAHPI_SENS_REMOVE_EVENTS_FROM_MASKS) {
                        sensor_info->assert_mask = sensor_info->assert_mask &
                                                   ~(assert);
                        if (rpt->ResourceCapabilities &
                            SAHPI_CAPABILITY_EVT_DEASSERTS) {
                                sensor_info->deassert_mask =
                                        sensor_info->assert_mask;
                        } else {
                                sensor_info->deassert_mask =
                                        sensor_info->deassert_mask &
                                        ~(deassert);
                        }
                }

                if (sensor_info->assert_mask != orig_assert_mask) {
                        /* If the assert mask has change, raise a
                         * "sensor enable event" to intimate the change to
                         * framework
                         */
                        rv =  generate_sensor_enable_event(oh_handler, rdr_num,
                                                           rpt, rdr,
                                                           sensor_info);
                        if (rv != SA_OK) {
                                err("Event generation failed");
                                return rv;
                        }
                } else {
                        if (! (rpt->ResourceCapabilities &
                               SAHPI_CAPABILITY_EVT_DEASSERTS) &&
                            sensor_info->deassert_mask != orig_deassert_mask) {
                                /* If the de assert mask has change, raise a
                                 * "sensor enable event" to intimate the change
                                 * to framework
                                 */
                                rv =  generate_sensor_enable_event(oh_handler,
                                                                   rdr_num,
                                                                   rpt, rdr,
                                                                   sensor_info);
                                if (rv != SA_OK) {
                                        err("Event generation failed");
                                        return rv;
                                }
                        }
                }
        } else {
                err("Sensor do no support setting event masks");
                return SA_ERR_HPI_READ_ONLY;
        }
        return SA_OK;
}

/**
 * build_enclosure_thermal_sensor_rdr
 *      @oh_handler: Handler data pointer
 *      @con: Pointer to the soap client handler
 *      @bay_number: Bay number
 *      @rdr: Rdr Structure for sensor tempature
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the thermal information of cClass enclosure and creates sensor rdr
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values and the
 *        enclosure thermal thresholds
 *      - Thresholds are enabled only for read operation since
 *        the HP BladeSystem cClass does not support setting the
 *        thermal thresholds for any resource
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 **/
SaErrorT build_enclosure_thermal_sensor_rdr(struct oh_handler_state *oh_handler,
                                            SOAP_CON *con,
                                            SaHpiInt32T bay_number,
                                            SaHpiRdrT *rdr,
                                            struct oa_soap_sensor_info
                                                **sensor_info)
{
        SaErrorT rv = SA_OK;
        char enclosure_temp_str[] = ENCLOSURE_THERMAL_STRING;
        struct getThermalInfo request;
        struct thermalInfo response;
        SaHpiBoolT event_support = SAHPI_FALSE;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Set sensor type in request to enclosure to retrieve
         * thermal information of enclosure
         */
        request.sensorType = SENSOR_TYPE_ENC;
        request.bayNumber = bay_number;

        /* Make a soap call to retrieve thermal threshold information
         * of the enclosure
         */
        rv = soap_getThermalInfo(con, &request, &response);
        if (rv != SOAP_OK) {
                err("Get thermalInfo failed for enclosure");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->oa_soap_resources.enclosure_rid;
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor RDR with default value and threshold values */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_TEMP_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_TEMPERATURE;
        /* Event is set to UNSPECIFIED, as there are no events supported on
         * the thermal information of enclosure
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_DEGREES_C;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags =
                SAHPI_SRF_MAX | SAHPI_SRF_NORMAL_MAX;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorFloat64 =
                response.criticalThreshold;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.Value.
                SensorFloat64  = response.cautionThreshold;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold =
                SAHPI_STM_UP_CRIT | SAHPI_STM_UP_MAJOR;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 0x0;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(enclosure_temp_str) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", enclosure_temp_str);

        /* Sensor specific information such as sensor_enable status,
         * sensor_event_status and threshold masks are stored in sensor_info
         * structure stored as part of the private data area of the sensor rdr
         */
        rv = build_thermal_sensor_info(response, sensor_info, event_support);
        if (rv != SA_OK) {
                err("oa_soap creating enclosure sensor information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_oa_thermal_sensor_rdr
 *      @oh_handler: Handler data pointer
 *      @con: Pointer to the soap client handler
 *      @bay_number: Onboard Administrator bay number
 *      @rdr: Rdr Structure for sensor tempature
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the thermal information of Onboard Administrator and
 *      creates sensor rdr
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values and the
 *        OA thermal thresholds
 *      - Thresholds are enabled only for read operation since
 *        the HP BladeSystem cClass does not support setting the
 *        thermal thresholds for any resource
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 **/
SaErrorT build_oa_thermal_sensor_rdr(struct oh_handler_state *oh_handler,
                                     SOAP_CON *con,
                                     SaHpiInt32T bay_number,
                                     SaHpiRdrT *rdr,
                                     struct oa_soap_sensor_info **sensor_info)
{
        SaErrorT rv = SA_OK;
        char oa_temp_str[] = OA_THERMAL_STRING;
        struct getThermalInfo request;
        struct thermalInfo response;
        SaHpiBoolT event_support = SAHPI_FALSE;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Setting sensor type in request to OA to retrieve
         * thermal information of OA
         */
        request.sensorType = SENSOR_TYPE_OA;
        request.bayNumber = bay_number;

        rv = soap_getThermalInfo(con,
                                 &request, &response);
        if (rv != SOAP_OK) {
                err("Get thermalInfo failed for OA");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.oa.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor RDR with default value and threshold values */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_TEMP_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_TEMPERATURE;
        /* Event is set to UNSPECIFIED, as there are no events supported on
         * the thermal information of OA
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_DEGREES_C;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags =
                SAHPI_SRF_MAX | SAHPI_SRF_NORMAL_MAX;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorFloat64 =
                response.criticalThreshold;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.Value.
                SensorFloat64  = response.cautionThreshold;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold =
                SAHPI_STM_UP_CRIT | SAHPI_STM_UP_MAJOR;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 0x0;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(oa_temp_str) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", oa_temp_str);

        /* Sensor specific information such as sensor_enable status,
         * sensor_event_status and threshold masks are stored in sensor_info
         * structure stored as part of the private data area of the sensor rdr
         */
        rv = build_thermal_sensor_info(response, sensor_info, event_support);
        if (rv != SA_OK) {
                err("oa_soap creating onboard sensor information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_server_thermal_sensor_rdr
 *      @oh_handler: Handler data pointer
 *      @con: Pointer to the soap client handler
 *      @bay_number: Server bay number
 *      @resource_id: Resource Id
 *      @rdr: Rdr Structure for sensor tempature
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the thermal information of server blade and creates sensor rdr
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values and the
 *        server thermal thresholds
 *      - Thresholds are enabled only for read operation since
 *        the HP BladeSystem cClass does not support setting the
 *        thermal thresholds for any resource
 *      - Server sensor rdr is enabled for rising the thermal events
 *        When upper caution and upper critical thresholds are crossed
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT build_server_thermal_sensor_rdr(struct oh_handler_state *oh_handler,
                                         SOAP_CON *con,
                                         SaHpiInt32T bay_number,
                                         SaHpiRdrT *rdr,
                                         struct oa_soap_sensor_info
                                                **sensor_info)
{
        SaErrorT rv = SA_OK;
        char server_temp_str[] = SERVER_THERMAL_STRING;
        struct getThermalInfo request;
        struct thermalInfo response;
        SaHpiBoolT event_support = SAHPI_FALSE;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Set sensor type in request to enclosure to retrieve
         * thermal information of enclosure
         */
        request.sensorType = SENSOR_TYPE_BLADE;
        request.bayNumber = bay_number;

        /* Make a soap call to retrieve thermal information of server*/
        rv = soap_getThermalInfo(con,
                                 &request, &response);
        if (rv != SOAP_OK) {
                err("Get thermalInfo failed for server");
                return SA_ERR_HPI_INTERNAL_ERROR;
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
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor RDR with default value and threshold values */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_TEMP_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_TEMPERATURE;
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl = SAHPI_SEC_PER_EVENT;
        rdr->RdrTypeUnion.SensorRec.Events =
                SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_DEGREES_C;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags =
                SAHPI_SRF_MAX | SAHPI_SRF_NORMAL_MAX;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorFloat64 =
                response.criticalThreshold;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.Value.
                SensorFloat64  = response.cautionThreshold;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold =
                SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 0x0;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(server_temp_str) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", server_temp_str);

        event_support = SAHPI_TRUE;

        /* Sensor specific information such as sensor_enable status,
         * sensor_event_status and threshold masks are stored in sensor_info
         * structure stored as part of the private data area of the sensor rdr
         */
        rv = build_thermal_sensor_info(response, sensor_info, event_support);
        if (rv != SA_OK) {
                err("oa_soap creating server thermal sensor "
                    "information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_server_power_sensor_rdr
 *      @oh_handler: Handler data pointer
 *      @con: Pointer to the soap client handler
 *      @bay_number: Server bay number
 *      @resource_id: Resource Id
 *      @rdr: Rdr Structure for sensor power
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the power information of server blade and creates sensor rdr
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values
 *      - Threshold support is not provided for power consumption since
 *        HP BladeSystem cClass does have not any threshold limits for
 *        power consumption by server
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 **/
SaErrorT build_server_power_sensor_rdr(struct oh_handler_state *oh_handler,
                                       SOAP_CON *con,
                                       SaHpiInt32T bay_number,
                                       SaHpiRdrT *rdr,
                                       struct oa_soap_sensor_info **sensor_info)
{
        SaErrorT rv = SA_OK;
        char server_power_str[] = SERVER_POWER_STRING;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameter.");
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
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor rdr with default values */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_POWER_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_POWER_SUPPLY;
        /* Event category set and events are to UNSPECIFIED, as there are
         * no events supported on power consumption by server
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_WATTS;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = 0;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(server_power_str) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", server_power_str);

        /* Sensor specific information such as sensor_enable status,
         * sensor_event_status and threshold masks are stored in sensor_info
         * structure stored as part of the private data area of the sensor rdr
         */
        rv = build_power_sensor_info(sensor_info);
        if (rv != SA_OK) {
                err("oa_soap creating server power sensor information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_interconnect_thermal_sensor_rdr
 *      @oh_handler: Handler data pointer
 *      @con: Pointer to the soap client handler
 *      @bay_number: Interconnect bay number
 *      @resource_id: Resource Id
 *      @rdr: Rdr Structure for sensor tempature
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the thermal information of interconnect blade and
 *      creates sensor rdr
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values and the
 *        interconnect thermal thresholds.
 *      - Thresholds are enabled only for read operation since
 *        the HP BladeSystem cClass does not support setting the
 *        thermal thresholds for any resource
 *      - Interconnect thermal sensor rdr is enabled for rising the thermal
 *        events only when upper caution and upper critical thresholds are
 *        crossed
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 **/
SaErrorT build_interconnect_thermal_sensor_rdr(struct oh_handler_state
                                                       *oh_handler,
                                               SOAP_CON *con,
                                               SaHpiInt32T bay_number,
                                               SaHpiRdrT *rdr,
                                               struct oa_soap_sensor_info
                                                       **sensor_info)
{
        SaErrorT rv = SA_OK;
        char interconnect_temp_str[] = INTERCONNECT_THERMAL_STRING;
        struct getThermalInfo request;
        struct thermalInfo response;
        SaHpiBoolT event_support = SAHPI_FALSE;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Set sensor type in request to interconnect to retrieve
         * thermal information of enclosure
         */
        request.sensorType = SENSOR_TYPE_INTERCONNECT;
        request.bayNumber = bay_number;

        /* Make a soap call to retrieve thermal information of the interconnect
         */
        rv = soap_getThermalInfo(con,
                                 &request, &response);
        if (rv != SOAP_OK) {
                err("Get thermalInfo failed for interconnect");
                return SA_ERR_HPI_INTERNAL_ERROR;
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

        rdr->Entity = rpt->ResourceEntity;
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_TEMP_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_TEMPERATURE;
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_PER_EVENT;
        rdr->RdrTypeUnion.SensorRec.Events =
                SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_DEGREES_C;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags =
                SAHPI_SRF_MAX | SAHPI_SRF_NORMAL_MAX;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorFloat64 =
                response.criticalThreshold;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.NormalMax.Value.
                SensorFloat64 = response.cautionThreshold;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        /* Threshold support */
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold =
                SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 0x0;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(interconnect_temp_str) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", interconnect_temp_str);

        event_support = SAHPI_TRUE;
        /* Sensor specific information such as sensor_enable status,
         * sensor_event_status and threshold masks are stored in sensor_info
         * structure stored as part of the private data area of the sensor rdr
         */
        rv = build_thermal_sensor_info (response, sensor_info, event_support);
        if (rv != SA_OK) {
                err("oa_soap creating enclosure sensor information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_fan_speed_sensor:
 *      @oh_handler: Handler data pointer
 *      @con: Pointer to the soap client handler
 *      @bay_number: Fan bay number
 *      @rdr: Rdr Structure for sensor tempature
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the speed information of the fan and creates sensor rdr
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values and the
 *        fan speed thresholds
 *      - Thresholds are enabled only for read operation since
 *        the HP BladeSystem cClass does not support setting the
 *        speed thresholds
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 **/
SaErrorT build_fan_speed_sensor_rdr(struct oh_handler_state *oh_handler,
                                    SOAP_CON *con,
                                    SaHpiInt32T bay_number,
                                    SaHpiRdrT *rdr,
                                    struct oa_soap_sensor_info **sensor_info)
{
        SaErrorT rv = SA_OK;
        char fan_speed_str[] = FAN_SPEED_STRING;
        struct getFanInfo request;
        struct fanInfo response;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        request.bayNumber = bay_number;

        rv = soap_getFanInfo(con,
                             &request, &response);
        if (rv != SOAP_OK) {
                err("Get FanInfo failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.fan.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor rdr with default values and the fan speed
         * thresholds
         */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_FAN_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_COOLING_DEVICE;
        /* Event is set to UNSPECIFIED,
         * as there are no events supported on the thermal information of
         * enclosure
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_RPM;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = SAHPI_SRF_MAX |
                                                             SAHPI_SRF_MIN ;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorFloat64 =
                response.maxFanSpeed;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Min.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Min.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Min.Value.SensorFloat64 =
                response.lowLimitFanSpeed;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold =
                SAHPI_STM_UP_CRIT | SAHPI_STM_LOW_CRIT;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 0x0;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(fan_speed_str) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", fan_speed_str);
        /* Sensor specific information such as sensor_enable status,
         * sensor_event_status and threshold masks are stored in sensor_info
         * structure stored as part of the private data area of the sensor rdr
         */
        rv = build_fan_speed_sensor_info(response, sensor_info);
        if (rv != SA_OK) {
                err("oa_soap creating fan speed sensor information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_fan_power_sensor_rdr
 *      @oh_handler: Handler data pointer.
 *      @con: Pointer to the oa_info handler
 *      @bay_number: Fan bay number.
 *      @rdr: Rdr Structure for sensor tempature.
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the power consumption information of fan and creates sensor rdr.
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values.
 *      - Threshold support is not provided for power sensor rdr since
 *        HP BladeSystem cClass does have not any threshold limits for
 *        power consumption by fan.
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT build_fan_power_sensor_rdr(struct oh_handler_state *oh_handler,
                                    SOAP_CON *con,
                                    SaHpiInt32T bay_number,
                                    SaHpiRdrT *rdr,
                                    struct oa_soap_sensor_info **sensor_info)
{
        SaErrorT rv = 0;
        char fan_power_str[] = FAN_POWER_STRING;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.fan.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor rdr with default values */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_POWER_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_POWER_SUPPLY;
        /* Event is set to UNSPECIFIED,
         * as there are no events supported on the power information of fan
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_WATTS;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(fan_power_str) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", fan_power_str);

        /* Sensor specific information such as sensor_enable status,
         * sensor_event_status and threshold masks are stored in sensor_info
         * structure stored as part of the private data area of the sensor rdr
         */
        rv = build_power_sensor_info(sensor_info);
        if (rv != SA_OK) {
                err("oa_soap creating fan power sensor information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_ps_power_sensor_rdr
 *      @oh_handler: Handler data pointer
 *      @con: Pointer to the oa_info handler
 *      @bay_number: Power supply bay number
 *      @rdr: Rdr Structure for sensor tempature
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the power information of power supply and
 *      creates sensor rdr and uploads to resource
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values and the
 *        power capacity thresholds.
 *      - Thresholds are enabled only for read operation since
 *        the HP BladeSystem cClass does not support setting the
 *        power capacity thresholds.
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 **/
SaErrorT build_ps_power_sensor_rdr(struct oh_handler_state *oh_handler,
                                   SOAP_CON *con,
                                   SaHpiInt32T bay_number,
                                   SaHpiRdrT *rdr,
                                   struct oa_soap_sensor_info **sensor_info)
{
        SaErrorT rv = SA_OK;
        char ps_power_str[] = POWER_SUPPLY_POWER_STRING;
        struct getPowerSupplyInfo request;
        struct powerSupplyInfo response;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        request.bayNumber = bay_number;

        rv = soap_getPowerSupplyInfo(con, &request,
                                  &response);
        if (rv != SOAP_OK) {
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.ps_unit.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor rdr with default values and the
         * power capacity threshold.
         */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_POWER_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_POWER_SUPPLY;
        /* Event is set to UNSPECIFIED, as there are no events supported
         * on the thermal information of enclosure
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_WATTS;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = SAHPI_SRF_MAX ;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.IsSupported =
                SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorFloat64 =
                response.capacity;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold = SAHPI_STM_UP_CRIT;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 0x0;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(ps_power_str) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", ps_power_str);

        /* Sensor specific information such as sensor_enable status,
         * sensor_event_status and threshold masks are stored in sensor_info
         * structure stored as part of the private data area of the sensor rdr
         */
        rv = build_ps_sensor_info(response, sensor_info);
        if (rv != SA_OK) {
                err("oa_soap creating fan power sensor information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_ps_subsystem_in_power_sensor_rdr
 *      @oh_handler: Handler data pointer.
 *      @rdr: Pointer to RDR Structure.
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the input power information of power subsystem and
 *      creates power sensor RDR
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values.
 *      - Threshold support is not provided for any parameters of power
 *        subsystem since HP BladeSystem cClass does have not any threshold
 *        limits for operation parameters of the power subsystem.
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT build_ps_subsystem_input_power_sensor_rdr(struct oh_handler_state
                                                           *oh_handler,
                                                   SaHpiRdrT *rdr,
                                                   struct oa_soap_sensor_info
                                                           **sensor_info)
{
        SaErrorT rv = SA_OK;
        char sensor_name[] = POWER_SUBSYSTEM_IN_POWER;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->oa_soap_resources.power_subsystem_rid;
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor RDR with default values */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_IN_POWER_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_POWER_SUPPLY;
        /* Event category set and events are to UNSPECIFIED, as there are
         * no events supported on input power of power subsystem
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_WATTS;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = 0;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(sensor_name) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", sensor_name);

        rv = build_power_sensor_info(sensor_info);
        if (rv != SA_OK) {
                err("oa_soap creating ps subsystem input power sensor "
                    "information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_ps_subsystem_out_power_sensor_rdr
 *      @oh_handler: Handler data pointer.
 *      @rdr: Pointer to RDR Structure.
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the output power information of power subsystem and
 *      creates power sensor RDR
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values.
 *      - Threshold support is not provided for any parameters of power
 *        subsystem since HP BladeSystem cClass does have not any threshold
 *        limits for operation parameters of the power subsystem.
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 **/
SaErrorT build_ps_subsystem_output_power_sensor_rdr(struct oh_handler_state
                                                            *oh_handler,
                                                    SaHpiRdrT *rdr,
                                                    struct oa_soap_sensor_info
                                                            **sensor_info)
{
        SaErrorT rv = SA_OK;
        char sensor_name[] = POWER_SUBSYSTEM_OUT_POWER;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->oa_soap_resources.power_subsystem_rid;
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor RDR with default values */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_OUT_POWER_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_POWER_SUPPLY;
        /* Event category set and events are to UNSPECIFIED, as there are
         * no events supported on output power of power subsystem
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_WATTS;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = 0;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(sensor_name) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", sensor_name);

        rv = build_power_sensor_info(sensor_info);
        if (rv != SA_OK) {
                err("oa_soap creating ps subsystem output power sensor "
                    "info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_ps_subsystem_power_consumed_sensor_rdr
 *      @oh_handler: Handler data pointer.
 *      @rdr: Pointer to RDR Structure.
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the power consumtion information of power subsystem and
 *      creates power sensor RDR
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values.
 *      - Threshold support is not provided for any parameters of power
 *        subsystem since HP BladeSystem cClass does have not any threshold
 *        limits for operation parameters of the power subsystem.
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT build_ps_subsystem_power_consumed_sensor_rdr(struct oh_handler_state
                                                              *oh_handler,
                                                      SaHpiRdrT *rdr,
                                                      struct oa_soap_sensor_info
                                                              **sensor_info)
{
        SaErrorT rv = SA_OK;
        char sensor_name[] = POWER_SUBSYSTEM_POWER_CONSUMED;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->oa_soap_resources.power_subsystem_rid;
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor RDR with default values */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_POWER_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_POWER_SUPPLY;
        /* Event category and events are set to UNSPECIFIED, as there are no
         * events support on power consumption by power subsystem
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_WATTS;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = 0;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(sensor_name) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", sensor_name);

        rv = build_power_sensor_info(sensor_info);
        if (rv != SA_OK) {
                err("oa_soap creating ps subsystem power consumed sensor "
                    "info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_ps_subsystem_power_capacity_sensor_rdr
 *      @oh_handler: Handler data pointer.
 *      @rdr: Pointer to RDR Structure.
 *      @sensor_info: Pointer to the sensor information structure
 *
 * Purpose:
 *      Gets the power capacity information of power subsystem and
 *      creates power sensor RDR
 *
 * Detailed Description:
 *      - Populates the sensor rdr with default values.
 *      - Threshold support is not provided for any parameters of power
 *        subsystem since HP BladeSystem cClass does have not any threshold
 *        limits for operation parameters of the power subsystem.
 *      - All sensor specific information is stored in private structure
 *        which is stored as part of the private data area of sensor RDR
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 **/
SaErrorT build_ps_subsystem_power_capacity_sensor_rdr(struct oh_handler_state
                                                              *oh_handler,
                                                      SaHpiRdrT *rdr,
                                                      struct oa_soap_sensor_info
                                                              **sensor_info)
{
        SaErrorT rv = SA_OK;
        char sensor_name[] = POWER_SUBSYSTEM_POWER_CAPACITY;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->oa_soap_resources.power_subsystem_rid;
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populate the sensor RDR with default values */
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec.Num = OA_SOAP_RES_SEN_POWER_CAPACITY_NUM;
        rdr->RdrTypeUnion.SensorRec.Type = SAHPI_POWER_SUPPLY;
        /* Event category and events are set to UNSPECIFIED, as there are no
         * events supported on power capacity of power subsystem
         */
        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.EnableCtrl  = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.EventCtrl  = SAHPI_SEC_READ_ONLY;
        rdr->RdrTypeUnion.SensorRec.Events  = SAHPI_ES_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_WATTS;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
                SAHPI_SU_UNSPECIFIED;
        rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
        rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = 0;
        rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor  = 0;
        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(sensor_name) + 1;
        snprintf((char *)rdr->IdString.Data, rdr->IdString.DataLength,
                 "%s", sensor_name);

        rv = build_power_sensor_info(sensor_info);
        if (rv != SA_OK) {
                err("oa_soap creating ps subsystem power capacity sensor "
                    "information failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        return SA_OK;
}

/**
 * build_thermal_sensor_info
 *      @response: Pointer to response structure thermal info.
 *      @sensor: Pointer to sensor RDR's data
 *      @event_flag: Flag indicating the event support
 *
 * Purpose:
 *      Builds the data information for the thermal sensor RDR
 *
 * Detailed Description:
 *      - Populates the sensor info structure with default values and
 *        provided threshold details.
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_OUT_OF_MEMORY - oa_soap plugin has encountered out of
 *                                 memory error
 **/
SaErrorT build_thermal_sensor_info(struct thermalInfo response,
                                   struct oa_soap_sensor_info **sensor_info,
                                   SaHpiBoolT event_flag)
{
        struct oa_soap_sensor_info *local_sensor_info = NULL;

        if (sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        local_sensor_info = (struct oa_soap_sensor_info*)
                g_malloc0(sizeof(struct oa_soap_sensor_info));
        if (local_sensor_info == NULL) {
                err("oa_soap out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
        local_sensor_info->sensor_enable = SAHPI_TRUE;

        /* In HP BladeSystem cClass:
         * 1.Thermal events are supported only for server blade and interconnect
         *   hence event_flag is TRUE only for server and interconnect.
         * 2.The supported threshold for thermal data are
         *   UPPER CRITICAL (CRITICAL TEMPERATURE) and
         *   UPPER MAJOR (CAUTION TEMPERATURE) hence sensor rdr is enabled
         *   only for rising the thermal events when upper caution and
         *   upper critical thresholds are crossed
         */
        if (event_flag == SAHPI_TRUE) {
                local_sensor_info->event_enable = SAHPI_TRUE;
                local_sensor_info->assert_mask = SAHPI_ES_UPPER_CRIT |
                                                 SAHPI_ES_UPPER_MAJOR;
                local_sensor_info->deassert_mask = SAHPI_ES_UPPER_CRIT |
                                                 SAHPI_ES_UPPER_MAJOR;
        } else {
               local_sensor_info->event_enable = SAHPI_FALSE;
               local_sensor_info->assert_mask = OA_SOAP_STM_UNSPECIFED;
               local_sensor_info->deassert_mask = OA_SOAP_STM_UNSPECIFED;
        }

        local_sensor_info->threshold.UpCritical.IsSupported = SAHPI_TRUE;
        local_sensor_info->threshold.UpCritical.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        local_sensor_info->threshold.UpCritical.Value.SensorFloat64 =
                response.criticalThreshold;

        local_sensor_info->threshold.UpMajor.IsSupported = SAHPI_TRUE;
        local_sensor_info->threshold.UpMajor.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        local_sensor_info->threshold.UpMajor.Value.SensorFloat64 =
                response.cautionThreshold;

        *sensor_info = local_sensor_info;
        return SA_OK;
}

/**
 * build_fan_speed_sensor_info
 *      @response: Pointer to response structure of fan info.
 *      @sensor: Pointer to sensor RDR's data
 *
 * Purpose:
 *      Builds the data information for the fan speed sensor RDR
 *
 * Detailed Description:
 *      - Populates the sensor info structure with default values and
 *        provided threshold details.
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_OUT_OF_MEMORY - oa_soap plugin has encountered out of
 *                                 memory error
 **/
SaErrorT build_fan_speed_sensor_info(struct fanInfo response,
                                     struct oa_soap_sensor_info **sensor_info)
{
        struct oa_soap_sensor_info *local_sensor_info = NULL;

        if (sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        local_sensor_info = (struct oa_soap_sensor_info*)
                g_malloc0(sizeof(struct oa_soap_sensor_info));
        if (local_sensor_info == NULL) {
                err("oa_soap out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
        local_sensor_info->sensor_enable = SAHPI_TRUE;
        local_sensor_info->event_enable = SAHPI_FALSE;
        local_sensor_info->assert_mask = OA_SOAP_STM_UNSPECIFED;
        local_sensor_info->deassert_mask = OA_SOAP_STM_UNSPECIFED;

        local_sensor_info->threshold.UpCritical.IsSupported = SAHPI_TRUE;
        local_sensor_info->threshold.UpCritical.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        local_sensor_info->threshold.UpCritical.Value.SensorFloat64 =
                response.maxFanSpeed;

        local_sensor_info->threshold.LowCritical.IsSupported = SAHPI_TRUE;
        local_sensor_info->threshold.LowCritical.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        local_sensor_info->threshold.LowCritical.Value.SensorFloat64 =
                response.lowLimitFanSpeed;
        *sensor_info = local_sensor_info;
        return SA_OK;
}

/**
 * build_ps_sensor_info
 *      @response: Pointer to response structure of fan info.
 *      @sensor: Pointer to sensor RDR's data
 *
 * Purpose:
 *      Builds the data information for the powersupply power sensor RDR
 *
 * Detailed Description:
 *      - Populates the sensor info structure with default values and
 *        provided threshold details.
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_OUT_OF_MEMORY - oa_soap plugin has encountered out of
 *                                 memory error
 **/
SaErrorT build_ps_sensor_info(struct powerSupplyInfo response,
                              struct oa_soap_sensor_info **sensor_info)
{
        struct oa_soap_sensor_info *local_sensor_info = NULL;

        if (sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        local_sensor_info = (struct oa_soap_sensor_info*)
                g_malloc0(sizeof(struct oa_soap_sensor_info));
        if (local_sensor_info == NULL) {
                err("oa_soap out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
        local_sensor_info->sensor_enable = SAHPI_TRUE;
        local_sensor_info->event_enable = SAHPI_FALSE;
        local_sensor_info->assert_mask = OA_SOAP_STM_UNSPECIFED;
        local_sensor_info->deassert_mask = OA_SOAP_STM_UNSPECIFED;

        local_sensor_info->threshold.UpCritical.IsSupported = SAHPI_TRUE;
        local_sensor_info->threshold.UpCritical.Type =
                SAHPI_SENSOR_READING_TYPE_FLOAT64;
        local_sensor_info->threshold.UpCritical.Value.SensorFloat64 =
                response.capacity;
        *sensor_info = local_sensor_info;
        return SA_OK;
}

/**
 * build_power_sensor_info
 *      @sensor: Pointer to sensor RDR's data
 *
 * Purpose:
 *      Builds the data information for the non thermal sensor RDR
 *
 * Detailed Description:
 *      - Populates the sensor info structure with default values.
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_OUT_OF_MEMORY - oa_soap plugin has encountered out of
 *                                 memory error
 **/
SaErrorT build_power_sensor_info(struct oa_soap_sensor_info **sensor_info)
{
        struct oa_soap_sensor_info *local_sensor_info = NULL;

        if (sensor_info == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        local_sensor_info = (struct oa_soap_sensor_info*)
                g_malloc0(sizeof(struct oa_soap_sensor_info));
        if (local_sensor_info == NULL) {
                err("oa_soap out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
        local_sensor_info->sensor_enable = SAHPI_TRUE;
        local_sensor_info->event_enable = SAHPI_FALSE;
        local_sensor_info->assert_mask = SAHPI_ES_UNSPECIFIED;
        local_sensor_info->deassert_mask = SAHPI_ES_UNSPECIFIED;

        *sensor_info = local_sensor_info;
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
        thermal_request.bayNumber = server_status_request.bayNumber = location;
        fan_request.bayNumber = power_supply_request.bayNumber = location;

        /* Getting the current reading of the sensor directly from resource
         * using a soap call
         */
        switch (rpt->ResourceEntity.Entry[0].EntityType) {
                case (SAHPI_ENT_SYSTEM_BLADE):
                case (SAHPI_ENT_IO_BLADE):
                case (SAHPI_ENT_DISK_BLADE):
                        if (rdr_num == OA_SOAP_RES_SEN_TEMP_NUM) {
                                thermal_request.sensorType = SENSOR_TYPE_BLADE;

                                /* Fetching current thermal reading of the
                                 * server blade in the specified bay number
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
                        }
                        else if (rdr_num == OA_SOAP_RES_SEN_POWER_NUM) {

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
                case (SAHPI_ENT_COOLING_DEVICE):

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
                        if (rdr_num == OA_SOAP_RES_SEN_FAN_NUM) {
                                sensor_data->data.Value.SensorFloat64 =
                                        fan_response.maxFanSpeed;
                        } else if (rdr_num == OA_SOAP_RES_SEN_POWER_NUM) {
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
                        if (rdr_num == OA_SOAP_RES_SEN_IN_POWER_NUM) {
                                sensor_data->data.Value.SensorFloat64 =
                                        ps_response.inputPowerVa;
                        }
                        if (rdr_num == OA_SOAP_RES_SEN_OUT_POWER_NUM) {
                                sensor_data->data.Value.SensorFloat64 =
                                        ps_response.outputPower;
                        }
                        if (rdr_num == OA_SOAP_RES_SEN_POWER_NUM) {
                                sensor_data->data.Value.SensorFloat64 =
                                        ps_response.powerConsumed;
                        }
                        if (rdr_num == OA_SOAP_RES_SEN_POWER_CAPACITY_NUM) {
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
 * update_ps_subsystem_sensor_rdr
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Sensor rdr number
 *      @rdr: Pointer to rdr Structure
 *
 * Purpose:
 *      Updates the power subsystem sensor rdr with latest status from hardware
 *
 * Detailed Description:
 *      - Fetches current reading of the power subsystem sensor from the
 *        resource by soap call and returns reading in sensor data.
 *
 * Return values:
 *      SA_OK - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error.
 *      SA_ERR_HPI_INVALID_DATA - Invalid rdr number.
 **/
SaErrorT update_ps_subsystem_sensor_rdr(struct oh_handler_state *oh_handler,
                                        SaHpiResourceIdT resource_id,
                                        SaHpiSensorNumT rdr_num,
                                        SaHpiRdrT *rdr)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler = NULL;
        struct oa_soap_handler *oa_handler = NULL;
        struct oa_soap_sensor_info *sensor_info = NULL;

        if (oh_handler == NULL || rdr == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;

        switch (rdr_num) {
                case OA_SOAP_RES_SEN_IN_POWER_NUM:
                        rv = build_ps_subsystem_input_power_sensor_rdr(
                                handler, rdr, &sensor_info);
                        break;
                case OA_SOAP_RES_SEN_OUT_POWER_NUM:
                        rv = build_ps_subsystem_output_power_sensor_rdr(
                                handler, rdr, &sensor_info);
                        break;
                case OA_SOAP_RES_SEN_POWER_NUM:
                        rv = build_ps_subsystem_power_consumed_sensor_rdr(
                                handler, rdr, &sensor_info);
                        break;
                case OA_SOAP_RES_SEN_POWER_CAPACITY_NUM:
                        rv = build_ps_subsystem_power_capacity_sensor_rdr(
                                handler, rdr, &sensor_info);
                        break;
                default:
                        err("wrong sensor number <%d>", rdr_num);
                        return SA_ERR_HPI_INVALID_DATA;
        }
        return rv;
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
                SAHPI_TEMPERATURE;
        event.event.EventDataUnion.SensorEnableChangeEvent.EventCategory =
                SAHPI_EC_THRESHOLD;
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
        if (sensor_info->current_state == SAHPI_ES_UPPER_CRIT) {
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
                                              OA_SOAP_RES_SEN_TEMP_NUM,
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
                                              OA_SOAP_RES_SEN_TEMP_NUM,
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
