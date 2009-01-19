/*
 * Copyright (C) 2008, Hewlett-Packard Development Company, LLP
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
 *
 **/

#include "oa_soap_resources.h"

/* Array for mapping the OA enums to HPI values. -1 is assigned if the OA is not
 * supporting the enum value.
 * 
 * Please modify the array on adding new sensor class and/or changing the 
 * sensor enum values defined in oa_soap_resources.h
 */
const SaHpiInt32T oa_soap_sen_val_map_arr[OA_SOAP_MAX_SEN_CLASS]
					 [OA_SOAP_MAX_ENUM] = {
	/* OA_SOAP_OPER_CLASS. This uses the enum opStatus values as index */
	{
		SAHPI_ES_DISABLED, /* OP_STATUS_UNKNOWN */
		SAHPI_ES_DISABLED, /* OP_STATUS_OTHER */
		SAHPI_ES_ENABLED, /* OP_STATUS_OK */
		SAHPI_ES_ENABLED, /* OP_STATUS_DEGRADED */
		-1, /* OP_STATUS_STRESSED - not supported by OA */
		-1, /* OP_STATUS_PREDICTIVE_FAILURE - not supported by OA */
		-1, /* OP_STATUS_ERROR - not supported by OA */
		SAHPI_ES_DISABLED, /* OP_STATUS_NON_RECOVERABLE_ERROR */
		-1, /* OP_STATUS_STARTING - not supported by OA */
		-1, /* OP_STATUS_STOPPING - not supported by OA */
		-1, /* OP_STATUS_STOPPED - not supported by OA */
		-1, /* OP_STATUS_IN_SERVICE - not supported by OA */
		-1, /* OP_STATUS_NO_CONTACT - not supported by OA */
		-1, /* OP_STATUS_LOST_COMMUNICATION - not supported by OA */
		-1, /* OP_STATUS_ABORTED - not supported by OA */
		-1, /* OP_STATUS_DORMANT - not supported by OA */
		-1, /* OP_STATUS_SUPPORTING_ENTITY_IN_ERROR - not supported
		     * by OA
		     */
		-1, /* OP_STATUS_COMPLETED - not supported by OA */
		-1, /* OP_STATUS_POWER_MODE - not supported by OA */
		-1, /* OP_STATUS_DMTF_RESERVED - not supported by OA */
		-1, /* OP_STATUS_VENDER_RESERVED - not supported by OA */
	},
	/* OA_SOAP_PRED_FAIL_CLASS. This uses the enum opStatus values
	 * as index
	 */
	{
		SAHPI_ES_PRED_FAILURE_ASSERT, /* OP_STATUS_UNKNOWN */
		SAHPI_ES_PRED_FAILURE_ASSERT, /* OP_STATUS_OTHER */
		SAHPI_ES_PRED_FAILURE_DEASSERT, /* OP_STATUS_OK */
		SAHPI_ES_PRED_FAILURE_ASSERT, /* OP_STATUS_DEGRADED */
		-1, /* OP_STATUS_STRESSED - not supported by OA */
		-1, /* OP_STATUS_PREDICTIVE_FAILURE - not supported by OA */
		-1, /* OP_STATUS_ERROR - not supported by OA */
		SAHPI_ES_PRED_FAILURE_ASSERT,
			/* OP_STATUS_NON_RECOVERABLE_ERROR */
		-1, /* OP_STATUS_STARTING - not supported by OA */
		-1, /* OP_STATUS_STOPPING - not supported by OA */
		-1, /* OP_STATUS_STOPPED - not supported by OA */
		-1, /* OP_STATUS_IN_SERVICE - not supported by OA */
		-1, /* OP_STATUS_NO_CONTACT - not supported by OA */
		-1, /* OP_STATUS_LOST_COMMUNICATION - not supported by OA */
		-1, /* OP_STATUS_ABORTED - not supported by OA */
		-1, /* OP_STATUS_DORMANT - not supported by OA */
		-1, /* OP_STATUS_SUPPORTING_ENTITY_IN_ERROR - not supported
		     * by OA
		     */
		-1, /* OP_STATUS_COMPLETED - not supported by OA */
		-1, /* OP_STATUS_POWER_MODE - not supported by OA */
		-1, /* OP_STATUS_DMTF_RESERVED - not supported by OA */
		-1, /* OP_STATUS_VENDER_RESERVED - not supported by OA */
	},
	/* OA_SOAP_REDUND_CLASS. This uses the enum redundancy values
	 * as index
	 */
	{
		-1, /* REDUNDANCY_NO_OP - This state is returned if the
		     * device is not present. Ideally, this should never
		     * get executed.
		     */
		SAHPI_ES_REDUNDANCY_LOST, /* REDUNDANCY_UNKNOWN */
		SAHPI_ES_REDUNDANCY_LOST, /* NOT_REDUNDANT */
		SAHPI_ES_FULLY_REDUNDANT, /* REDUNDANT */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_DIAG_CLASS. This uses the enum diagnosticStatus
	 * values as index
	 */
	{
		-1, /* NOT_RELEVANT - this means that sensor is not
		     * supported. Ideally, this will never be returned.
		     * Not supported sensors will not be modelled for
		     * the resources in OA SOAP plugin
		     */
		SAHPI_ES_ENABLED, /* DIAGNOSTIC_CHECK_NOT_PERFORMED */
		SAHPI_ES_ENABLED, /* NO_ERROR */
		SAHPI_ES_DISABLED, /* ERROR */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_TEMP_CLASS sensor class. set the array contents to -1,
	 * as thermal class sensors do not utilize this mapping entry
	 */
	{
		
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_PWR_STATUS_CLASS sensor class. set the array contents to
	 * -1, as power status class sensors do not utilize this mapping entry
	 */
	{
		
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_FAN_SPEED_CLASS sensor class. set the array contents to
	 * -1, as fan speed class sensors do not utilize this mapping entry
	 */
	{
		
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_PWR_SUBSYS_CLASS sensor class. set the array contents to
	 * -1, as power subsystem class sensors do not utilize this mapping
	 * entry
	 */
	{
		
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_ENC_AGR_OPER_CLASS sensor class. This uses the enum 
	 * lcdSetupHealth values as index
	 */
	{
		SAHPI_ES_DISABLED, /* LCD_SETUP_HEALTH_UNKNOWN */
		SAHPI_ES_ENABLED, /* LCD_SETUP_HEALTH_OK */
		SAHPI_ES_ENABLED, /* LCD_SETUP_HEALTH_INFORMATIONAL */
		SAHPI_ES_ENABLED, /* LCD_SETUP_HEALTH_DEGRADED */
		SAHPI_ES_DISABLED, /* LCD_SETUP_HEALTH_FAILED */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_ENC_AGR_PRED_FAIL_CLASS sensor class. This uses the enum 
	 * lcdSetupHealth values as index
	 */
	{
		SAHPI_ES_DISABLED, /* LCD_SETUP_HEALTH_UNKNOWN */
		SAHPI_ES_ENABLED, /* LCD_SETUP_HEALTH_OK */
		SAHPI_ES_DISABLED, /* LCD_SETUP_HEALTH_INFORMATIONAL */
		SAHPI_ES_DISABLED, /* LCD_SETUP_HEALTH_DEGRADED */
		SAHPI_ES_DISABLED, /* LCD_SETUP_HEALTH_FAILED */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_BOOL_CLASS sensor class. This uses the enum 
	 * hpoa_boolean values as index. This class is used for OA link status.
	 * The sensor value 'true' means sensor state is enabled.
	 */
	{
		SAHPI_ES_DISABLED, /* false */
		SAHPI_ES_ENABLED, /* true */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_BOOL_RVRS_CLASS sensor class. This uses the enum 
	 * hpoa_boolean values as index. This class is used for interconnect CPU
	 * fault and health LED. Then sensor value 'false' means sensor state
	 * is enabled.
	 */
	{
		SAHPI_ES_ENABLED, /* false */
		SAHPI_ES_DISABLED, /* true */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_HEALTH_OPER_CLASS sensor class. This uses the enum 
	 * oa_soap_extra_data_health values as index
	 */
	{
		SAHPI_ES_DISABLED, /* HEALTH_UNKNOWN */
		SAHPI_ES_DISABLED, /* HEALTH_OTHER */
		SAHPI_ES_ENABLED, /* HEALTH_OK */
		SAHPI_ES_ENABLED, /* HEALTH_DEGRAD */
		SAHPI_ES_ENABLED, /* HEALTH_STRESSED */
		SAHPI_ES_ENABLED, /* HEALTH_PRED_FAIL */
		SAHPI_ES_DISABLED, /* HEALTH_ERROR */
		SAHPI_ES_DISABLED, /* HEALTH_NRE */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_HEALTH_PRED_FAIL_CLASS sensor class. This uses the
	 * enum oa_soap_extra_data_health values as index
	 */
	{
		SAHPI_ES_DISABLED, /* HEALTH_UNKNOWN */
		SAHPI_ES_DISABLED, /* HEALTH_OTHER */
		SAHPI_ES_ENABLED, /* HEALTH_OK */
		SAHPI_ES_DISABLED, /* HEALTH_DEGRAD */
		SAHPI_ES_DISABLED, /* HEALTH_STRESSED */
		SAHPI_ES_DISABLED, /* HEALTH_PRED_FAIL */
		SAHPI_ES_DISABLED, /* HEALTH_ERROR */
		SAHPI_ES_DISABLED, /* HEALTH_NRE */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, 
	},
};

/* Array for mapping the OA enums to OA SOAP plugin sensor assert states. 
 * -1 is assigned, if the OA is not supporting the enum value.
 * 
 * Please modify the array on adding new sensor class and/or changing the 
 * sensor enum values defined in oa_soap_resources.h
 */
const SaHpiInt32T oa_soap_sen_assert_map_arr[OA_SOAP_MAX_SEN_CLASS]
					    [OA_SOAP_MAX_ENUM] = {
	/* OA_SOAP_OPER_CLASS. This uses the enum opStatus values
	 * as index
	 */
	{
		/* uses the enum opStatus values as index */
		OA_SOAP_SEN_ASSERT_TRUE, /* OP_STATUS_UNKNOWN */
		OA_SOAP_SEN_ASSERT_TRUE, /* OP_STATUS_OTHER */
		OA_SOAP_SEN_ASSERT_FALSE, /* OP_STATUS_OK */
		OA_SOAP_SEN_ASSERT_FALSE, /* OP_STATUS_DEGRADED */
		-1, /* OP_STATUS_STRESSED - not supported by OA */
		-1, /* OP_STATUS_PREDICTIVE_FAILURE - not supported by OA */
		-1, /* OP_STATUS_ERROR - not supported by OA */
		OA_SOAP_SEN_ASSERT_TRUE, /* OP_STATUS_NON_RECOVERABLE_ERROR */
		-1, /* OP_STATUS_STARTING - not supported by OA */
		-1, /* OP_STATUS_STOPPING - not supported by OA */
		-1, /* OP_STATUS_STOPPED - not supported by OA */
		-1, /* OP_STATUS_IN_SERVICE - not supported by OA */
		-1, /* OP_STATUS_NO_CONTACT - not supported by OA */
		-1, /* OP_STATUS_LOST_COMMUNICATION - not supported by OA */
		-1, /* OP_STATUS_ABORTED - not supported by OA */
		-1, /* OP_STATUS_DORMANT - not supported by OA */
		-1, /* OP_STATUS_SUPPORTING_ENTITY_IN_ERROR - not supported
		     * by OA
		     */
		-1, /* OP_STATUS_COMPLETED - not supported by OA */
		-1, /* OP_STATUS_POWER_MODE - not supported by OA */
		-1, /* OP_STATUS_DMTF_RESERVED - not supported by OA */
		-1, /* OP_STATUS_VENDER_RESERVED - not supported by OA */
	},
	/* OA_SOAP_PRED_FAIL_CLASS. This uses the enum opStatus values
	 * as index
	 */
	{
		/* uses the enum opStatus values as index */
		OA_SOAP_SEN_ASSERT_TRUE, /* OP_STATUS_UNKNOWN */
		OA_SOAP_SEN_ASSERT_TRUE, /* OP_STATUS_OTHER */
		OA_SOAP_SEN_ASSERT_FALSE, /* OP_STATUS_OK */
		OA_SOAP_SEN_ASSERT_TRUE, /* OP_STATUS_DEGRADED */
		-1, /* OP_STATUS_STRESSED - not supported by OA */
		-1, /* OP_STATUS_PREDICTIVE_FAILURE - not supported
		     * by OA
		     */
		-1, /* OP_STATUS_ERROR - not supported by OA */
		OA_SOAP_SEN_ASSERT_TRUE, /* OP_STATUS_NON_RECOVERABLE_ERROR */
		-1, /* OP_STATUS_STARTING - not supported by OA */
		-1, /* OP_STATUS_STOPPING - not supported by OA */
		-1, /* OP_STATUS_STOPPED - not supported by OA */
		-1, /* OP_STATUS_IN_SERVICE - not supported by OA */
		-1, /* OP_STATUS_NO_CONTACT - not supported by OA */
		-1, /* OP_STATUS_LOST_COMMUNICATION - not supported
		     * by OA
		     */
		-1, /* OP_STATUS_ABORTED - not supported by OA */
		-1, /* OP_STATUS_DORMANT - not supported by OA */
		-1, /* OP_STATUS_SUPPORTING_ENTITY_IN_ERROR - not 
		     * supported by OA
		     */
		-1, /* OP_STATUS_COMPLETED - not supported by OA */
		-1, /* OP_STATUS_POWER_MODE - not supported by OA */
		-1, /* OP_STATUS_DMTF_RESERVED - not supported by OA */
		-1, /* OP_STATUS_VENDER_RESERVED - not supported
		     * by OA
		     */
	},
	/* OA_SOAP_REDUND_CLASS. This uses the enum redundancy values
	 * as index
	 */
	{
		/* uses the enum opStatus values as index */
		-1, /* REDUNDANCY_NO_OP - This state is returned if the
		     * device is not present. Ideally, this should never
		     * get executed.
		     */
		OA_SOAP_SEN_ASSERT_TRUE, /* REDUNDANCY_UNKNOWN */
		OA_SOAP_SEN_ASSERT_TRUE, /* NOT_REDUNDANT */
		OA_SOAP_SEN_ASSERT_FALSE, /* REDUNDANT */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_DIAG_CLASS. This uses the enum diagnosticStatus
	 * values as index
	 */
	{
		/* uses the enum opStatus values as index */
		-1, /* NOT_RELEVANT - this means that sensor is not
		     * supported. Ideally, this will never be returned.
		     * Not supported sensors will not be modelled for
		     * the resources in OA SOAP plugin
		     */
		OA_SOAP_SEN_ASSERT_FALSE, /* DIAGNOSTIC_CHECK_NOT_PERFORMED */
		OA_SOAP_SEN_ASSERT_FALSE, /* NO_ERROR */
		OA_SOAP_SEN_ASSERT_TRUE, /* ERROR */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_TEMP_CLASS sensor class. set the array contents to -1,
	 * as thermal class sensors do not utilize this mapping entry
	 */
	{
		
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_PWR_STATUS_CLASS sensor class. set the array contents to
	 * -1, as power status class sensors do not utilize this mapping entry
	 */
	{
		
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_FAN_SPEED_CLASS sensor class. set the array contents to
	 * -1, as fan speed class sensors do not utilize this mapping entry
	 */
	{
		
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_PWR_SUBSYS_CLASS sensor class. set the array contents to
	 * -1, as power subsystem class sensors do not utilize this mapping
	 * entry
	 */
	{
		
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_ENC_AGR_OPER_CLASS sensor class. This uses the enum 
	 * lcdSetupHealth values as index
	 */
	{
		OA_SOAP_SEN_ASSERT_TRUE, /* LCD_SETUP_HEALTH_UNKNOWN */
		OA_SOAP_SEN_ASSERT_FALSE, /* LCD_SETUP_HEALTH_OK */
		OA_SOAP_SEN_ASSERT_FALSE, /* LCD_SETUP_HEALTH_INFORMATIONAL */
		OA_SOAP_SEN_ASSERT_FALSE, /* LCD_SETUP_HEALTH_DEGRADED */
		OA_SOAP_SEN_ASSERT_TRUE, /* LCD_SETUP_HEALTH_FAILED */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_ENC_AGR_PRED_FAIL_CLASS sensor class. This uses the enum 
	 * lcdSetupHealth values as index
	 */
	{
		OA_SOAP_SEN_ASSERT_TRUE, /* LCD_SETUP_HEALTH_UNKNOWN */
		OA_SOAP_SEN_ASSERT_FALSE, /* LCD_SETUP_HEALTH_OK */
		OA_SOAP_SEN_ASSERT_TRUE, /* LCD_SETUP_HEALTH_INFORMATIONAL */
		OA_SOAP_SEN_ASSERT_TRUE, /* LCD_SETUP_HEALTH_DEGRADED */
		OA_SOAP_SEN_ASSERT_TRUE, /* LCD_SETUP_HEALTH_FAILED */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_BOOL_CLASS sensor class. This uses the enum 
	 * hpoa_boolean values as index. This class is used for OA link status.
	 * The sensor values 'false' means sensor state is asserted.
	 */
	{
		OA_SOAP_SEN_ASSERT_TRUE, /* false */
		OA_SOAP_SEN_ASSERT_FALSE, /* true */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_BOOL_RVRS_CLASS sensor class. This uses the enum 
	 * hpoa_boolean values as index. This class is used for interconnect CPU
	 * fault and health LED. The sensor value 'true' means sensor state
	 * is asserted
	 */
	{
		OA_SOAP_SEN_ASSERT_FALSE, /* false */
		OA_SOAP_SEN_ASSERT_TRUE, /* true */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_HEALTH_OPER_CLASS sensor class. This uses the enum 
	 * oa_soap_extra_data_health values as index
	 */
	{
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_UNKNOWN */
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_OTHER */
		OA_SOAP_SEN_ASSERT_FALSE, /* HEALTH_OK */
		OA_SOAP_SEN_ASSERT_FALSE, /* HEALTH_DEGRAD */
		OA_SOAP_SEN_ASSERT_FALSE, /* HEALTH_STRESSED */
		OA_SOAP_SEN_ASSERT_FALSE, /* HEALTH_PRED_FAIL */
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_ERROR */
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_NRE */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, 
	},
	/* OA_SOAP_HEALTH_PRED_FAIL_CLASS sensor class. This uses the
	 * enum oa_soap_extra_data_health values as index
	 */
	{
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_UNKNOWN */
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_OTHER */
		OA_SOAP_SEN_ASSERT_FALSE, /* HEALTH_OK */
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_DEGRAD */
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_STRESSED */
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_PRED_FAIL */
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_ERROR */
		OA_SOAP_SEN_ASSERT_TRUE, /* HEALTH_NRE */
		 /* dummy value to fill up array */
		-1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, 
	},
};

/* Array for creating the sensor RDRs
 * 
 * Please add new entries to the array on adding a new sensor
 */
const struct oa_soap_sensor oa_soap_sen_arr[] = {
	/* operational status sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_OPER_STATUS,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_OPER_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_OPER_STATUS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_OPER_STATUS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Operational status",
	},
	/* predictive failure status sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_PRED_FAIL,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_PRED_FAIL,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_PRED_FAILURE_DEASSERT |
				  SAHPI_ES_PRED_FAILURE_ASSERT,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_PRED_FAILURE_DEASSERT,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_PRED_FAILURE_ASSERT,
			.deassert_mask = SAHPI_ES_PRED_FAILURE_DEASSERT,
		},
		.sensor_class = OA_SOAP_PRED_FAIL_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_PRED_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_PRED_FAIL,
					.Assertion = SAHPI_TRUE,
					.EventState =
						SAHPI_ES_PRED_FAILURE_ASSERT,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState = 
						SAHPI_ES_PRED_FAILURE_DEASSERT,
					.CurrentState =
						SAHPI_ES_PRED_FAILURE_ASSERT,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_PRED_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_PRED_FAIL,
					.Assertion = SAHPI_FALSE,
					.EventState =
						SAHPI_ES_PRED_FAILURE_DEASSERT,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =
						SAHPI_ES_PRED_FAILURE_ASSERT,
					.CurrentState =
						SAHPI_ES_PRED_FAILURE_DEASSERT,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Predictive failure",
	},
	/* Thermal status sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_TEMP_STATUS,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 43,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 38,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			 /* For the resource supporting events,
			  * update the event enable to TRUE and
			  * set the masks accordingly to the support provided by
			  * the resource. Default values below highlight
			  * no-event support
    		 	  */
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 43,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 38,
			},
		},
		.sensor_class = OA_SOAP_TEMP_CLASS,
		.sen_evt = {
			/* Assert an event for crossing CAUTION threshold
			 * from OK condition
			 */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_TEMP_STATUS,
					.SensorType = SAHPI_TEMPERATURE,
					.EventCategory = SAHPI_EC_THRESHOLD,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_UPPER_MAJOR,
					.OptionalDataPresent =  
						SAHPI_SOD_TRIGGER_READING |
						SAHPI_SOD_TRIGGER_THRESHOLD |
						SAHPI_SOD_PREVIOUS_STATE |
						SAHPI_SOD_CURRENT_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_TRUE,
						.Type =
					      SAHPI_SENSOR_READING_TYPE_FLOAT64,
						 /* Trigger reading value is
						  * initialized to "0". 
						  * Replace this value with 
						  * current sensor reading
						  * value retrieved from OA
						  */
						.Value.SensorFloat64 = 0,
					  },
					  .TriggerThreshold = {
						.IsSupported = SAHPI_TRUE,
						.Type =
					      SAHPI_SENSOR_READING_TYPE_FLOAT64,
						 /* This default Normal Max 
						  * value should be replaced 
						  * with Normal Max value 
						  * retrieved from OA
						  */
						.Value.SensorFloat64 = 38,
 					 },
					.PreviousState =  SAHPI_ES_UNSPECIFIED,
					.CurrentState = SAHPI_ES_UPPER_MAJOR,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* De assert an event for crossing CAUTION threshold
			 * to OK condition
			 */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_TEMP_STATUS,
					.SensorType = SAHPI_TEMPERATURE,
					.EventCategory = SAHPI_EC_THRESHOLD,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_UPPER_MAJOR,
					.OptionalDataPresent =  
						SAHPI_SOD_TRIGGER_READING |
						SAHPI_SOD_TRIGGER_THRESHOLD |
						SAHPI_SOD_PREVIOUS_STATE |
						SAHPI_SOD_CURRENT_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_TRUE,
						.Type =
					      SAHPI_SENSOR_READING_TYPE_FLOAT64,
						 /* Trigger reading value is
						  * initialized to "0". 
						  * Replace this value with 
						  * current sensor reading value
						  * retrieved from OA
						  */
						.Value.SensorFloat64 = 0
					  },
					  .TriggerThreshold = {
						.IsSupported = SAHPI_TRUE,
						.Type =
		        		      SAHPI_SENSOR_READING_TYPE_FLOAT64,
						 /* This default Normal Max 
						  * value should be replaced 
						  * with Normal Max value 
						  * retrieved from OA
						  */
						.Value.SensorFloat64 = 38
 					 },
					.PreviousState = SAHPI_ES_UPPER_MAJOR,
					.CurrentState =  SAHPI_ES_UNSPECIFIED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Assert an event for crossing CRITICAL threshold
			 * from CAUTION condition
		 	 */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_TEMP_STATUS,
					.SensorType = SAHPI_TEMPERATURE,
					.EventCategory = SAHPI_EC_THRESHOLD,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_UPPER_CRIT,
					.OptionalDataPresent =  
						SAHPI_SOD_TRIGGER_READING |
						SAHPI_SOD_TRIGGER_THRESHOLD |
						SAHPI_SOD_PREVIOUS_STATE |
						SAHPI_SOD_CURRENT_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_TRUE,
						.Type =
			  		      SAHPI_SENSOR_READING_TYPE_FLOAT64,
						/* Trigger reading value is
				 		 * initialized to "0". 
						 * Replace this value with 
						 * current sensor reading value 
						 * retrieved from OA
						 */
						.Value.SensorFloat64 = 0
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_TRUE,
						.Type =
					      SAHPI_SENSOR_READING_TYPE_FLOAT64,
						/* This default Normal Max value
						 * should be replaced with 
						 * Normal Max value retrieved 
						 * from OA
						 */
						.Value.SensorFloat64 = 43
					},
					.PreviousState =  SAHPI_ES_UPPER_MAJOR,
					.CurrentState = SAHPI_ES_UPPER_CRIT,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert an event for crossing CRITICAL threshold
			 * to CAUTION condition
	   		 */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_TEMP_STATUS,
					.SensorType = SAHPI_TEMPERATURE,
					.EventCategory = SAHPI_EC_THRESHOLD,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_UPPER_CRIT,
					.OptionalDataPresent =  
						SAHPI_SOD_TRIGGER_READING |
						SAHPI_SOD_TRIGGER_THRESHOLD |
						SAHPI_SOD_PREVIOUS_STATE |
						SAHPI_SOD_CURRENT_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_TRUE,
						.Type =
					      SAHPI_SENSOR_READING_TYPE_FLOAT64,
						/* Trigger reading value is
				 		 * initialized to "0". 
						 * Replace this value with 
						 * current sensor reading value 
						 * retrieved from OA
						 */
						.Value.SensorFloat64 = 0
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_TRUE,
						.Type =
					      SAHPI_SENSOR_READING_TYPE_FLOAT64,
						/* This default Normal Max value
						 * should be replaced with 
						 * Normal Max value retrieved 
						 * from OA
						 */
						.Value.SensorFloat64 = 43
					},
					.PreviousState = 
						SAHPI_ES_UPPER_CRIT,
					.CurrentState = SAHPI_ES_UPPER_MAJOR,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
		},
      		.comment = "Ambient Zone Thermal Status",
	},
	/* Redundancy status sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_REDUND,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_REDUNDANCY,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_FULLY_REDUNDANT |
				  SAHPI_ES_REDUNDANCY_LOST,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_FULLY_REDUNDANT,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_REDUNDANCY_LOST,
			.deassert_mask = SAHPI_ES_FULLY_REDUNDANT,
		},
		.sensor_class = OA_SOAP_REDUND_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_REDUND,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_REDUNDANCY,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_REDUNDANCY_LOST,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =
						SAHPI_ES_FULLY_REDUNDANT,
					.CurrentState =
						SAHPI_ES_REDUNDANCY_LOST,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_REDUND,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_REDUNDANCY,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_FULLY_REDUNDANT,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =
						SAHPI_ES_REDUNDANCY_LOST,
					.CurrentState =
						SAHPI_ES_FULLY_REDUNDANT,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Redundancy status",
	},
	/* Fan Speed Sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_FAN_SPEED,
			.Type = SAHPI_COOLING_DEVICE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_RPM,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_MIN,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 18000,
				.Range.Min.IsSupported = SAHPI_TRUE,
				.Range.Min.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Min value should be
				  * replaced with Min value retrieved
				  * from OA
				  */
				.Range.Min.Value.SensorFloat64 = 10,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_LOWER_CRIT,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 18000,
				.LowCritical.IsSupported = SAHPI_TRUE,
				.LowCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.LowCritical.Value.SensorFloat64 = 10,
			},
		},
		.sensor_class = OA_SOAP_FAN_SPEED_CLASS,
		.comment = "Fan speed",
	},
	/* Power status sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_PWR_STATUS,
			.Type = SAHPI_POWER_SUPPLY,
			.Category = SAHPI_EC_UNSPECIFIED,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_WATTS,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = 0,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
		},
		.sensor_class = OA_SOAP_PWR_STATUS_CLASS,
		.comment = "Power status",
	},
	/* Diagnostic internal data error sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_INT_DATA_ERR,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_INT_DATA_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_INT_DATA_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Internal Data error",
	},
	/* Diagnostic management processor error sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_MP_ERR,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_MP_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_MP_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Management processor error",
	},
	/* Power Supply Subsystem Power input Sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_IN_PWR,
			.Type = SAHPI_POWER_SUPPLY,
			.Category = SAHPI_EC_UNSPECIFIED,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_WATTS,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = 0,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
		},
		.sensor_class = OA_SOAP_PWR_SUBSYS_CLASS,
		.comment = "Power Input sensor",
	},
	/* Power Supply Subsystem Power output Sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_OUT_PWR,
			.Type = SAHPI_POWER_SUPPLY,
			.Category = SAHPI_EC_UNSPECIFIED,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_WATTS,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = 0,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
		},
		.sensor_class = OA_SOAP_PWR_SUBSYS_CLASS,
		.comment = "Power Output sensor",
	},
	/* Power Supply Subsystem Power capacity Sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_PWR_CAPACITY,
			.Type = SAHPI_POWER_SUPPLY,
			.Category = SAHPI_EC_UNSPECIFIED,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_WATTS,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = 0,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
		},
		.sensor_class = OA_SOAP_PWR_SUBSYS_CLASS,
		.comment = "Power Capacity sensor",
	},
	/* Diagnostic thermal warning sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_THERM_WARN,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_THERM_WARN,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_THERM_WARN,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Thermal warning",
	},
	/* Diagnostic thermal danger sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_THERM_DANGER,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_THERM_DANGER,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_THERM_DANGER,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Thermal danger",
	},
	/* Diagnostic IO configuration error sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_IO_CONFIG_ERR,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_IO_CONFIG_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_IO_CONFIG_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "IO configuration error",
	},
	/* Diagnostic device power request error sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_PWR_REQ,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_PWR_REQ,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_PWR_REQ,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device power request error",
	},
	/* Diagnostic insufficient cooling sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_INSUF_COOL,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_INSUF_COOL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_INSUF_COOL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Insufficient cooling",
	},
	/* Diagnostic device location error sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_LOC_ERR,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_LOC_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_LOC_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device location error",
	},
	/* Diagnostic device failure sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_FAIL,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device failure",
	},
	/* Diagnostic device degraded sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_DEGRAD,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_DEGRAD,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_DEGRAD,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device degraded",
	},
	/* Diagnostic AC failure sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_AC_FAIL,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_AC_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_AC_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "AC failure",
	},
	/* Diagnostic i2c buses sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_I2C_BUS,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_I2C_BUS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_I2C_BUS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "i2c buses",
	},
	/* Diagnostic redundancy error sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_REDUND_ERR,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_REDUND_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_REDUND_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "redundancy error",
	},
	/* Enclosure aggregate operational status sensor */
  	{
		.sensor = {
			.Num = OA_SOAP_SEN_ENC_AGR_OPER,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
  			},
  			.Oem = 0,
  		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_ENC_AGR_OPER_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_ENC_AGR_OPER,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_ENC_AGR_OPER,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Enclosure aggregate operational status",
  	},
	/* Enclosure aggregate predictive failure sensor */
  	{
		.sensor = {
			.Num = OA_SOAP_SEN_ENC_AGR_PRED_FAIL,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
  			},
  			.Oem = 0,
  		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_ENC_AGR_PRED_FAIL_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_ENC_AGR_PRED_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = 
					OA_SOAP_SEN_ENC_AGR_PRED_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Enclosure aggregate predictive failure",
  	},
	/* OA redundancy sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_OA_REDUND,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_BOOL_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_OA_REDUND,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_OA_REDUND,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "OA Redundancy",
	},
	/* OA link status sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_OA_LINK_STATUS,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_BOOL_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_OA_LINK_STATUS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_OA_LINK_STATUS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "OA link status",
	},
	/* Interconnect CPU fault sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_CPU_FAULT,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_BOOL_RVRS_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_CPU_FAULT,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_CPU_FAULT,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Interconnect CPU Fault",
	},
	/* Interconnect health LED sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_HEALTH_LED,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_BOOL_RVRS_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_HEALTH_LED,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_HEALTH_LED,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Interconnect health LED",
	},
	/* Health status operational sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_HEALTH_OPER,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_HEALTH_OPER_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_HEALTH_OPER,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_CRITICAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_HEALTH_OPER,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Health status operational",
	},
	/* Health status predictive failure sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_HEALTH_PRED_FAIL,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_HEALTH_PRED_FAIL_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_HEALTH_PRED_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = 
					OA_SOAP_SEN_HEALTH_PRED_FAIL,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Health status predictive failure",
	},
	/* DiagnosticChecksEx Device missing sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_MISS,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_MISS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_MISS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device missing",
	},
	/* DiagnosticChecksEx Device power sequence sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_PWR_SEQ,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_PWR_SEQ,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_PWR_SEQ,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device power sequence",
	},
	/* DiagnosticChecksEx Device bonding sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_BOND,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_BOND,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_BOND,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device bonding",
	},
	/* DiagnosticChecksEx network configuration sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_NET_CONFIG,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_NET_CONFIG,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_NET_CONFIG,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Network configuration",
	},
	/* DiagnosticChecksEx firmware mismatch sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_FW_MISMATCH,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_FW_MISMATCH,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_FW_MISMATCH,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Firmwre mismatch",
	},
	/* DiagnosticChecksEx Profile unassigned error sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_PROF_UNASSIGN_ERR,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_PROF_UNASSIGN_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_PROF_UNASSIGN_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Profile unassigned error",
	},
	/* DiagnosticChecksEx Device not supported sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_NOT_SUPPORT,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_DEV_NOT_SUPPORT,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_DEV_NOT_SUPPORT,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device not supported",
	},
	/* DiagnosticChecksEx Too low power request sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_TOO_LOW_PWR_REQ,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_TOO_LOW_PWR_REQ,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_TOO_LOW_PWR_REQ,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Too low power request",
	},
	/* DiagnosticChecksEx Call HP sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_CALL_HP,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_INFORMATIONAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_CALL_HP,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_INFORMATIONAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_CALL_HP,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Call HP",
	},
	/* DiagnosticChecksEx Device informational sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_INFO,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_INFORMATIONAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_INFO,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_INFORMATIONAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_INFO,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device informational",
	},
	/* DiagnosticChecksEx Storage device missing sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_STORAGE_DEV_MISS,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_STORAGE_DEV_MISS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_STORAGE_DEV_MISS,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Storage device missing",
	},
	/* DiagnosticChecksEx Enclosure ID mismatch sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_ENC_ID_MISMATCH,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_ENC_ID_MISMATCH,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_ENC_ID_MISMATCH,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Enclosure ID mismatch",
	},
	/* DiagnosticChecksEx Device mix match sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DEV_MIX_MATCH,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_MIX_MATCH,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MINOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_DEV_MIX_MATCH,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Device mix match",
	},
	/* DiagnosticChecksEx Power capping error sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_GRPCAP_ERR,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_GRPCAP_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_GRPCAP_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Power capping error",
	},
	/* DiagnosticChecksEx IML recorded errors sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_IML_ERR,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_INFORMATIONAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_IML_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_INFORMATIONAL,
				.EventDataUnion.SensorEvent = {
					.SensorNum = OA_SOAP_SEN_IML_ERR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "IML recorded errors",
	},
	/* DiagnosticChecksEx Duplicate management IP address sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_DUP_MGMT_IP_ADDR,
			.Type = SAHPI_OPERATIONAL,
			.Category = SAHPI_EC_ENABLE,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_PER_EVENT,
			.Events = SAHPI_ES_ENABLED | SAHPI_ES_DISABLED,
			.DataFormat = {
				.IsSupported = SAHPI_FALSE,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_ENABLED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_TRUE,
			.assert_mask = SAHPI_ES_DISABLED,
			.deassert_mask = SAHPI_ES_ENABLED,
		},
		.sensor_class = OA_SOAP_DIAG_CLASS,
		.sen_evt = {
			/* Assert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_DUP_MGMT_IP_ADDR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_TRUE,
					.EventState = SAHPI_ES_DISABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_ENABLED,
					.CurrentState = SAHPI_ES_DISABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			/* Deassert event */
			{
				.EventType = SAHPI_ET_SENSOR,
				.Severity = SAHPI_MAJOR,
				.EventDataUnion.SensorEvent = {
					.SensorNum =
					OA_SOAP_SEN_DUP_MGMT_IP_ADDR,
					.SensorType = SAHPI_OPERATIONAL,
					.EventCategory = SAHPI_EC_ENABLE,
					.Assertion = SAHPI_FALSE,
					.EventState = SAHPI_ES_ENABLED,
					.OptionalDataPresent =
						SAHPI_SOD_CURRENT_STATE |
						SAHPI_SOD_PREVIOUS_STATE,
					.TriggerReading = {
						.IsSupported = SAHPI_FALSE,
					},
					.TriggerThreshold = {
						.IsSupported = SAHPI_FALSE,
					},
					.PreviousState =  SAHPI_ES_DISABLED,
					.CurrentState = SAHPI_ES_ENABLED,
					.Oem = 0,
					.SensorSpecific = 0,
				},
			},
			{},
			{},
		},
		.comment = "Duplicate management IP address",
	},
	/* System zone1 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_SYSTEM_ZONE1,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 85,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 80,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 85,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 80,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "System Zone thermal status",
	},
	/* System zone2 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_SYSTEM_ZONE2,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 85,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 80,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 85,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 80,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "System Zone thermal status",
	},
	/* System zone3 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_SYSTEM_ZONE3,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 85,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 80,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 85,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 80,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "System Zone thermal status",
	},
	/* System zone4 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_SYSTEM_ZONE4,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 85,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 80,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 85,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 80,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "System Zone thermal status",
	},
	/* CPU zone1 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU_ZONE1,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 70,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 65,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 70,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 65,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU Zone thermal status",
	},
	/* CPU zone2 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU_ZONE2,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 70,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 65,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 70,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 65,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU Zone thermal status",
	},
	/* CPU zone3 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU_ZONE3,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 70,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 65,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 70,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 65,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU Zone thermal status",
	},
	/* CPU zone4 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU_ZONE4,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 70,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 65,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 70,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 65,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU Zone thermal status",
	},
	/* Memory zone1 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_MEM_ZONE1,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 85,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 85,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "Memory Zone thermal status",
	},
	/* Memory zone2 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_MEM_ZONE2,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 85,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 85,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "Memory Zone thermal status",
	},
	/* Memory zone3 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_MEM_ZONE3,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 85,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 85,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "Memory Zone thermal status",
	},
	/* Memory zone4 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_MEM_ZONE4,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 85,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 85,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "Memory Zone thermal status",
	},
	/* Disk zone1 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_DISK_ZONE1,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 85,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 85,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "Disk Zone thermal status",
	},
	/* Disk zone2 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_DISK_ZONE2,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 85,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 85,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "Disk Zone thermal status",
	},
	/* Disk zone3 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_DISK_ZONE3,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 85,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 85,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "Disk Zone thermal status",
	},
	/* Disk zone4 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_DISK_ZONE4,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 85,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 85,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "Disk Zone thermal status",
	},
	/* CPU 1 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU1_1,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 1 thermal status",
	},
	/* CPU 1 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU1_2,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 1 thermal status",
	},
	/* CPU 1 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU1_3,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 1 thermal status",
	},
	/* CPU 1 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU1_4,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 1 thermal status",
	},
	/* CPU 2 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU2_1,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 2 thermal status",
	},
	/* CPU 2 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU2_2,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 2 thermal status",
	},
	/* CPU 2 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU2_3,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 2 thermal status",
	},
	/* CPU 2 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU2_4,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 2 thermal status",
	},
	/* CPU 3 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU3_1,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 3 thermal status",
	},
	/* CPU 3 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU3_2,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 3 thermal status",
	},
	/* CPU 3 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU3_3,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 3 thermal status",
	},
	/* CPU 3 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU3_4,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 3 thermal status",
	},
	/* CPU 4 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU4_1,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 4 thermal status",
	},
	/* CPU 4 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU4_2,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 4 thermal status",
	},
	/* CPU 4 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU4_3,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 4 thermal status",
	},
	/* CPU 4 sensor */
	{
		.sensor = {
			.Num = OA_SOAP_SEN_BLADE_CPU4_4,
			.Type = SAHPI_TEMPERATURE,
			.Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_TRUE,
			.EventCtrl = SAHPI_SEC_READ_ONLY,
			.Events = SAHPI_ES_UNSPECIFIED,
			.DataFormat = {
				.IsSupported = SAHPI_TRUE,
				.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				.BaseUnits = SAHPI_SU_DEGREES_C,
				.ModifierUnits = SAHPI_SU_UNSPECIFIED,
				.ModifierUse = SAHPI_SMUU_NONE,
				.Percentage = SAHPI_FALSE,
				.Range.Flags = SAHPI_SRF_MAX |
					       SAHPI_SRF_NORMAL_MAX,
				.Range.Max.IsSupported = SAHPI_TRUE,
				.Range.Max.Type = 
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Max value should be replaced
				  * with Max value retrieved from OA
				  */
				.Range.Max.Value.SensorFloat64 = 100,
				.Range.NormalMax.IsSupported = SAHPI_TRUE,
				.Range.NormalMax.Type =
				       SAHPI_SENSOR_READING_TYPE_FLOAT64,
				 /* This default Normal Max value should be
				  * replaced with Normal Max value retrieved
				  * from OA
				  */
				.Range.NormalMax.Value.SensorFloat64 = 95,
				.AccuracyFactor =  0,
			},
			.ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
				.ReadThold = SAHPI_ES_UPPER_CRIT |
					     SAHPI_ES_UPPER_MAJOR,
				.WriteThold = 0x0,
			},
			.Oem = 0,
		},
		.sensor_info = {
			.current_state = SAHPI_ES_UNSPECIFIED,
			.sensor_enable = SAHPI_TRUE,
			.event_enable = SAHPI_FALSE,
			.assert_mask = OA_SOAP_STM_UNSPECIFED,
			.deassert_mask = OA_SOAP_STM_UNSPECIFED,
			.sensor_reading = {
				.IsSupported = SAHPI_TRUE,
				.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with current reading */
				.Value.SensorFloat64 = 0x0,
			},
			.threshold = {
				.UpCritical.IsSupported = SAHPI_TRUE,
				.UpCritical.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with critical threshold
				 * reading
				 */
				.UpCritical.Value.SensorFloat64 = 100,
				.UpMajor.IsSupported = SAHPI_TRUE,
				.UpMajor.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64,
				/* Update the value with major threshold
				 * reading
				 */
				.UpMajor.Value.SensorFloat64 = 95,
			},
		},
		.sensor_class = OA_SOAP_BLADE_THERMAL_CLASS,
      		.comment = "CPU 4 thermal status",
	},
	/* NULL element to end the array */
	{}
};

/* Global array containing the details of all control rdr structure details 
 *
 * Please add new entries to the array on supporting new control in OA SOAP
 */
const struct oa_soap_control oa_soap_cntrl_arr[] = {
	/* UID status */
	{
		.control = {
			.Num = OA_SOAP_UID_CNTRL,
			.OutputType = SAHPI_CTRL_LED,
			.Type = SAHPI_CTRL_TYPE_DIGITAL,
			.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_OFF,
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_MANUAL,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
			.Oem = 0,
		},
		.comment = "UID LED state",
	},
	/* Power status */
	{
		.control = {
			.Num = OA_SOAP_PWR_CNTRL,
			.OutputType = SAHPI_CTRL_POWER_STATE,
			.Type = SAHPI_CTRL_TYPE_DIGITAL,
			.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_ON,
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_MANUAL,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
			.Oem = 0,
		},
		.comment = "power state",
	},
	/* LCD button lock */
	{
		.control = {
			.Num = OA_SOAP_LCD_BUTN_LCK_CNTRL,
			.OutputType = SAHPI_CTRL_FRONT_PANEL_LOCKOUT,
			.Type = SAHPI_CTRL_TYPE_DIGITAL,
			.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_OFF,
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_MANUAL,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
			.Oem = 0,
		},
		.comment = "LCD button lock",
	},
	{} /* Terminate array with a null element */
};

/* Array for constructing the RPT entry. The EntiyLocation from 0 to proper
 * value except for SAHPI_ENT_ROOT
 * 
 * Please add items to the array on adding a new resource
 */
const SaHpiRptEntryT oa_soap_rpt_arr[] = {
	/* OA_SOAP_ENT_ENC */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR |
					SAHPI_CAPABILITY_INVENTORY_DATA,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 9,
		.ResourceTag.Data = "Enclosure",
	},
	/* OA_SOAP_ENT_SERVER */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_SYSTEM_BLADE,
				.EntityLocation = 0,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR |
					SAHPI_CAPABILITY_RESET |
					SAHPI_CAPABILITY_POWER |
					SAHPI_CAPABILITY_FRU |
					SAHPI_CAPABILITY_MANAGED_HOTSWAP |
					SAHPI_CAPABILITY_CONTROL |
					SAHPI_CAPABILITY_INVENTORY_DATA,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 12,
		.ResourceTag.Data = "Server Blade",
	},
	/* OA_SOAP_ENT_IO */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_IO_BLADE,
				.EntityLocation = 0,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR |
					SAHPI_CAPABILITY_FRU |
					SAHPI_CAPABILITY_INVENTORY_DATA,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 8,
		.ResourceTag.Data = "IO Blade",
	},
	/* OA_SOAP_ENT_STORAGE */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_DISK_BLADE,
				.EntityLocation = 0,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR |
					SAHPI_CAPABILITY_FRU |
					SAHPI_CAPABILITY_INVENTORY_DATA,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 13,
		.ResourceTag.Data = "Storage Blade",
	},
	/* OA_SOAP_ENT_SWITCH */
	{
		.ResourceInfo = {
			/* Change the manufacture ID, if the switch is belongs
			 * to Cisco Systems
			 */
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_SWITCH_BLADE,
				.EntityLocation = 0,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR |
					SAHPI_CAPABILITY_RESET |
					SAHPI_CAPABILITY_POWER |
					SAHPI_CAPABILITY_FRU |
					SAHPI_CAPABILITY_MANAGED_HOTSWAP |
					SAHPI_CAPABILITY_CONTROL |
					SAHPI_CAPABILITY_INVENTORY_DATA,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 12,
		.ResourceTag.Data = "Switch Blade",
	},
	/* OA_SOAP_ENT_OA */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_SYS_MGMNT_MODULE,
				.EntityLocation = 0,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR |
					SAHPI_CAPABILITY_FRU |
					SAHPI_CAPABILITY_INVENTORY_DATA,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 22,
		.ResourceTag.Data = "Onboard Administrator",
	},
	/* OA_SOAP_ENT_PS_SUBSYS */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_POWER_MGMNT,
				.EntityLocation = 1,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 15,
		.ResourceTag.Data = "Power subsystem",
	},
	/* OA_SOAP_ENT_PS */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_POWER_SUPPLY,
				.EntityLocation = 0,
			},
			{
				.EntityType = SAHPI_ENT_POWER_MGMNT,
				.EntityLocation = 1,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR |
					SAHPI_CAPABILITY_FRU |
					SAHPI_CAPABILITY_INVENTORY_DATA,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 12,
		.ResourceTag.Data = "Power supply",
	},
	/* OA_SOAP_ENT_THERM_SUBSYS */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_COOLING_UNIT,
				.EntityLocation = 1,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 18,
		.ResourceTag.Data = "Thermal Subsystem",
	},
	/* OA_SOAP_ENT_FZ */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_COOLING_DEVICE,
				.EntityLocation = 0,
			},
			{
				.EntityType = SAHPI_ENT_COOLING_UNIT,
				.EntityLocation = 1,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR |
					SAHPI_CAPABILITY_INVENTORY_DATA,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 8,
		.ResourceTag.Data = "Fan Zone",
	},
	/* OA_SOAP_ENT_FAN */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_FAN,
				.EntityLocation = 0,
			},
			{
				.EntityType = SAHPI_ENT_COOLING_DEVICE,
				.EntityLocation = 0,
			},
			{
				.EntityType = SAHPI_ENT_COOLING_UNIT,
				.EntityLocation = 1,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR |
					SAHPI_CAPABILITY_FRU |
					SAHPI_CAPABILITY_INVENTORY_DATA,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 3,
		.ResourceTag.Data = "Fan",
	},
	/* OA_SOAP_ENT_LCD */
	{
		.ResourceInfo = {
			.ManufacturerId = HP_MANUFACTURING_ID,
		},
		.ResourceEntity = {
			.Entry[0] =
			{
				.EntityType = SAHPI_ENT_DISPLAY_PANEL,
				.EntityLocation = 1,
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 0,
			},
		},
		.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
					SAHPI_CAPABILITY_INVENTORY_DATA |
					SAHPI_CAPABILITY_CONTROL |
					SAHPI_CAPABILITY_RESOURCE |
					SAHPI_CAPABILITY_SENSOR,
		.ResourceSeverity = SAHPI_OK,
		.ResourceFailed = SAHPI_FALSE,
		.HotSwapCapabilities = 0x0,
		.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT,
		.ResourceTag.Language = SAHPI_LANG_ENGLISH,
		.ResourceTag.DataLength = 3,
		.ResourceTag.Data = "LCD",
	},
	/* NULL element to end the array */
	{}
};

/* Array for constructing the inventory RDR.
 * 
 * Please add items to the array on adding a new resource or on adding area or
 * field
 */
const struct oa_soap_inv_rdr oa_soap_inv_arr[] = {
	/* OA_SOAP_ENT_ENC */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 9,
			.IdString.Data = "Enclosure",
		},
	},
	/* OA_SOAP_ENT_SERVER */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 12,
			.IdString.Data = "Server Blade",
		},
	},
	/* OA_SOAP_ENT_IO */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 8,
			.IdString.Data = "IO Blade",
		},
	},
	/* OA_SOAP_ENT_STORAGE */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 13,
			.IdString.Data = "Storage Blade",
		},
	},
	/* OA_SOAP_ENT_SWITCH */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 12,
			.IdString.Data = "Switch Blade",
		},
	},
	/* OA_SOAP_ENT_OA */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 22,
			.IdString.Data = "Onboard Administrator",
		},
	},
	/* OA_SOAP_ENT_PS_SUBSYS */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 15,
			.IdString.Data = "Power Subsystem",
		},
	},
	/* OA_SOAP_ENT_PS */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 12,
			.IdString.Data = "Power Supply",
		},
	},
	/* OA_SOAP_ENT_THERM_SUBSYS */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 17,
			.IdString.Data = "Thermal Subsystem",
		},
	},
	/* OA_SOAP_ENT_FZ */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 8,
			.IdString.Data = "Fan Zone",
		},
		.inventory = {
			.inv_rec = {
				.IdrId = SAHPI_DEFAULT_INVENTORY_ID,
				.Persistent = SAHPI_FALSE,
				.Oem = 0,
			},
			.info = {
				.idr_info = {
					.IdrId = SAHPI_DEFAULT_INVENTORY_ID,
					.UpdateCount = 1,
					.ReadOnly = SAHPI_FALSE,
					.NumAreas = 1,
				},
				.area_list = NULL,
			},
		},
		.area_array = {
			{
				.area = {
					.idr_area_head = {
						.AreaId = 1,
						.Type =
						SAHPI_IDR_AREATYPE_OEM,
						.ReadOnly = SAHPI_FALSE,
						.NumFields = 2,
					},
					.next_area = NULL,
				},
				.field_array = {
					{
						/* Field for storing the device
						 * bays for this Fan Zone
						 */
						.field = {
							.AreaId = 1,
							.FieldId = 1,
							.Type =
							OA_SOAP_INV_FZ_DEV_BAY,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
					{
						/* Field for storing the fan
						 * bays for this Fan Zone
						 */
						.field = {
							.AreaId = 1,
							.FieldId = 2,
							.Type =
							OA_SOAP_INV_FZ_FAN_BAY,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
				},
			},
		},
	},
	/* OA_SOAP_ENT_FAN */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 3,
			.IdString.Data = "Fan",
		},
		.inventory = {
			.inv_rec = {
				.IdrId = SAHPI_DEFAULT_INVENTORY_ID,
				.Persistent = SAHPI_FALSE,
				.Oem = 0,
			},
			.info = {
				.idr_info = {
					.IdrId = SAHPI_DEFAULT_INVENTORY_ID,
					.UpdateCount = 1,
					.ReadOnly = SAHPI_FALSE,
					.NumAreas = 3,
				},
				.area_list = NULL,
			},
		},
		.area_array = {
			{
				.area = {
					.idr_area_head = {
						.AreaId = 1,
						.Type =
						SAHPI_IDR_AREATYPE_PRODUCT_INFO,
						.ReadOnly = SAHPI_FALSE,
						.NumFields = 1,
					},
					.next_area = NULL,
				},
				.field_array = {
					{
						.field = {
							.AreaId = 1,
							.FieldId = 1,
							.Type =
					SAHPI_IDR_FIELDTYPE_PRODUCT_NAME,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
				},
			},
			{
				.area = {
					.idr_area_head = {
						.AreaId = 2,
						.Type =
						SAHPI_IDR_AREATYPE_BOARD_INFO,
						.ReadOnly = SAHPI_FALSE,
						.NumFields = 2,
						},
					.next_area = NULL,
				},
				.field_array = {
					{
						.field = {
							.AreaId = 2,
							.FieldId = 1,
							.Type =
						SAHPI_IDR_FIELDTYPE_PART_NUMBER,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
					{
						.field = {
							.AreaId = 2,
							.FieldId = 2,
							.Type =
					SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
				},
			},
			{
				.area = {
					.idr_area_head = {
						.AreaId = 3,
						.Type =
						SAHPI_IDR_AREATYPE_OEM,
						.ReadOnly = SAHPI_FALSE,
						.NumFields = 2,
					},
					.next_area = NULL,
				},
				.field_array = {
					{
						/* This field indicates whether
						 * this fan is shared or not
						 */
						.field = {
							.AreaId = 3,
							.FieldId = 1,
							.Type =
						OA_SOAP_INV_FAN_SHARED,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
					{
						/* Field for storing the fan
						 * Fan Zone(s) to which this fan
						 * belongs
						 */
						.field = {
							.AreaId = 3,
							.FieldId = 2,
							.Type =
							OA_SOAP_INV_FZ_NUM,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
				},
			},
		},
	},
	/* OA_SOAP_ENT_LCD */
	{
		.rdr = {
			.RecordId = 0,
			.RdrType = SAHPI_INVENTORY_RDR,
			.RdrTypeUnion.InventoryRec.IdrId =
				SAHPI_DEFAULT_INVENTORY_ID,
			.IdString.DataType = SAHPI_TL_TYPE_TEXT,
			.IdString.Language = SAHPI_LANG_ENGLISH,
			.IdString.DataLength = 3,
			.IdString.Data = "LCD",
		},
		.inventory = {
			.inv_rec = {
				.IdrId = SAHPI_DEFAULT_INVENTORY_ID,
				.Persistent = SAHPI_FALSE,
				.Oem = 0,
			},
			.info = {
				.idr_info = {
					.IdrId = SAHPI_DEFAULT_INVENTORY_ID,
					.UpdateCount = 1,
					.ReadOnly = SAHPI_FALSE,
					.NumAreas = 2,
				},
				.area_list = NULL,
			},
		},
		.area_array = {
			{
				.area = {
					.idr_area_head = {
						.AreaId = 1,
						.Type =
						SAHPI_IDR_AREATYPE_PRODUCT_INFO,
						.ReadOnly = SAHPI_FALSE,
						.NumFields = 3,
					},
					.next_area = NULL,
				},
				.field_array = {
					{
						.field = {
							.AreaId = 1,
							.FieldId = 1,
							.Type =
					SAHPI_IDR_FIELDTYPE_PRODUCT_NAME,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
					{
						.field = {
							.AreaId = 1,
							.FieldId = 2,
							.Type =
					SAHPI_IDR_FIELDTYPE_MANUFACTURER,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
					{
						.field = {
							.AreaId = 1,
							.FieldId = 3,
							.Type =
					SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
				},
			},
			{
				.area = {
					.idr_area_head = {
						.AreaId = 2,
						.Type =
						SAHPI_IDR_AREATYPE_BOARD_INFO,
						.ReadOnly = SAHPI_FALSE,
						.NumFields = 1,
						},
					.next_area = NULL,
				},
				.field_array = {
					{
						.field = {
							.AreaId = 2,
							.FieldId = 1,
							.Type =
						SAHPI_IDR_FIELDTYPE_PART_NUMBER,
							.ReadOnly = SAHPI_FALSE,
							.Field.DataType =
							SAHPI_TL_TYPE_TEXT,
							.Field.Language =
							SAHPI_LANG_ENGLISH,
						},
						.next_field = NULL,
					},
				},
			},
		},
	},
	/* NULL element to end the array */
	{}
};

/* Array for mapping the fans to fan zones 
 *
 * Please add entries to the array on supporting new enclosure type or on change
 * in the max fans number defined in oa_soap_inventory.h
 */
const struct oa_soap_fz_map oa_soap_fz_map_arr[][OA_SOAP_MAX_FAN] = {
	/* OA_SOAP_ENC_C7000 */
	{
		/* Fan slot 1 */
		{
			.zone = 2,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 2 */
		{
			.zone = 2,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 3 */
		{
			.zone = 1,
			.secondary_zone = 2,
			.shared = SAHPI_TRUE,
		},
		/* Fan slot 4 */
		{
			.zone = 1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 5 */
		{
			.zone = 1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 6 */
		{
			.zone = 4,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 7 */
		{
			.zone = 4,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 8 */
		{
			.zone = 3,
			.secondary_zone = 4,
			.shared = SAHPI_TRUE,
		},
		/* Fan slot 9 */
		{
			.zone = 3,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 10 */
		{
			.zone = 3,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
	},
	/* OA_SOAP_ENC_C3000 */
	{
		/* Fan slot 1 */
		{
			.zone = 1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 2 */
		{
			.zone = 1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 3 */
		{
			.zone = 1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 4 */
		{
			.zone = 1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 5 */
		{
			.zone = 1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 6 */
		{
			.zone = 1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 7 */
		{
			.zone = -1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 8 */
		{
			.zone = -1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 9 */
		{
			.zone = -1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
		/* Fan slot 10 */
		{
			.zone = -1,
			.secondary_zone = 0,
			.shared = SAHPI_FALSE,
		},
	},
	/* NULL element to end the array */
	{}
};

/* Array to hold the values of healthStatus field in extraData structure.
 * This array is indexed on enum oa_soap_extra_data_health
 */
const char *oa_soap_health_arr[] = {
	"UNKNOWN",
	"OTHER",
	"OK",
	"DEGRADED",
	"STRESSED",
	"PREDICTIVE_FAILURE",
	"ERROR",
	"NONRECOVERABLE_ERROR"
};

/* Array to hold the supported fields of diagnosticChecksEx structure. This
 * array is indexed on enum oa_soap_diag_ex
 *
 * When a new field is added to diagnosticChecksEx structure, please update the
 * enum oa_soap_diag_ex and OA_SOAP_MAX_DIAG_EX in oa_soap_sensor.h
 */
const char *oa_soap_diag_ex_arr[] = {
	/* Missing Hardware (such as partner device) */
	"deviceMissing",
	/* Sequencing Error (some hardware was introduced in the wrong order) */
	"devicePowerSequence",
	/* Bonding Error (some hardware was bonded that should not) */
	"deviceBonding",
	 /* VCM personality issue */
	"profileUnassignedError",
	/* A device failed lagacy list match and FRU "Manufactured For"
	 * check
	 */
	"deviceNotSupported",
	/* Network Configuration issue. It is likely that iLO is  unable to ARP
	 * it's default gateway.
	 */
	"networkConfiguration",
	/* server requested too little power - hardware issue */
	"tooLowPowerRequest",
	/* some hardware warrants a HP call by the customer */
	"callHP",
	/* Generic informational message in syslog */
	"deviceInformational",
	/* IOM storage is missing */
	"storageDeviceMissing",
	/* OA Firmware out of sync */
	"firmwareMismatch",
	/* Enclosure ID mismatch */
	"enclosureIdMismatch",
	/* PowerDelay is in use. 'Power delay in use' is not used as sensor.
	 * 'Power delay in use' does not indicates any failure
	 */
	"powerdelayInUse",
	/* Device mix-and-match alert */
	"deviceMixMatch",
	/* Power capping alert */
	"grpcapError",
	/* IML recorded errors */
	"imlRecordedErrors",
	/* Duplicate Management IP address */
	"duplicateMgmtIpAddress"
};

/* Array containing the possible sensor description string provided by
 * getBladeThermalInfoArray soap call
 */
const char *oa_soap_thermal_sensor_string[] = {
	"System",
	"CPU Zone",
	"CPU 1",
	"CPU 2",
	"CPU 3",
	"CPU 4",
	"Disk",
	"Memory",
	"Ambient"
};

/* Array containing the name strings of the possible blade types
 * which can be accomodated in HP cClass BladeSystem chassis
 */
const char *oa_soap_bld_type_str[] = {
	"BL260C",
	"BL2x220C",
	"BL460C",
	"BL465C",
	"BL480C",
	"BL495C",
	"BL680C",
	"BL685C",
	"BL860C",
	"BL870C",
	"NB50000C",
	"AMC",
	"STORAGE",
	"TAPE",
	"SAN"
};

/* Array containing static thermal sensor configuration for different type of 
 * blade resources in the hardware portfolio supported by HP cClass BladeSystem
 * This static configuration for each blade type is based on the generalized 
 * information available from BladeThermalInfo response from the blades. 
 * TODO: If a particular version of a blade supports more sensors than the 
 * statically modeled sensors when powered on, then those sensors cannot be 
 * monitored. When the plug-in migrates to HPI-B.03.01 specification, then
 * condition can be overcome. 
 *
 * Please modify the array on adding new blade type in oa_soap_resources.h
 */
const struct oa_soap_static_thermal_sensor_info 
	oa_soap_static_thrm_sen_config[OA_SOAP_MAX_BLD_TYPE]
					    [OA_SOAP_MAX_THRM_SEN] = {
	/* BL260c blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 1},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 1},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 1},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* BL2x220c blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 1},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 1},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 1},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* BL460c blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 2},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 2},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 2},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 1},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* BL465c blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 1},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 1},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 2},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* BL480c blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 4},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 1},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 1},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 1},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* BL495c blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 2},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 2},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 1},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* BL680 blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 2},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 2},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 1},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* BL685 blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 2},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 2},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 1},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 1},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 1},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 1},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 0},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* BL860c blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 4},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 1},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 1},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 0},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* BL870c blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 4},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 1},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 1},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 1},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 1},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 0},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* NB50000c blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 4},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 4},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 4},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 4},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 4},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 0},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* AMC Expansion IO blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 1},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 0},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 0},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 0},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* Storage blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 0},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 0},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 1},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 0},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* Tape blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 0},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 0},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 1},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 0},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* SAN blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 0},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 0},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 0},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 0},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 0},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 1},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 0},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
	/* OTHER blade type */
	{
		{OA_SOAP_SEN_BLADE_SYSTEM_ZONE1, SYSTEM_ZONE, 2},
		{OA_SOAP_SEN_BLADE_CPU_ZONE1, CPU_ZONE, 2},
		{OA_SOAP_SEN_BLADE_CPU1_1, CPU_1, 1},
		{OA_SOAP_SEN_BLADE_CPU2_1, CPU_2, 1},
		{OA_SOAP_SEN_BLADE_CPU3_1, CPU_3, 1},
		{OA_SOAP_SEN_BLADE_CPU4_1, CPU_4, 1},
		{OA_SOAP_SEN_BLADE_DISK_ZONE1, DISK_ZONE, 0},
		{OA_SOAP_SEN_BLADE_MEM_ZONE1, MEMORY_ZONE, 1},
		{OA_SOAP_SEN_TEMP_STATUS, AMBIENT_ZONE, 1}
	},
};

/* Array containing the sensor base number of the thermal sensor types.
 * These base number for sensor are required during sensor read operation
 */
const SaHpiInt32T oa_soap_bld_thrm_sen_base_arr[] = {
	OA_SOAP_SEN_BLADE_SYSTEM_ZONE1,
	OA_SOAP_SEN_BLADE_SYSTEM_ZONE1,
	OA_SOAP_SEN_BLADE_SYSTEM_ZONE1,
	OA_SOAP_SEN_BLADE_SYSTEM_ZONE1,
	OA_SOAP_SEN_BLADE_CPU_ZONE1,
	OA_SOAP_SEN_BLADE_CPU_ZONE1,
	OA_SOAP_SEN_BLADE_CPU_ZONE1,
	OA_SOAP_SEN_BLADE_CPU_ZONE1,
	OA_SOAP_SEN_BLADE_MEM_ZONE1,
	OA_SOAP_SEN_BLADE_MEM_ZONE1,
	OA_SOAP_SEN_BLADE_MEM_ZONE1,
	OA_SOAP_SEN_BLADE_MEM_ZONE1,
	OA_SOAP_SEN_BLADE_DISK_ZONE1,
	OA_SOAP_SEN_BLADE_DISK_ZONE1,
	OA_SOAP_SEN_BLADE_DISK_ZONE1,
	OA_SOAP_SEN_BLADE_DISK_ZONE1,
	OA_SOAP_SEN_BLADE_CPU1_1,
	OA_SOAP_SEN_BLADE_CPU1_1,
	OA_SOAP_SEN_BLADE_CPU1_1,
	OA_SOAP_SEN_BLADE_CPU1_1,
	OA_SOAP_SEN_BLADE_CPU2_1,
	OA_SOAP_SEN_BLADE_CPU2_1,
	OA_SOAP_SEN_BLADE_CPU2_1,
	OA_SOAP_SEN_BLADE_CPU2_1,
	OA_SOAP_SEN_BLADE_CPU3_1,
	OA_SOAP_SEN_BLADE_CPU3_1,
	OA_SOAP_SEN_BLADE_CPU3_1,
	OA_SOAP_SEN_BLADE_CPU3_1,
	OA_SOAP_SEN_BLADE_CPU4_1,
	OA_SOAP_SEN_BLADE_CPU4_1,
	OA_SOAP_SEN_BLADE_CPU4_1,
	OA_SOAP_SEN_BLADE_CPU4_1,
};

/* Array which indicates the power status of the blade in different slots
 */
SaHpiPowerStateT oa_soap_bay_pwr_status[OA_SOAP_C7000_MAX_BLADE] = {
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF,
	SAHPI_POWER_OFF
};
	
