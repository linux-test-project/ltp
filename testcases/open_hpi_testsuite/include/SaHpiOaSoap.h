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

#ifndef __SAHPIOASOAP_H
#define __SAHPIOASOAP_H

/* Sensor Numbers used in OA SOAP plugin 
 *
 * On adding new sensor, the following data structures may require updation.
 * Please update accordingly.
 * 1. New sensor class in plugin/oa_soap/oa_soap_sensor.h
 * 2. Max sensor class in plugin/oa_soap/oa_soap_resources.h
 * 3. New sensor event assert state in plugin/oa_soap/oa_soap_sensor.h
 * 4. Max sensor event assert state mapping array in
 *    plugin/oa_soap/oa_soap_resources.c
 * 5. Max sensor enum value mapping array in plugin/oa_soap/oa_soap_resources.c
 * 6. Global sensor array in plugin/oa_soap/oa_soap_resources.c
 * 7. Sensor event array in global sensor array in
 *    plugin/oa_soap/oa_soap_resources.c
 * 8. Global sensor enum value mapping array in
 *    plugin/oa_soap/oa_soap_resources.c
 * 9. Global sensor event assert state mapping array in
 *    plugin/oa_soap/oa_soap_resources.c
 */

/* Operational status sensor */
#define OA_SOAP_SEN_OPER_STATUS			(SaHpiSensorNumT) 0x000 
/* Predictive faliure sensor */
#define OA_SOAP_SEN_PRED_FAIL 			(SaHpiSensorNumT) 0x001
/* Thermal reading sensor */
#define OA_SOAP_SEN_TEMP_STATUS			(SaHpiSensorNumT) 0x002
/* Redundancy sensor */
#define OA_SOAP_SEN_REDUND			(SaHpiSensorNumT) 0x003
/* Fan speed sensor */
#define OA_SOAP_SEN_FAN_SPEED			(SaHpiSensorNumT) 0x004
/* Power reading sensor */
#define OA_SOAP_SEN_PWR_STATUS			(SaHpiSensorNumT) 0x005
/* Internal data error sensor */
#define OA_SOAP_SEN_INT_DATA_ERR		(SaHpiSensorNumT) 0x006
/* Management processor error sensor */
#define OA_SOAP_SEN_MP_ERR			(SaHpiSensorNumT) 0x007
/* Power supply subsystem power input sensor */
#define OA_SOAP_SEN_IN_PWR			(SaHpiSensorNumT) 0x008
/* Power supply subsystem power output sensor */
#define OA_SOAP_SEN_OUT_PWR			(SaHpiSensorNumT) 0x009
/* Power supply subsystem power capacity sensor */
#define OA_SOAP_SEN_PWR_CAPACITY		(SaHpiSensorNumT) 0x00a
/* Thermal warning sensor */
#define OA_SOAP_SEN_THERM_WARN			(SaHpiSensorNumT) 0x00b
/* Thermal danger sensor */
#define OA_SOAP_SEN_THERM_DANGER		(SaHpiSensorNumT) 0x00c
/* IO configuration error sensor */
#define OA_SOAP_SEN_IO_CONFIG_ERR		(SaHpiSensorNumT) 0x00d
/* Device power request error sensor */
#define OA_SOAP_SEN_DEV_PWR_REQ			(SaHpiSensorNumT) 0x00e
/* Insufficient cooling error sensor */
#define OA_SOAP_SEN_INSUF_COOL			(SaHpiSensorNumT) 0x00f
/* Device location error sensor */
#define OA_SOAP_SEN_DEV_LOC_ERR			(SaHpiSensorNumT) 0x010
/* Device failure sensor */
#define OA_SOAP_SEN_DEV_FAIL			(SaHpiSensorNumT) 0x011
/* Device degraded sensor */
#define OA_SOAP_SEN_DEV_DEGRAD			(SaHpiSensorNumT) 0x012
/* AC failure sensor */
#define OA_SOAP_SEN_AC_FAIL			(SaHpiSensorNumT) 0x013
/* i2c buses sensor */
#define OA_SOAP_SEN_I2C_BUS			(SaHpiSensorNumT) 0x014
/* Redundancy error sensor */
#define OA_SOAP_SEN_REDUND_ERR			(SaHpiSensorNumT) 0x015
/* Enclosure aggregate operational status sensor */
#define OA_SOAP_SEN_ENC_AGR_OPER		(SaHpiSensorNumT) 0x016
/* Enclosure aggregate predictive failure sensor */
#define OA_SOAP_SEN_ENC_AGR_PRED_FAIL		(SaHpiSensorNumT) 0x017
/* Enclosure OA redundancy sensor */
#define OA_SOAP_SEN_OA_REDUND			(SaHpiSensorNumT) 0x018
/* Enclosure OA link status sensor */
#define OA_SOAP_SEN_OA_LINK_STATUS		(SaHpiSensorNumT) 0x019
/* Interconnect CPU fault sensor */
#define OA_SOAP_SEN_CPU_FAULT			(SaHpiSensorNumT) 0x01a
/* Interconnect health LED sensor */
#define OA_SOAP_SEN_HEALTH_LED			(SaHpiSensorNumT) 0x01b
/* Health status operational sensor */
#define OA_SOAP_SEN_HEALTH_OPER			(SaHpiSensorNumT) 0x01c
/* Health status predictive failure sensor */
#define OA_SOAP_SEN_HEALTH_PRED_FAIL		(SaHpiSensorNumT) 0x01d
/* Device missing sensor */
#define OA_SOAP_SEN_DEV_MISS			(SaHpiSensorNumT) 0x01e
/* Device power sequence sensor */
#define OA_SOAP_SEN_DEV_PWR_SEQ			(SaHpiSensorNumT) 0x01f
/* Device bonding sensor */
#define OA_SOAP_SEN_DEV_BOND			(SaHpiSensorNumT) 0x020
/* Network configuration sensor */
#define OA_SOAP_SEN_NET_CONFIG			(SaHpiSensorNumT) 0x021
/* Firmware mismatch */
#define OA_SOAP_SEN_FW_MISMATCH			(SaHpiSensorNumT) 0x022
/* Profile unassigned error sensor */
#define OA_SOAP_SEN_PROF_UNASSIGN_ERR		(SaHpiSensorNumT) 0x023
/* Device not supported sensor */
#define OA_SOAP_SEN_DEV_NOT_SUPPORT		(SaHpiSensorNumT) 0x024
/* Too low power request sensor */
#define OA_SOAP_SEN_TOO_LOW_PWR_REQ		(SaHpiSensorNumT) 0x025
/* Call HP sensor */
#define OA_SOAP_SEN_CALL_HP			(SaHpiSensorNumT) 0x026
/* Device informational sensor */
#define OA_SOAP_SEN_DEV_INFO			(SaHpiSensorNumT) 0x027
/* Storage device missing sensor */
#define OA_SOAP_SEN_STORAGE_DEV_MISS		(SaHpiSensorNumT) 0x028
/* Enclosure ID mismatch sensor */
#define OA_SOAP_SEN_ENC_ID_MISMATCH		(SaHpiSensorNumT) 0x029
/* Device mix match sensor */
#define OA_SOAP_SEN_DEV_MIX_MATCH		(SaHpiSensorNumT) 0x02a
/* Power capping error sensor */
#define OA_SOAP_SEN_GRPCAP_ERR			(SaHpiSensorNumT) 0x02b
/* IML recorded errors sensor */
#define OA_SOAP_SEN_IML_ERR			(SaHpiSensorNumT) 0x02c
/* Duplicate management IP address sensor */
#define OA_SOAP_SEN_DUP_MGMT_IP_ADDR		(SaHpiSensorNumT) 0x02d
/* Server Blade System zone1 */
#define OA_SOAP_SEN_BLADE_SYSTEM_ZONE1		(SaHpiSensorNumT) 0x02e
/* Server Blade System zone2 */
#define OA_SOAP_SEN_BLADE_SYSTEM_ZONE2		(SaHpiSensorNumT) 0x02f
/* Server Blade System zone3 */
#define OA_SOAP_SEN_BLADE_SYSTEM_ZONE3		(SaHpiSensorNumT) 0x030
/* Server Blade System zone4 */
#define OA_SOAP_SEN_BLADE_SYSTEM_ZONE4		(SaHpiSensorNumT) 0x031
/* Server Blade CPU zone1 */
#define OA_SOAP_SEN_BLADE_CPU_ZONE1		(SaHpiSensorNumT) 0x032
/* Server Blade CPU zone2 */
#define OA_SOAP_SEN_BLADE_CPU_ZONE2		(SaHpiSensorNumT) 0x033
/* Server Blade CPU zone3 */
#define OA_SOAP_SEN_BLADE_CPU_ZONE3		(SaHpiSensorNumT) 0x034
/* Server Blade CPU zone4 */
#define OA_SOAP_SEN_BLADE_CPU_ZONE4		(SaHpiSensorNumT) 0x035
/* Server Blade Memory zone1 */
#define OA_SOAP_SEN_BLADE_MEM_ZONE1		(SaHpiSensorNumT) 0x036
/* Server Blade Memory zone2 */
#define OA_SOAP_SEN_BLADE_MEM_ZONE2		(SaHpiSensorNumT) 0x037
/* Server Blade Memory zone3 */
#define OA_SOAP_SEN_BLADE_MEM_ZONE3		(SaHpiSensorNumT) 0x038
/* Server Blade Memory zone4 */
#define OA_SOAP_SEN_BLADE_MEM_ZONE4		(SaHpiSensorNumT) 0x039
/* Storage Blade Disk zone1 */
#define OA_SOAP_SEN_BLADE_DISK_ZONE1		(SaHpiSensorNumT) 0x03a
/* Storage Blade Disk zone2 */
#define OA_SOAP_SEN_BLADE_DISK_ZONE2		(SaHpiSensorNumT) 0x03b
/* Storage Blade Disk zone3 */
#define OA_SOAP_SEN_BLADE_DISK_ZONE3		(SaHpiSensorNumT) 0x03c
/* Storage Blade Disk zone4 */
#define OA_SOAP_SEN_BLADE_DISK_ZONE4		(SaHpiSensorNumT) 0x03d
/* Server Blade CPU1 */
#define OA_SOAP_SEN_BLADE_CPU1_1		(SaHpiSensorNumT) 0x03e
/* Server Blade CPU1 */
#define OA_SOAP_SEN_BLADE_CPU1_2		(SaHpiSensorNumT) 0x03f
/* Server Blade CPU1 */
#define OA_SOAP_SEN_BLADE_CPU1_3		(SaHpiSensorNumT) 0x040
/* Server Blade CPU1 */
#define OA_SOAP_SEN_BLADE_CPU1_4		(SaHpiSensorNumT) 0x041
/* Server Blade CPU2 */
#define OA_SOAP_SEN_BLADE_CPU2_1		(SaHpiSensorNumT) 0x042
/* Server Blade CPU2 */
#define OA_SOAP_SEN_BLADE_CPU2_2		(SaHpiSensorNumT) 0x043
/* Server Blade CPU2 */
#define OA_SOAP_SEN_BLADE_CPU2_3		(SaHpiSensorNumT) 0x044
/* Server Blade CPU2 */
#define OA_SOAP_SEN_BLADE_CPU2_4		(SaHpiSensorNumT) 0x045
/* Server Blade CPU3 */
#define OA_SOAP_SEN_BLADE_CPU3_1		(SaHpiSensorNumT) 0x046
/* Server Blade CPU3 */
#define OA_SOAP_SEN_BLADE_CPU3_2		(SaHpiSensorNumT) 0x047
/* Server Blade CPU3 */
#define OA_SOAP_SEN_BLADE_CPU3_3		(SaHpiSensorNumT) 0x048
/* Server Blade CPU3 */
#define OA_SOAP_SEN_BLADE_CPU3_4		(SaHpiSensorNumT) 0x049
/* Server Blade CPU4 */
#define OA_SOAP_SEN_BLADE_CPU4_1		(SaHpiSensorNumT) 0x04a
/* Server Blade CPU4 */
#define OA_SOAP_SEN_BLADE_CPU4_2		(SaHpiSensorNumT) 0x04b
/* Server Blade CPU4 */
#define OA_SOAP_SEN_BLADE_CPU4_3		(SaHpiSensorNumT) 0x04c
/* Server Blade CPU4 */
#define OA_SOAP_SEN_BLADE_CPU4_4		(SaHpiSensorNumT) 0x04d

/* Control numbers used in OA SOAP plugin 
 * On adding new control, control array in in plugin/oa_soap/oa_soap_resources.c
 * may require updation.
 */
/* UID control */
#define OA_SOAP_UID_CNTRL			(SaHpiCtrlNumT)   0x000
/* Power control */
#define OA_SOAP_PWR_CNTRL			(SaHpiCtrlNumT)   0x001
/* LCD Button Lock control */
#define OA_SOAP_LCD_BUTN_LCK_CNTRL		(SaHpiCtrlNumT)   0x002

/* Custom inventory Area and fields used in OA SOAP plugin 
 * On adding new inventory area or field, fan zone mapping rray in in
 * plugin/oa_soap/oa_soap_resources.c may require updation.
 */
/* Fan Zone field type for storing the device bays */
#define OA_SOAP_INV_FZ_DEV_BAY			(SaHpiIdrIdT)	  0x100
/* Fan Zone field type for storing the fan bays */
#define OA_SOAP_INV_FZ_FAN_BAY			(SaHpiIdrIdT)	  0x101
/* Fan field type for storing the shared status */
#define OA_SOAP_INV_FAN_SHARED			(SaHpiIdrIdT)	  0x102
/* Fan field type for storing the Fan zone number */
#define OA_SOAP_INV_FZ_NUM			(SaHpiIdrIdT)	  0x103

#endif
