/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      Renier Morales <renierm@users.sf.net>
 *      Sean Dague <http://dague.net/sean>
 *      Steve Sherman <stevees@us.ibm.com>
 */

/**************************************************************************
 * This header file stubs resource and RDR static infomation that are used
 * in constructing RPT and RDR for IBM's Blade Center Enterprise (BCE).
 *
 * The BCE has the following entity hierarchy:
 *
 *  {CHASSIS,X}
 *      |  
 *      +-- {SUBCHASSIS,1}  (Power Distribution Sub-System)
 *      |   |
 *      |   +-- {SYS_MGMNT_MODULE,[1-2]}  (Management Module)
 *      |   |
 *      |   +-- {INTERCONNECT,[1-4]}  (Switch Module)
 *      |   |
 *      |   +-- {SBC_BLADE,[1-6]}  (Blade)
 *      |   |    |
 *      |   |    +-- {ADD_IN_CARD,1}  (Blade daughter card e.g. BSE card)     
 *      |   |
 *      |   +-- {PERIPHERAL_BAY,1}  (Control Panel/Media Tray)
 *      |   |
 *      |   +-- {POWER_SUPPLY,[1-2]}  (Power Module)
 *      |   |
 *      |   +-- {FAN,[1-2]}  (Blower Module)
 *      |
 *      +-- {SUBCHASSIS,2}  (Power Distribution Sub-System)
 *          |
 *          +-- {SBC_BLADE,[7-14]}  (Blade)
 *          |   |
 *          |   +-- {ADD_IN_CARD,1}  (Blade daughter card e.g. BSE card)
 *          |
 *          +-- {POWER_SUPPLY,[3-4]}  (Power Module)
 *************************************************************************/

/*************************************************************************
 * RESTRICTIONS!!!
 *
 * - If IsThreshold=SAHPI_TRUE for an interpreted sensor, 
 *   Range.Max.Interpreted.Type must be defined for snmp_bc.c 
 *   get_interpreted_thresholds to work.
 * - Digital controls must be integers and depend on SaHpiStateDigitalT
 *************************************************************************/

#ifndef __BC_RESOURCES_H
#define __BC_RESOURCES_H

/* HPI Spec dependencies */
#define ELEMENTS_IN_SaHpiStateDigitalT 5

/* Resource indexes to snmp_rpt array below */
typedef enum {
	BC_RPT_ENTRY_CHASSIS = 0,
	BC_RPT_ENTRY_SUBCHASSIS,
	BC_RPT_ENTRY_MGMNT_MODULE,
	BC_RPT_ENTRY_SWITCH_MODULE,
	BC_RPT_ENTRY_BLADE,
	BC_RPT_ENTRY_BLADE_ADDIN_CARD,
	BC_RPT_ENTRY_MEDIA_TRAY,
	BC_RPT_ENTRY_BLOWER_MODULE,
        BC_RPT_ENTRY_POWER_MODULE
} BCRptEntryT;

/* Start HPI Instance numbers from 1 */
#define BC_HPI_INSTANCE_BASE 1
/* Character for blanking out normalized oid elements */
#define OID_BLANK_CHAR 'x'
#define OID_BLANK_STR "x"

/* Maximum number of sub-chassis */
#define BC_MAX_SUBCHASSIS 2

/* Maximum number of resources per sub-chassis */
#define BC_MAX_BLADES_ON_SC1  6
#define BC_MAX_FANS_ON_SC1    2
#define BC_MAX_MM_ON_SC1      2
#define BC_MAX_POWER_ON_SC1   2
#define BC_MAX_SWITCH_ON_SC1  4

/* OID definitions for discovering resources.*/
#define SNMP_BC_BLADE_VECTOR        ".1.3.6.1.4.1.2.3.51.2.2.5.2.49.0"
#define SNMP_BC_BLADE_ADDIN_VECTOR  ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.x" /* Uses temp DASD1 MIB reading */
#define SNMP_BC_FAN_VECTOR          ".1.3.6.1.4.1.2.3.51.2.2.5.2.73.0"
#define SNMP_BC_MGMNT_VECTOR        ".1.3.6.1.4.1.2.3.51.2.22.4.30.0"
#define SNMP_BC_MGMNT_ACTIVE        ".1.3.6.1.4.1.2.3.51.2.22.4.34.0"
#define SNMP_BC_MEDIATRAY_EXISTS    ".1.3.6.1.4.1.2.3.51.2.2.5.2.74.0" /* Installed vs communicating */
#define SNMP_BC_POWER_VECTOR        ".1.3.6.1.4.1.2.3.51.2.2.5.2.89.0"
#define SNMP_BC_SWITCH_VECTOR       ".1.3.6.1.4.1.2.3.51.2.2.5.2.113.0"
#define SNMP_BC_TIME_DST            ".1.3.6.1.4.1.2.3.51.2.4.4.2.0" /* "+0:00,no" */

#if 0
#define SNMP_BC_MAX_BLADES          ".1.3.6.1.4.1.2.3.51.2.22.4.19.0"
#define SNMP_BC_MAX_SWITCHES        ".1.3.6.1.4.1.2.3.51.2.22.4.20.0"
#define SNMP_BC_MAX_MM              ".1.3.6.1.4.1.2.3.51.2.22.4.21.0"
#define SNMP_BC_MAX_POWER           ".1.3.6.1.4.1.2.3.51.2.22.4.22.0"
#define SNMP_BC_MAX_MT              ".1.3.6.1.4.1.2.3.51.2.22.4.23.0"
#define SNMP_BC_MAX_FANS            ".1.3.6.1.4.1.2.3.51.2.22.4.24.0"
#endif

/*************************************************************************
 *                   Resource Definitions
 *************************************************************************/
struct ResourceMibInfo {
	const char* OidHealth;
	int   HealthyValue;
	const char* OidReset;
	const char* OidPowerState;
	const char* OidPowerOnOff;
};

struct snmp_rpt {
        SaHpiRptEntryT rpt;
	struct ResourceMibInfo mib;
        const  char* comment;
};

extern struct snmp_rpt snmp_rpt_array[];

/******************************************************************************
 *                      Sensor Definitions
 ******************************************************************************/
struct SNMPRawThresholdsOIDs {
	const char *OidLowMinor;
	const char *OidLowMajor;
	const char *OidLowCrit;
	const char *OidUpMinor;
	const char *OidUpMajor;
	const char *OidUpCrit;
	const char *OidLowHysteresis;
	const char *OidUpHysteresis;
};

struct SNMPInterpretedThresholdsOIDs {
	const char *OidLowMinor;
	const char *OidLowMajor;
	const char *OidLowCrit;
	const char *OidUpMinor;
	const char *OidUpMajor;
	const char *OidUpCrit;
	const char *OidLowHysteresis;
	const char *OidUpHysteresis;
};

struct SnmpSensorThresholdOids {
	struct SNMPRawThresholdsOIDs RawThresholds;
	struct SNMPInterpretedThresholdsOIDs InterpretedThresholds;
};

struct SensorMibInfo {
        unsigned int not_avail_indicator_num; /* 0 for none, n>0 otherwise */
	int write_only; /* Write-only SNMP command; 0 no; 1 yes  */
	int convert_snmpstr; /* -1 no conversion; else use SaHpiSensorInterpretedTypeT values */
	const char* oid;
        struct SnmpSensorThresholdOids threshold_oids;
};

struct snmp_bc_sensor {
        SaHpiSensorRecT sensor;
	struct SensorMibInfo mib;
        const char* comment;
};

extern struct snmp_bc_sensor snmp_bc_chassis_sensors[];
extern struct snmp_bc_sensor snmp_bc_subchassis_sensors[];
extern struct snmp_bc_sensor snmp_bc_blade_sensors[];
extern struct snmp_bc_sensor snmp_bc_blade_addin_sensors[];
extern struct snmp_bc_sensor snmp_bc_mgmnt_sensors[];
extern struct snmp_bc_sensor snmp_bc_mediatray_sensors[];
extern struct snmp_bc_sensor snmp_bc_fan_sensors[];
extern struct snmp_bc_sensor snmp_bc_power_sensors[];
extern struct snmp_bc_sensor snmp_bc_switch_sensors[];

/*************************************************************************
 *                   Control Definitions
 *************************************************************************/
struct ControlMibInfo {
        unsigned int not_avail_indicator_num; /* 0 for none, n>0 otherwise */
        int write_only; /* Write-only SNMP command; 0 no; 1 yes  */
	const char* oid;
	int digitalmap[ELEMENTS_IN_SaHpiStateDigitalT];
};

struct snmp_bc_control {
        SaHpiCtrlRecT control;
        struct ControlMibInfo mib;
        const char* comment;
};

extern struct snmp_bc_control snmp_bc_chassis_controls[];
extern struct snmp_bc_control snmp_bc_subchassis_controls[];
extern struct snmp_bc_control snmp_bc_blade_controls[];
extern struct snmp_bc_control snmp_bc_blade_addin_controls[];
extern struct snmp_bc_control snmp_bc_mgmnt_controls[];
extern struct snmp_bc_control snmp_bc_mediatray_controls[];
extern struct snmp_bc_control snmp_bc_fan_controls[];
extern struct snmp_bc_control snmp_bc_power_controls[];
extern struct snmp_bc_control snmp_bc_switch_controls[];
 
/*************************************************************************
 *                   Inventory Definitions
 *************************************************************************/
struct SnmpInventoryOids {
        const char *OidMfgDateTime;
        const char *OidManufacturer;
        const char *OidProductName;
        const char *OidProductVersion;
        const char *OidModelNumber;
        const char *OidSerialNumber;
        const char *OidPartNumber;
        const char *OidFileId;
        const char *OidAssetTag;
};

struct InventoryMibInfo {
        unsigned int not_avail_indicator_num; /* 0 for none, n>0 otherwise */
        int write_only; /* Write-only SNMP command; 0 no; 1 yes  */
        SaHpiInventDataRecordTypeT  inventory_type;
	SaHpiInventChassisTypeT chassis_type; /* Ignore if inventory_type not CHASSIS */
	struct SnmpInventoryOids oid;
};

struct snmp_bc_inventory {
	SaHpiInventoryRecT  inventory;
        struct InventoryMibInfo mib;
        const char* comment;
};

extern struct snmp_bc_inventory snmp_bc_chassis_inventories[];
extern struct snmp_bc_inventory snmp_bc_subchassis_inventories[];
extern struct snmp_bc_inventory snmp_bc_fan_inventories[];
extern struct snmp_bc_inventory snmp_bc_mgmnt_inventories[];
extern struct snmp_bc_inventory snmp_bc_switch_inventories[];
extern struct snmp_bc_inventory snmp_bc_blade_inventories[];
extern struct snmp_bc_inventory snmp_bc_blade_addin_inventories[];
extern struct snmp_bc_inventory snmp_bc_mediatray_inventories[];
extern struct snmp_bc_inventory snmp_bc_power_inventories[];

#endif  /* End __BC_RESOURCES.H */
