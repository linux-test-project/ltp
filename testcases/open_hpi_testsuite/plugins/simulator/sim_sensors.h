/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *	  Christina Hernandez <hernanc@us.ibm.com>
 *        W. David Ashley <dashley@us.ibm.com>
 *	  Renier Morales <renier@openhpi.org>
 */


#ifndef __SIM_SENSORS_H
#define __SIM_SENSORS_H

#include <oh_event.h>
#include <oh_handler.h>

#define SIM_MAX_EVENTS_PER_SENSOR 24
#define SIM_MAX_READING_MAPS_PER_SENSOR 3

/* Includes an ending NULL entry */
#define SIM_MAX_SENSOR_EVENT_ARRAY_SIZE  (SIM_MAX_EVENTS_PER_SENSOR + 1)
#define SIM_MAX_SENSOR_READING_MAP_ARRAY_SIZE (SIM_MAX_READING_MAPS_PER_SENSOR + 1)

/********************
 * Sensor Definitions
 ********************/

struct SimSensorThresholdOids {
       const char *LowMinor;
       const char *LowMajor;
       const char *LowCritical;
       const char *UpMinor;
       const char *UpMajor;
       const char *UpCritical;
       const char *PosThdHysteresis;
       const char *NegThdHysteresis;
       const char *TotalPosThdHysteresis;
       const char *TotalNegThdHysteresis;
};

struct SimSensorWritableThresholdOids {
        const char *LowMinor;
        const char *LowMajor;
        const char *LowCritical;
        const char *UpMinor;
        const char *UpMajor;
        const char *UpCritical;
        const char *PosThdHysteresis;
        const char *NegThdHysteresis;
};

struct sensor_event_map {
      char *event;
      SaHpiBoolT event_assertion;
      SaHpiBoolT event_res_failure;
      SaHpiBoolT event_res_failure_unexpected;
      SaHpiEventStateT event_state;
      SaHpiEventStateT recovery_state;
};

struct sensor_reading_map {
	int num;
	SaHpiSensorRangeT rangemap;
	SaHpiEventStateT state;
};

struct SensorInfo {
        SaHpiEventStateT cur_state; /* This really records the last state read from the SEL */
                           /* Which probably isn't the current state of the sensor */
	SaHpiBoolT sensor_enabled;
	SaHpiBoolT events_enabled;
	SaHpiEventStateT assert_mask;
	SaHpiEventStateT deassert_mask;
	struct sensor_event_map event_array[SIM_MAX_SENSOR_EVENT_ARRAY_SIZE];
	struct sensor_reading_map reading2event[SIM_MAX_SENSOR_READING_MAP_ARRAY_SIZE];
	SaHpiSensorReadingT reading;
	SaHpiSensorThresholdsT thres;
};


/*
struct SensorMoreInfo {
	SaHpiSensorReadingT reading;
	SaHpiSensorThresholdsT thres;
};
*/


struct sim_sensor {
      /* Usually sensor.Num = index; index is used to search thru sensor arrays. It allows
         sensor.Num to be independent from array index (e.g. for aggregate sensors) */
      int index;
      SaHpiSensorRecT sensor;
      struct SensorInfo sensor_info;
      const char *comment;
};


SaErrorT sim_discover_chassis_sensors(struct oh_handler_state *state,
                                      struct oh_event *e);
SaErrorT sim_discover_cpu_sensors(struct oh_handler_state *state,
                                  struct oh_event *e);
SaErrorT sim_discover_dasd_sensors(struct oh_handler_state *state,
                                   struct oh_event *e);
SaErrorT sim_discover_hs_dasd_sensors(struct oh_handler_state *state,
                                      struct oh_event *e);
SaErrorT sim_discover_fan_sensors(struct oh_handler_state *state,
                                  struct oh_event *e);


#endif

