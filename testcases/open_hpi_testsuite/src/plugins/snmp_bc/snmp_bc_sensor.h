/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <sesherman@us.ibm.com>
 */

#ifndef SNMP_BC_SENSOR_H
#define SNMP_BC_SENSOR_H

SaErrorT snmp_bc_get_sensor_data(void *hnd,
				 SaHpiResourceIdT id,
				 SaHpiSensorNumT num,
				 SaHpiSensorReadingT *data);

SaErrorT snmp_bc_get_sensor_thresholds(void *hnd,
				       SaHpiResourceIdT id,
				       SaHpiSensorNumT num,
				       SaHpiSensorThresholdsT *thres);

SaErrorT snmp_bc_set_sensor_thresholds(void *hnd,
				       SaHpiResourceIdT id,
				       SaHpiSensorNumT num,
				       const SaHpiSensorThresholdsT *thres);


SaErrorT snmp_bc_get_sensor_event_enables(void *hnd,
					  SaHpiResourceIdT id,
					  SaHpiSensorNumT num,
					  SaHpiSensorEvtEnablesT *enables);

SaErrorT snmp_bc_set_sensor_event_enables(void *hnd,
					  SaHpiResourceIdT id,
					  SaHpiSensorNumT num,
					  const SaHpiSensorEvtEnablesT *enables);

#endif /* SNMP_BC_SENSOR_H */
