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
 */

#ifndef _OA_SOAP_SENSOR_H
#define _OA_SOAP_SENSOR_H

/* Include files */
#include "oa_soap_utils.h"

/* String for the sensor RDR */
#define ENCLOSURE_THERMAL_STRING "Enclosure Temperature"
#define OA_THERMAL_STRING "OA Temperature"
#define SERVER_THERMAL_STRING "Server Board Temperature"
#define SERVER_THERMAL_STRING "Server Board Temperature"
#define SERVER_POWER_STRING "Server Board Power Consumed"
#define INTERCONNECT_THERMAL_STRING "InterConnect Temperature"
#define FAN_SPEED_STRING "Fan Speed"
#define FAN_POWER_STRING "Fan Power Consumed"
#define POWER_SUPPLY_POWER_STRING "Power Output"
#define POWER_SUBSYSTEM_IN_POWER "Power Subsystem Input Power"
#define POWER_SUBSYSTEM_OUT_POWER "Power Subsystem Output Power"
#define POWER_SUBSYSTEM_POWER_CONSUMED "Power Subsystem Power Consumed"
#define POWER_SUBSYSTEM_POWER_CAPACITY "Power Subsystem Power Capacity"

/* OA_SOAP_STM_VALID_MASK represents masks for "up critical"
 * and "up major" thresholds
 */
#define OA_SOAP_STM_VALID_MASK (SaHpiSensorThdMaskT)0x30
#define OA_SOAP_STM_UNSPECIFED (SaHpiSensorThdMaskT)0x00

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

SaErrorT build_enclosure_thermal_sensor_rdr(struct oh_handler_state *oh_handler,
                                            SOAP_CON *con,
                                            SaHpiInt32T bay_number,
                                            SaHpiRdrT *rdr,
                                            struct oa_soap_sensor_info
                                                    **sensor_info);

SaErrorT build_oa_thermal_sensor_rdr(struct oh_handler_state *oh_handler,
                                     SOAP_CON *con,
                                     SaHpiInt32T bay_number,
                                     SaHpiRdrT *rdr,
                                     struct oa_soap_sensor_info **sensor_info);

SaErrorT build_server_thermal_sensor_rdr(struct oh_handler_state *oh_handler,
                                         SOAP_CON *con,
                                         SaHpiInt32T bay_number,
                                         SaHpiRdrT *rdr,
                                         struct oa_soap_sensor_info
                                                 **sensor_info);

SaErrorT build_inserted_server_thermal_sensor_rdr(struct oh_handler_state
                                                          *oh_handler,
                                                  SOAP_CON *con,
                                                  SaHpiInt32T bay_number,
                                                  SaHpiRdrT *rdr,
                                                  struct oa_soap_sensor_info
                                                          **sensor_info);

SaErrorT build_server_power_sensor_rdr(struct oh_handler_state *oh_handler,
                                       SOAP_CON *con,
                                       SaHpiInt32T bay_number,
                                       SaHpiRdrT *rdr,
                                       struct oa_soap_sensor_info
                                               **sensor_info);

SaErrorT build_interconnect_thermal_sensor_rdr(struct oh_handler_state
                                                       *oh_handler,
                                               SOAP_CON *con,
                                               SaHpiInt32T bay_number,
                                               SaHpiRdrT *rdr,
                                               struct oa_soap_sensor_info
                                                       **sensor_info);

SaErrorT build_fan_speed_sensor_rdr(struct oh_handler_state *oh_handler,
                                    SOAP_CON *con,
                                    SaHpiInt32T bay_number,
                                    SaHpiRdrT *rdr,
                                    struct oa_soap_sensor_info **sensor_info);

SaErrorT build_fan_power_sensor_rdr(struct oh_handler_state *oh_handler,
                                    SOAP_CON *con,
                                    SaHpiInt32T bay_number,
                                    SaHpiRdrT *rdr,
                                    struct oa_soap_sensor_info **sensor_info);

SaErrorT build_ps_power_sensor_rdr(struct oh_handler_state *oh_handler,
                                   SOAP_CON *con,
                                   SaHpiInt32T powerNumber,
                                   SaHpiRdrT *rdr,
                                   struct oa_soap_sensor_info **sensor_info);

SaErrorT build_ps_subsystem_input_power_sensor_rdr(struct oh_handler_state
                                                           *oh_handler,
                                                   SaHpiRdrT *rdr,
                                                   struct oa_soap_sensor_info
                                                           **sensor_info);

SaErrorT build_ps_subsystem_output_power_sensor_rdr(struct oh_handler_state
                                                            *oh_handler,
                                                    SaHpiRdrT *rdr,
                                                    struct oa_soap_sensor_info
                                                            **sensor_info);

SaErrorT build_ps_subsystem_power_consumed_sensor_rdr(struct oh_handler_state
                                                              *oh_handler,
                                                      SaHpiRdrT *rdr,
                                                      struct oa_soap_sensor_info
                                                              **sensor_info);

SaErrorT build_ps_subsystem_power_capacity_sensor_rdr(struct oh_handler_state
                                                              *oh_handler,
                                                      SaHpiRdrT *rdr,
                                                      struct oa_soap_sensor_info
                                                              **sensor_info);

SaErrorT build_thermal_sensor_info(struct thermalInfo response,
                                   struct oa_soap_sensor_info **sensor_info,
                                   SaHpiBoolT event_flag);

SaErrorT build_fan_speed_sensor_info(struct fanInfo response,
                                     struct oa_soap_sensor_info **sensor_info);

SaErrorT build_ps_sensor_info(struct powerSupplyInfo response,
                              struct oa_soap_sensor_info **sensor_info);

SaErrorT build_power_sensor_info(struct oa_soap_sensor_info **sensor_info);

SaErrorT update_sensor_rdr(struct oh_handler_state *oh_handler,
                           SaHpiResourceIdT resource_id,
                           SaHpiSensorNumT num,
                           SaHpiRptEntryT *rpt,
                           struct oa_soap_sensor_reading_data *data);

SaErrorT update_ps_subsystem_sensor_rdr(struct oh_handler_state *oh_handler,
                                        SaHpiResourceIdT resource_id,
                                        SaHpiSensorNumT rdr_num,
                                        SaHpiRdrT *rdr);

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

#endif
