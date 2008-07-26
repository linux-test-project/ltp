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
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 */

#ifndef _OA_SOAP_DISCOVER_H
#define _OA_SOAP_DISCOVER_H

/* Include files */
#include "oa_soap_sensor.h"
#include "oa_soap_control.h"
#include "oa_soap_event.h"

/* The OA does not gives the resource names for power supply, power subsystem
 * and OA. The names will be used for creating the tag in RPT entry
 */
#define POWER_SUPPLY_NAME "Power Supply Unit"
#define POWER_SUBSYSTEM_NAME "Power Subsystem"
#define OA_NAME "Onboard Administrator"
#define MAX_SERIAL_NUM_LENGTH 32
#define MAX_NAME_LEN 64

#define CISCO "CISCO"                   /* Identifies the Cisco interconnects */

/* Function prototypes */

SaErrorT oa_soap_discover_resources(void *oh_handler);

SaErrorT discover_oa_soap_system(struct oh_handler_state *oh_handler);

SaErrorT build_enclosure_info(struct oh_handler_state *oh_handler,
                              struct enclosureInfo *info);

SaErrorT build_enclosure_rpt(struct oh_handler_state *oh_handler,
                             char *name,
                             SaHpiResourceIdT *resource_id);

SaErrorT build_enclosure_rdr(struct oh_handler_state *oh_handler,
                             SOAP_CON *con,
                             struct enclosureInfo *response,
                             SaHpiResourceIdT resource_id);

SaErrorT discover_enclosure(struct oh_handler_state *oh_handler);

SaErrorT build_oa_rpt(struct oh_handler_state *oh_handler,
                      SaHpiInt32T bay_number,
                      SaHpiResourceIdT *resource_id);

SaErrorT build_oa_rdr(struct oh_handler_state *oh_handler,
                      SOAP_CON *con,
                      struct oaInfo *response,
                      SaHpiResourceIdT resource_id);

SaErrorT discover_oa(struct oh_handler_state *oh_handler);

SaErrorT build_server_rpt(struct oh_handler_state *oh_handler,
                          SOAP_CON *con,
                          struct bladeInfo *response,
                          SaHpiResourceIdT *resource_id);

SaErrorT build_server_rdr(struct oh_handler_state *oh_handler,
                          SOAP_CON *con,
                          SaHpiInt32T bay_number,
                          SaHpiResourceIdT resource_id);

SaErrorT discover_server(struct oh_handler_state *oh_handler);

SaErrorT build_interconnect_rpt(struct oh_handler_state *oh_handler,
                                SOAP_CON *con,
                                char *name,
                                SaHpiInt32T bay_number,
                                SaHpiResourceIdT *resource_id);

SaErrorT build_interconnect_rdr(struct oh_handler_state *oh_handler,
                                SOAP_CON *con,
                                SaHpiInt32T bay_number,
                                SaHpiResourceIdT resource_id);

SaErrorT discover_interconnect(struct oh_handler_state *oh_handler);

SaErrorT build_fan_rpt(struct oh_handler_state *oh_handler,
                       char *name,
                       SaHpiInt32T bay_number,
                       SaHpiResourceIdT *resource_id);

SaErrorT build_fan_rdr(struct oh_handler_state *oh_handler,
                       SOAP_CON *con,
                       struct fanInfo *response,
                       SaHpiResourceIdT resource_id);

SaErrorT discover_fan(struct oh_handler_state *oh_handler);

SaErrorT build_power_subsystem_rpt(struct oh_handler_state *oh_handler,
                                   char *name,
                                   SaHpiResourceIdT *resource_id);

SaErrorT build_power_subsystem_rdr(struct oh_handler_state *oh_handler,
                                   SaHpiResourceIdT resource_id);

SaErrorT discover_power_subsystem(struct oh_handler_state *oh_handler);

SaErrorT build_power_supply_rpt(struct oh_handler_state *oh_handler,
                                char *name,
                                SaHpiInt32T bay_number,
                                SaHpiResourceIdT *resource_id);

SaErrorT build_power_supply_rdr(struct oh_handler_state *oh_handler,
                                SOAP_CON *con,
                                struct powerSupplyInfo *response,
                                SaHpiResourceIdT resource_id);

SaErrorT discover_power_supply(struct oh_handler_state *oh_handler);

#endif
