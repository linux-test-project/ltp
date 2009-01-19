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
 */

#ifndef _OA_SOAP_SENSOR_H
#define _OA_SOAP_SENSOR_H

/* Include files */
#include "oa_soap_utils.h"

/* OA_SOAP_STM_VALID_MASK represents masks for "up critical"
 * and "up major" thresholds
 */
#define OA_SOAP_STM_VALID_MASK (SaHpiSensorThdMaskT)0x30
#define OA_SOAP_STM_UNSPECIFED (SaHpiSensorThdMaskT)0x00

/* Sensor classes of OA SOAP plugin
 *
 * On adding a new sesor class, please update the maximum sensor class value in
 * oa_soap_resources.h. Accordingly, add items to sensor enum mapping and
 * event assert mapping arrays in oa_soap_resources.c
 */
#define OA_SOAP_OPER_CLASS			0
#define OA_SOAP_PRED_FAIL_CLASS			1
#define OA_SOAP_REDUND_CLASS			2
#define OA_SOAP_DIAG_CLASS			3
#define OA_SOAP_TEMP_CLASS      		4
#define OA_SOAP_PWR_STATUS_CLASS		5
#define OA_SOAP_FAN_SPEED_CLASS			6
#define OA_SOAP_PWR_SUBSYS_CLASS		7
#define OA_SOAP_ENC_AGR_OPER_CLASS		8
#define OA_SOAP_ENC_AGR_PRED_FAIL_CLASS		9
#define OA_SOAP_BOOL_CLASS			10
/* For some of the sensors like CPU fault, boolean false indicates sensor
 * state as enabled. BOOLEAN_REVERSE_CLASS represents the sensors whose sensor
 * value indicates reverse meaning
 */
#define OA_SOAP_BOOL_RVRS_CLASS		11
#define OA_SOAP_HEALTH_OPER_CLASS	12
#define OA_SOAP_HEALTH_PRED_FAIL_CLASS	13
#define OA_SOAP_BLADE_THERMAL_CLASS	14

/* Sensor assert states of OA SOAP plugin
 *
 * On adding new sensor event assert state, please update the maximum sensor
 * event array size value in oa_soap_resources.h. Accordingly, add items to
 * sensor event array in global sensor array in oa_soap_resources.c
 */
#define OA_SOAP_SEN_ASSERT_TRUE		0
#define OA_SOAP_SEN_ASSERT_FALSE	1
#define OA_SOAP_SEN_NO_CHANGE		2

/* Index of different thermal events event in event array.
 * These index will be used during the event generation
 */
/* OK to CAUTION */
#define OA_SOAP_TEMP_OK_CAUT	0
/* CAUTION to OK */
#define OA_SOAP_TEMP_CAUT_OK	1
/* CAUTION to CRITICAL */
#define OA_SOAP_TEMP_CAUT_CRIT	2
/* CRITICAL to CAUTION */
#define OA_SOAP_TEMP_CRIT_CAUT	3

/* The below code reduces the code size and eases code maintenance */
#define OA_SOAP_PROCESS_SENSOR_EVENT(sensor_num, sensor_value, reading, \
				     threshold) \
{ \
	rv = oa_soap_proc_sen_evt(oh_handler, resource_id, sensor_num, \
				 (SaHpiInt32T) sensor_value, reading, \
				 threshold); \
	if (rv != SA_OK) { \
		err("processing the sensor event for sensor %x has failed", \
		     sensor_num); \
		return; \
	} \
}

/* Maximum number of enum values for healthStatus field in extraData structure
 */
#define OA_SOAP_MAX_HEALTH_ENUM 8

/* healthStatus string. This is used for parsing the healthStatus field in
 * extraData structure
 */
#define OA_SOAP_HEALTH_STATUS_STR "healthStatus"

/* Different values supported by helthStatus structure in extraData */
enum oa_soap_extra_data_health {
	/* extraData healthStatus UNKNOWN */
	HEALTH_UNKNOWN,
	/* extraData healthStatus OTHER */
	HEALTH_OTHER,
	/* extraData healthStatus OK */
	HEALTH_OK,
	/* extraData healthStatus DEGRADED */
	HEALTH_DEGRAD,
	/* extraData healthStatus STRESSED */
	HEALTH_STRESSED,
	/* extraData healthStatus PREDICTIVE_FAILURE */
	HEALTH_PRED_FAIL,
	/* extraData healthStatus ERROR */
	HEALTH_ERROR,
	/* extraData healthStatus NON_RECOVERABLERROR */
	HEALTH_NRE
};

/* Maximum number of supported fields in diagnosticChecksEx structure
 *
 * When a new field is added to diagnosticChecksEx structure, please update the
 * #define and enum oa_soap_diag_ex in oa_soap_sensor.h, also update
 * oa_soap_diag_ex_arr in oa_soap_resources.c
 */
#define OA_SOAP_MAX_DIAG_EX 17

/* Possible fields supported by diagnosticChecksEx structure 
 *
 * When a new field is added to diagnosticChecksEx structure, please update the
 * #define and enum oa_soap_diag_ex in oa_soap_sensor.h, also update
 * oa_soap_diag_ex_arr in oa_soap_resources.c
 */

enum oa_soap_diag_ex {
	/* diagnosticChecksEx deviceMissing */
	DIAG_EX_DEV_MISS,
	/* diagnosticChecksEx devicePowerSequence */
	DIAG_EX_DEV_PWR_SEQ,
	/* diagnosticChecksEx deviceBonding */
	DIAG_EX_DEV_BOND,
	/* diagnosticChecksEx profileUnassignedError */
	DIAG_EX_PROF_UNASSIGN_ERR,
	/* diagnosticChecksEx deviceNotSupported */
	DIAG_EX_DEV_NOT_SUPPORT,
	/* diagnosticChecksEx networkConfiguration */
	DIAG_EX_NET_CONFIG,
	/* diagnosticChecksEx tooLowPowerRequest */
	DIAG_EX_TOO_LOW_PWR_REQ,
	/* diagnosticChecksEx callHP */
	DIAG_EX_CALL_HP,
	/* diagnosticChecksEx deviceInformational */
	DIAG_EX_DEV_INFO,
	/* diagnosticChecksEx storageDeviceMissing */
	DIAG_EX_STORAGE_DEV_MISS,
	/* diagnosticChecksEx firmwareMismatch */
	DIAG_EX_FW_MISMATCH,
	/* diagnosticChecksEx enclosureIdMismatch */
	DIAG_EX_ENC_ID_MISMATCH,
	/* POWERDELAY_IN_USE is not used as sensor. 'Power delay in use' does
	 * not indicates any failure.
	 */
	DIAG_EX_POWERDELAY_IN_USE,
	/* diagnosticChecksEx deviceMixMatch */
	DIAG_EX_DEV_MIX_MATCH,
	/* diagnosticChecksEx gprcapError */
	DIAG_EX_GRPCAP_ERR,
	/* diagnosticChecksEx imlRecordedError */
	DIAG_EX_IML_ERR,
	/* diagnosticChecksEx duplicateManagementIpAddress */
	DIAG_EX_DUP_MGMT_IP_ADDR
};

/* Maximum number of possible sensor strings provided 
 * by getBladeThermalInfoArray soap call 
 */
#define OA_SOAP_MAX_THRM_SEN	9

/* Enum values for the sensor description strings provide by 
 * getBladeThermalInfoArray soap call
 */
enum oa_soap_thermal_sen {
	SYSTEM_ZONE,
	CPU_ZONE,
	CPU_1,
	CPU_2,
	CPU_3,
	CPU_4,
	DISK_ZONE,
	MEMORY_ZONE,
	AMBIENT_ZONE
};

/* Define the sensor number range for blade extra thermal sensors */

#define OA_SOAP_BLD_THRM_SEN_START	0x02e
#define OA_SOAP_BLD_THRM_SEN_END	0x04d

/* Structure required for building thermal sensor when server blade is off */
struct oa_soap_static_thermal_sensor_info {
	SaHpiSensorNumT base_sen_num; /* Base sensor number for sensor type */
	enum oa_soap_thermal_sen sensor; /* thermal sensor type */
	SaHpiInt32T sensor_count; /* Number of sensor to be created of 
				   * above thermal sensor type 
				   */
};

/* Structure containing thermal sensor information data*/
struct oa_soap_thrm_sen_data {
	SaHpiRdrT rdr_num;
	SaHpiSensorNumT sen_delta; /* Delta difference of the sensor rdr number
				    * from the base sensor number of particular
				    * sensor type
				    */
};
	
/* Structure for sensor reading */
struct oa_soap_sensor_reading_data {
        SaHpiSensorReadingT data;
        SaHpiEventStateT event;
};

/* Declaration of sensor specific information structure */
struct oa_soap_sensor_info {
        SaHpiEventStateT current_state;
        SaHpiEventStateT previous_state;
        SaHpiBoolT sensor_enable;
        SaHpiBoolT event_enable;
        SaHpiEventStateT assert_mask;
        SaHpiEventStateT deassert_mask;
        SaHpiSensorReadingT sensor_reading;
        SaHpiSensorThresholdsT threshold;
};

/* Declaration of the functions related to sensor */
SaErrorT oa_soap_get_sensor_reading(void *oh_handler,
                                   SaHpiResourceIdT resource_id,
                                   SaHpiSensorNumT num,
                                   SaHpiSensorReadingT *data,
                                   SaHpiEventStateT    *state);

SaErrorT oa_soap_get_sensor_thresholds(void *oh_handler,
                                      SaHpiResourceIdT resource_id,
                                      SaHpiSensorNumT num,
                                      SaHpiSensorThresholdsT *thres);

SaErrorT oa_soap_set_sensor_thresholds(void *oh_handler,
                                      SaHpiResourceIdT resource_id,
                                      SaHpiSensorNumT num,
                                      const SaHpiSensorThresholdsT *thres);

SaErrorT oa_soap_get_sensor_event_enable(void *oh_handler,
                                         SaHpiResourceIdT resource_id,
                                         SaHpiSensorNumT num,
                                         SaHpiBoolT *enable);

SaErrorT oa_soap_set_sensor_event_enable(void *oh_handler,
                                         SaHpiResourceIdT resource_id,
                                         SaHpiSensorNumT num,
                                         const SaHpiBoolT enable);

SaErrorT oa_soap_get_sensor_enable(void *oh_handler,
                                  SaHpiResourceIdT resource_id,
                                  SaHpiSensorNumT num,
                                  SaHpiBoolT *enable);

SaErrorT oa_soap_set_sensor_enable(void *oh_handler,
                                  SaHpiResourceIdT resource_id,
                                  SaHpiSensorNumT num,
                                  SaHpiBoolT enable);

SaErrorT oa_soap_get_sensor_event_masks(void *oh_handler,
                                       SaHpiResourceIdT resource_id,
                                       SaHpiSensorNumT num,
                                       SaHpiEventStateT *assert,
                                       SaHpiEventStateT *deassert);

SaErrorT oa_soap_set_sensor_event_masks(void *oh_handler,
                                       SaHpiResourceIdT resource_id,
                                       SaHpiSensorNumT num,
                                       SaHpiSensorEventMaskActionT act,
                                       SaHpiEventStateT assert,
                                       SaHpiEventStateT deassert);

SaErrorT update_sensor_rdr(struct oh_handler_state *oh_handler,
                           SaHpiResourceIdT resource_id,
                           SaHpiSensorNumT num,
                           SaHpiRptEntryT *rpt,
                           struct oa_soap_sensor_reading_data *data);

SaErrorT generate_sensor_enable_event(void *oh_handler,
                                      SaHpiSensorNumT rdr_num,
                                      SaHpiRptEntryT *rpt,
                                      SaHpiRdrT *rdr,
                                      struct oa_soap_sensor_info *sensor_info);

SaErrorT generate_sensor_assert_thermal_event(void *oh_handler,
                                              SaHpiSensorNumT rdr_num,
                                              SaHpiRptEntryT *rpt,
                                              SaHpiRdrT *rdr,
                                              SaHpiSensorReadingT
                                                      current_reading,
                                              SaHpiSeverityT event_severity,
                                              struct oa_soap_sensor_info
                                                      *sensor_info);

SaErrorT generate_sensor_deassert_thermal_event(void *oh_handler,
                                                SaHpiSensorNumT rdr_num,
                                                SaHpiRptEntryT *rpt,
                                                SaHpiRdrT *rdr,
                                                SaHpiSensorReadingT
                                                        current_reading,
                                                SaHpiSeverityT event_severity,
                                                struct oa_soap_sensor_info
                                                        *sensor_info);

SaErrorT check_and_deassert_event(struct oh_handler_state *oh_handler,
                                  SaHpiResourceIdT resource_id,
                                  SaHpiRdrT *rdr,
                                  struct oa_soap_sensor_info *sensor_info);

SaErrorT oa_soap_build_sen_rdr(struct oh_handler_state *oh_handler,
			       SaHpiResourceIdT resource_id,
			       SaHpiRdrT *rdr,
			       struct oa_soap_sensor_info **sensor_info,
			       SaHpiSensorNumT sensor_num);

SaErrorT oa_soap_map_sen_val(struct oa_soap_sensor_info *sensor_info,
			     SaHpiSensorNumT sensor_num,
			     SaHpiInt32T sensor_value,
			     SaHpiInt32T *sensor_status);

SaErrorT oa_soap_proc_sen_evt(struct oh_handler_state *oh_handler,
			      SaHpiResourceIdT resource_id,
			      SaHpiSensorNumT sen_num,
			      SaHpiInt32T sensor_value,
			      SaHpiFloat64T trigger_reading,
			      SaHpiFloat64T trigger_threshold);

SaErrorT oa_soap_map_thresh_resp(SaHpiRdrT *rdr,
				 void *response,
				 SaHpiBoolT event_support,
				 struct oa_soap_sensor_info
				 *sensor_info);

SaErrorT oa_soap_assert_sen_evt(struct oh_handler_state *oh_handler,
				SaHpiRptEntryT *rpt,
				GSList *assert_sensor_list);

SaErrorT oa_soap_get_bld_thrm_sen_data(SaHpiSensorNumT sen_num,
				       struct bladeThermalInfoArrayResponse
							response,
				       struct bladeThermalInfo *bld_thrm_info);

#endif
