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

#ifndef RTAS_DISCOVER_H
#define RTAS_DISCOVER_H

#include <glib.h>
#include <SaHpi.h>
#include <oh_handler.h>
#include <oh_utils.h>
#include <oh_error.h>
#include <oh_domain.h>
#include <rtas_sensor.h>
#include <librtas.h>

#define LSVPD_CMD "/sbin/lsvpd"

/* Enums */
typedef enum rtasSensorTokenEnum {
        RTAS_SECURITY_SENSOR = 1,
        RTAS_RESERVED_SENSOR_2,
        RTAS_THERMAL_SENSOR,
        RTAS_RESERVED_SENSOR_4,
        RTAS_RESERVED_SENSOR_5,
        RTAS_RESERVED_SENSOR_6,
        RTAS_RESERVED_SENSOR_7,
        RTAS_RESERVED_SENSOR_8,
        RTAS_POWER_STATE_SENSOR,
        RTAS_RESERVED_SENSOR_10,
        RTAS_RESERVED_SENSOR_11,
        RTAS_SURVEILLANCE_SENSOR = 9000,
        RTAS_FAN_SENSOR,
        RTAS_VOLTAGE_SENSOR,
        RTAS_CONNECTOR_SENSOR,
        RTAS_POWER_SUPPLY_SENSOR,
        RTAS_GIQ_SENSOR,
        RTAS_SYSTEM_ATTENTION_SENSOR,
        RTAS_IDENTIFY_INDICATOR_SENSOR,
        RTAS_RESERVED_SENSOR_9008,
        RTAS_COMPONENT_RESET_STATE_SENSOR,
        RTAS_OEM_SPECIFIC_SENSOR_START,
        RTAS_OEM_SPECIFIC_SENSOR_END = 9999
} rtasSensorToken;

/* Function Protos */
SaErrorT rtas_discover_resources(void *hnd);

SaErrorT rtas_discover_sensors(struct oh_handler_state *handle,
                               struct oh_event *res_oh_event);

SaErrorT rtas_discover_inventory(struct oh_handler_state *handle,
                                 struct oh_event *res_oh_event);

void populate_rtas_sensor_rec_info(int token, SaHpiSensorRecT *sensor_info);


#endif /* RTAS_DISCOVER_H */
