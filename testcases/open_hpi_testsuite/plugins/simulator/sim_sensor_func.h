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
 *        W. David Ashley <dashley@us.ibm.com>
 */

#ifndef SIM_SENSOR_FUNC_H
#define SIM_SENSOR_FUNC_H


SaErrorT sim_get_sensor_reading(void *hnd, SaHpiResourceIdT id, SaHpiSensorNumT num, SaHpiSensorReadingT *data, SaHpiEventStateT *state);

SaErrorT sim_get_sensor_thresholds(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorThresholdsT *thres);

SaErrorT sim_set_sensor_thresholds(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       const SaHpiSensorThresholdsT *thres);

SaErrorT sim_get_sensor_enable(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiSensorNumT sid,
				   SaHpiBoolT *enable);

SaErrorT sim_set_sensor_enable(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiSensorNumT sid,
				   const SaHpiBoolT enable);

SaErrorT sim_get_sensor_event_enable(void *hnd,
					 SaHpiResourceIdT rid,
					 SaHpiSensorNumT sid,
					 SaHpiBoolT *enable);

SaErrorT sim_set_sensor_event_enable(void *hnd,
					 SaHpiResourceIdT rid,
					 SaHpiSensorNumT sid,
					 const SaHpiBoolT enable);

SaErrorT sim_get_sensor_event_masks(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiSensorNumT sid,
					SaHpiEventStateT *AssertEventMask,
					SaHpiEventStateT *DeassertEventMask);

SaErrorT sim_set_sensor_event_masks(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiSensorNumT sid,
					SaHpiSensorEventMaskActionT act,
					const SaHpiEventStateT AssertEventMask,
					const SaHpiEventStateT DeassertEventMask);

SaErrorT sim_get_sensor_eventstate(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorReadingT *reading,
				       SaHpiEventStateT *state);

SaErrorT sim_set_threshold_reading(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       const SaHpiSensorReadingT *reading);
#endif
