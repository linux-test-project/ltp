/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <sesherman@us.ibm.com>
 */

#ifndef __SNMP_BC_SENSOR_H
#define __SNMP_BC_SENSOR_H

SaErrorT snmp_bc_get_sensor_reading(void *hnd,
				    SaHpiResourceIdT rid,
				    SaHpiSensorNumT sid,
				    SaHpiSensorReadingT *reading,
				    SaHpiEventStateT *state);

SaErrorT snmp_bc_get_sensor_thresholds(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorThresholdsT *thres);

SaErrorT snmp_bc_set_sensor_thresholds(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       const SaHpiSensorThresholdsT *thres);

SaErrorT snmp_bc_get_sensor_enable(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiSensorNumT sid,
				   SaHpiBoolT *enable);

SaErrorT snmp_bc_set_sensor_enable(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiSensorNumT sid,
				   const SaHpiBoolT enable);

SaErrorT snmp_bc_get_sensor_event_enable(void *hnd,
					 SaHpiResourceIdT rid,
					 SaHpiSensorNumT sid,
					 SaHpiBoolT *enable);

SaErrorT snmp_bc_set_sensor_event_enable(void *hnd,
					 SaHpiResourceIdT rid,
					 SaHpiSensorNumT sid,
					 const SaHpiBoolT enable);
					  
SaErrorT snmp_bc_get_sensor_event_masks(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiSensorNumT sid,
					SaHpiEventStateT *AssertEventMask,
					SaHpiEventStateT *DeassertEventMask);

SaErrorT snmp_bc_set_sensor_event_masks(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiSensorNumT sid,
					SaHpiSensorEventMaskActionT act,
					const SaHpiEventStateT AssertEventMask,
					const SaHpiEventStateT DeassertEventMask);

SaErrorT snmp_bc_get_sensor_eventstate(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorReadingT *reading,
				       SaHpiEventStateT *state);

SaErrorT snmp_bc_get_sensor_oid_reading(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiSensorNumT sid,
					const char *raw_oid,
					SaHpiSensorReadingT *reading);

SaErrorT snmp_bc_set_threshold_reading(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       const char *raw_oid,
				       const SaHpiSensorReadingT *reading);
				       
SaErrorT snmp_bc_get_logical_sensors(void *hnd,
				     SaHpiResourceIdT rid,
				     SaHpiSensorNumT sid,
				     SaHpiSensorReadingT *reading);
				       
SaErrorT snmp_bc_set_slot_state_sensor(void *hnd, 
				       struct oh_event *e, 
				       SaHpiEntityPathT *slot_ep);
					
SaErrorT snmp_bc_reset_slot_state_sensor(void *hnd, 
					 SaHpiEntityPathT *slot_ep);
					
SaErrorT snmp_bc_set_resource_slot_state_sensor(void *hnd, 
						struct oh_event *e,
						guint resourcewidth);
					
SaErrorT snmp_bc_reset_resource_slot_state_sensor(void *hnd, 
						  SaHpiRptEntryT *res);					
					
SaErrorT snmp_bc_get_slot_state_sensor(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorReadingT *reading);

SaErrorT snmp_bc_get_slot_power_sensor(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorReadingT *reading);
#endif
