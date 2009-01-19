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
 */

#ifndef _OA_SOAP_RESOURCES_H
#define _OA_SOAP_RESOURCES_H

/* Include files */
#include <SaHpiOaSoap.h>
#include "oa_soap_sensor.h"

/* Maximum sensor classes in OA SOAP
 * 
 * On adding a new sensor class in oa_soap_sensor.h, please change the maximum
 * sensor class value. Accordingly, add new sensor class support in global
 * sensor enum value mapping array and global sensor event assert state mapping
 * array in oa_soap_resources.c
 */
#define OA_SOAP_MAX_SEN_CLASS 14

/* Maximum sensor enum values in OA SOAP
 *
 * If a new sensor added in include/SaHpiOaSoap.h has more enum values, then
 * change the maximum enum. Accordingly, add new sensor enum values to global
 * sensor enum value mapping array and global sensor event assert state mapping
 * array in oa_soap_resources.c
 */
#define OA_SOAP_MAX_ENUM 21

/* Maximum sensor event array size
 *
 * Increase the event array size if a new sensor supports more number of sensor
 * event payload. Accordingly, increase the sensor_event arry in global sensor
 * array in oa_soap_resources.c
 */
#define OA_SOAP_MAX_SEN_EVT 4

/* Structure for storing the sensor RDR and event information */
struct oa_soap_sensor {
	SaHpiSensorRecT sensor;
	struct oa_soap_sensor_info sensor_info;
	SaHpiInt32T sensor_class;
	SaHpiEventT sen_evt[OA_SOAP_MAX_SEN_EVT];
	const char *comment;
};

/* Structure for storing the control RDR */
struct oa_soap_control {
	SaHpiCtrlRecT control;
	const char *comment;
};

#define OA_SOAP_MAX_BLD_TYPE 16

/* Enum for possible cclass blade types */
enum oa_soap_blade_type {
	BL260C,
	BL2x220C,
	BL460C,
	BL465C,
	BL480C,
	BL495C,
	BL680C,
	BL685C,
	BL860C,
	BL870C,
	NB50000C,
	AMC,
	STORAGE,
	TAPE,
	SAN,
	OTHER_BLADE_TYPE,
};

extern const SaHpiInt32T oa_soap_sen_val_map_arr[OA_SOAP_MAX_SEN_CLASS]
						[OA_SOAP_MAX_ENUM];
extern const SaHpiInt32T oa_soap_sen_assert_map_arr[OA_SOAP_MAX_SEN_CLASS]
						   [OA_SOAP_MAX_ENUM];
extern const struct oa_soap_sensor oa_soap_sen_arr[];
extern const struct oa_soap_control oa_soap_cntrl_arr[];
extern const SaHpiRptEntryT oa_soap_rpt_arr[];
extern const struct oa_soap_inv_rdr oa_soap_inv_arr[];
extern const struct oa_soap_fz_map oa_soap_fz_map_arr[][OA_SOAP_MAX_FAN];
extern const char *oa_soap_health_arr[];
extern const char *oa_soap_diag_ex_arr[];
extern const char *oa_soap_thermal_sensor_string[];
extern const struct oa_soap_static_thermal_sensor_info
		oa_soap_static_thrm_sen_config[OA_SOAP_MAX_BLD_TYPE]
						[OA_SOAP_MAX_THRM_SEN];
extern const char *oa_soap_bld_type_str[];
extern const SaHpiInt32T oa_soap_bld_thrm_sen_base_arr[];
extern SaHpiPowerStateT oa_soap_bay_pwr_status[OA_SOAP_C7000_MAX_BLADE];

#endif
