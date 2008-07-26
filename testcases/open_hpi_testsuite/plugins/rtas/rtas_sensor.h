/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *        Renier Morales <renier@openhpi.org>
 *        Daniel de Araujo <ddearauj@us.ibm.com>
 */
 
#ifndef RTAS_SENSOR_H
#define RTAS_SENSOR_H

#include <glib.h>
#include <SaHpi.h>
#include <oh_handler.h>
#include <oh_utils.h>
#include <oh_error.h>
#include <oh_domain.h>
#include <librtas.h>

#define RTAS_SENSORS_PATH	"/proc/device-tree/rtas/rtas-sensors"
#define RTAS_SENSOR_LOCATION    "/proc/device-tree/rtas/ibm,sensor-"
#define MAX_SENSOR_LOCATION_STRING_SIZE 24

#define MILLISECONDS_PER_MINUTE 1000
#define TOKEN_MASK 0xFFFFFFF0
#define TIME_MASK ~TOKEN_MASK


typedef enum rtasSensorStateEnum {
	SENSOR_OK            = 0,
	SENSOR_CRITICAL_LOW  = 9,
	SENSOR_WARNING_LOW   = 10,
	SENSOR_NORMAL        = 11,
	SENSOR_WARNING_HIGH  = 12,
	SENSOR_CRITICAL_HIGH = 13
}rtasSensorState;	
	


struct SensorInfo {
	SaHpiUint32T	 token;
	SaHpiUint32T	 index;
	SaHpiBoolT       enabled;
	SaHpiEventStateT current_state;
	SaHpiUint32T     current_val;
	char 		 sensor_location[MAX_SENSOR_LOCATION_STRING_SIZE];
};



/* Function Protos */
SaErrorT rtas_get_sensor_reading(void *handler,
				 SaHpiResourceIdT resourceid,
				 SaHpiSensorNumT  sensornum,
				 SaHpiSensorReadingT *reading,
				 SaHpiEventStateT *e_state);
				 
SaErrorT rtas_get_sensor_thresholds(void *handler,
				    SaHpiResourceIdT resourceid,
				    SaHpiSensorNumT sensornum,
				    SaHpiSensorThresholdsT *thresholds);	

SaErrorT rtas_set_sensor_thresholds(void *handler,
				       SaHpiResourceIdT resourceid,
				       SaHpiSensorNumT sensornum,
				       const SaHpiSensorThresholdsT *thresholds);
				       
				       
SaErrorT rtas_get_sensor_enable(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiSensorNumT num,
                                   SaHpiBoolT *enable);
				   
SaErrorT rtas_set_sensor_enable(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiSensorNumT num,
                                   SaHpiBoolT enable);
				   
SaErrorT rtas_get_sensor_event_enables(void *hnd,
                                          SaHpiResourceIdT id,
                                          SaHpiSensorNumT num,
                                          SaHpiBoolT *enables);
					  
SaErrorT rtas_set_sensor_event_enables(void *hnd,
                                          SaHpiResourceIdT id,
                                          SaHpiSensorNumT num,
                                          const SaHpiBoolT enables);
					  
SaErrorT rtas_get_sensor_event_masks(void *hnd,
                                        SaHpiResourceIdT id,
                                        SaHpiSensorNumT  num,
                                        SaHpiEventStateT *AssertEventMask,
                                        SaHpiEventStateT *DeassertEventMask);
					
SaErrorT rtas_set_sensor_event_masks(void *hnd,
                                        SaHpiResourceIdT id,
                                        SaHpiSensorNumT num,
                                        SaHpiSensorEventMaskActionT act,
                                        SaHpiEventStateT AssertEventMask,
                                        SaHpiEventStateT DeassertEventMask);	
					
SaErrorT rtas_get_sensor_event_enabled(void *hnd, 
                                          SaHpiResourceIdT id,
                                          SaHpiSensorNumT sensornum,
                                          SaHpiBoolT *enable);
					  
SaErrorT rtas_set_sensor_event_enabled(void *hnd, 
                                          SaHpiResourceIdT id,
                                          SaHpiSensorNumT sensornum,
                                          SaHpiBoolT *enable);									       
				       			       

int rtas_get_sensor_location_code(int token, int index, char *buffer);  


#endif
