/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#ifndef __SAHPIBLADECENTER_H
#define __SAHPIBLADECENTER_H

/* Slot Entity Path Numbers */
#define BLADECENTER_SWITCH_SLOT            SAHPI_ENT_CHASSIS_SPECIFIC + 0x10
#define BLADECENTER_POWER_SUPPLY_SLOT      SAHPI_ENT_CHASSIS_SPECIFIC + 0x11
#define BLADECENTER_PERIPHERAL_BAY_SLOT    SAHPI_ENT_CHASSIS_SPECIFIC + 0x12
#define BLADECENTER_SYS_MGMNT_MODULE_SLOT  SAHPI_ENT_CHASSIS_SPECIFIC + 0x13
#define BLADECENTER_BLOWER_SLOT            SAHPI_ENT_CHASSIS_SPECIFIC + 0x14
#define BLADECENTER_ALARM_PANEL_SLOT       SAHPI_ENT_CHASSIS_SPECIFIC + 0x15
#define BLADECENTER_MUX_SLOT               SAHPI_ENT_CHASSIS_SPECIFIC + 0x16
#define BLADECENTER_CLOCK_SLOT             SAHPI_ENT_CHASSIS_SPECIFIC + 0x17

/* Slot Sensor Numbers */
#define BLADECENTER_SENSOR_NUM_SLOT_STATE  	(SaHpiSensorNumT) 0x1010
#define BLADECENTER_SENSOR_NUM_MAX_POWER  	(SaHpiSensorNumT) 0x1012
#define BLADECENTER_SENSOR_NUM_ASSIGNED_POWER  	(SaHpiSensorNumT) 0x1011
#define BLADECENTER_SENSOR_NUM_MIN_POWER  	(SaHpiSensorNumT) 0x1013

#endif

