/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Sean Dague <http://dague.net/sean>
 *      Renier Morales <renier@openhpi.org>
 *      Steve Sherman <stevees@us.ibm.com>
 */

/*************************************************************************
 * This header file stubs resource and RDR static infomation that are used
 * in constructing RPTs and RDRs for IBM  BladeCenter chassis and 
 * RSA (Integrated and separate adapter models).
 * 
 * Differences between the models are discovered dynamically by this
 * plugin at run-time during resource discovery.
 *************************************************************************/

#ifndef __SNMP_BC_RESOURCES_H
#define __SNMP_BC_RESOURCES_H

/* Start HPI location numbers from 1 */
#define SNMP_BC_HPI_LOCATION_BASE 1

/* An invalid snmp_bc index */
#define SNMP_BC_NOT_VALID 0xFF

/* IBM Manufacturing Number - Use IANA number for "Modular Blade Server" */
#define IBM_MANUFACTURING_ID 20944

/* Maximum OID string length */
#define SNMP_BC_MAX_OID_LENGTH 50

/* OIDs to determine platform types */
#define SNMP_BC_CHASSIS_TYPE_OID     ".1.3.6.1.4.1.2.3.51.2.22.4.38.0"  /* chassisType    */
#define SNMP_BC_CHASSIS_SUBTYPE_OID  ".1.3.6.1.4.1.2.3.51.2.22.4.39.0"  /* chassisSubtype */
#define SNMP_BC_CHASSIS_TYPE_BC      97
#define SNMP_BC_CHASSIS_TYPE_BCT     98
#define SNMP_BC_CHASSIS_SUBTYPE_ORIG 0   
#define SNMP_BC_CHASSIS_SUBTYPE_H    2
/* Original models don't have chassis type/subtype OIDs - just health OIDs */
#define SNMP_BC_PLATFORM_OID_BC      ".1.3.6.1.4.1.2.3.51.2.2.7.1.0" /* systemHealthStat, BC System Health */
#define SNMP_BC_PLATFORM_OID_BCT     ".1.3.6.1.4.1.2.3.51.2.2.9.1.0" /* telcoSystemHealthStat, BCT System Health */
#define SNMP_BC_PLATFORM_OID_RSA     ".1.3.6.1.4.1.2.3.51.1.2.7.1.0" /* RSA System Health */

/* Run-time variables to distinguish platform types */
#define SNMP_BC_PLATFORM_BCT   0x0001
#define SNMP_BC_PLATFORM_BC    0x0002
#define SNMP_BC_PLATFORM_RSA   0x0004
#define SNMP_BC_PLATFORM_BCH   0x0008
#define SNMP_BC_PLATFORM_BCHT  0x0009
#define SNMP_BC_PLATFORM_ALL   0xFFFF

/* Resource indexes to snmp_rpt array in discovery */
typedef enum {
        BC_RPT_ENTRY_CHASSIS = 0,
	BC_RPT_ENTRY_VIRTUAL_MGMNT_MODULE,
        BC_RPT_ENTRY_MGMNT_MODULE,
        BC_RPT_ENTRY_SWITCH_MODULE,
        BC_RPT_ENTRY_BLADE,
        BC_RPT_ENTRY_BLADE_EXPANSION_CARD,
        BC_RPT_ENTRY_MEDIA_TRAY,
	BC_RPT_ENTRY_MEDIA_TRAY_2,
        BC_RPT_ENTRY_BLOWER_MODULE,
        BC_RPT_ENTRY_POWER_MODULE,
	BC_RPT_ENTRY_PHYSICAL_SLOT,
	BC_RPT_ENTRY_BEM_DASD,
	BC_RPT_ENTRY_ALARM_PANEL,
	BC_RPT_ENTRY_MUX_MODULE,
	BC_RPT_ENTRY_CLOCK_MODULE,
	BC_RPT_ENTRY_AIR_FILTER,
	BC_RPT_ENTRY_INTERPOSER_SWITCH,
	BC_RPT_ENTRY_INTERPOSER_MM
} BCRptEntryT;


/* Matching mmblade.mib definitions */		
/*	storageExpansion(1),        */
/* 	pciExpansion(2)             */
typedef enum {
        DEFAULT_BLADE_EXPANSION_CARD_TYPE = 0,
	BLADE_STORAGE_EXPANSION,
	BLADE_PCI_EXPANSION
} BCExpansionTypeT;

typedef enum {
	RSA_RPT_ENTRY_CHASSIS = 0,
	RSA_RPT_ENTRY_CPU,
	RSA_RPT_ENTRY_DASD,
	RSA_RPT_ENTRY_FAN
} RSARptEntryT;

/* Maximum number of RSA resources */
#define RSA_MAX_CPU    8
#define RSA_MAX_FAN    8
#define RSA_MAX_DASD   4

/* Maximum entries in eventlog, approximate */
#define BC_EL_MAX_SIZE 768 /* 512 */

/* OIDs definitions for Blade Center Chassis Topology */
#define SNMP_BC_NOS_FP_SUPPORTED  ".1.3.6.1.4.1.2.3.51.2.22.4.18.0" /* chassisNoOfFPsSupported, FanPack */
#define SNMP_BC_NOS_PB_SUPPORTED  ".1.3.6.1.4.1.2.3.51.2.22.4.19.0" /* chassisNoOfPBsSupported, ProcessorBlade */
#define SNMP_BC_NOS_SM_SUPPORTED  ".1.3.6.1.4.1.2.3.51.2.22.4.20.0" /* chassisNoOfSMsSupported, SwitchModule */
#define SNMP_BC_NOS_MM_SUPPORTED  ".1.3.6.1.4.1.2.3.51.2.22.4.21.0" /* chassisNoOfMMsSupported, ManagementModule */
#define SNMP_BC_NOS_PM_SUPPORTED  ".1.3.6.1.4.1.2.3.51.2.22.4.22.0" /* chassisNoOfPMsSupported, PowerModule */
#define SNMP_BC_NOS_MT_SUPPORTED  ".1.3.6.1.4.1.2.3.51.2.22.4.23.0" /* chassisNoOfMTsSupported, MediaTray */
#define SNMP_BC_NOS_BLOWER_SUPPORTED ".1.3.6.1.4.1.2.3.51.2.22.4.24.0" 	/* chassisNoOfBlowersSupported, Blower */
#define SNMP_BC_NOS_FILTER_SUPPORTED ".1.3.6.1.4.1.2.3.51.2.22.4.40.0"  /* chassisNoOfFBsSupported, Front Bezel */
#define SNMP_BC_NOS_AP_SUPPORTED  ".1.3.6.1.4.1.2.3.51.2.22.4.41.0" /* chassisNoOfAPsSupported, AlarmPanel */
#define SNMP_BC_NOS_NC_SUPPORTED  ".1.3.6.1.4.1.2.3.51.2.22.4.42.0" /* chassisNoOfNCsSupported, NetworkClock Card */
#define SNMP_BC_NOS_MX_SUPPORTED  ".1.3.6.1.4.1.2.3.51.2.22.4.43.0" /* chassisNoOfMXsSupported, Multiplexer Expansion Mod */
#define SNMP_BC_NOS_MMI_SUPPORTED ".1.3.6.1.4.1.2.3.51.2.22.4.44.0" /* chassisNoOfMMIsSupported, MM Interposer */
#define SNMP_BC_NOS_SMI_SUPPORTED ".1.3.6.1.4.1.2.3.51.2.22.4.45.0" /* chassisNoOfSMIsSupported, Switch Interposer */

#define SNMP_BC_PB_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.25.0" 	/* chassisPBsInstalled, ProcessorBlade */
#define SNMP_BC_SM_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.29.0" 	/* chassisSMsInstalled, SwitchModule */
#define SNMP_BC_MM_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.30.0" 	/* chassisMMsInstalled, ManagementModule */
#define SNMP_BC_PM_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.31.0" 	/* chassisPMsInstalled, PowerModule */
#define SNMP_BC_MT_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.32.0" 	/* chassisMTInstalled, MediaTray */
#define SNMP_BC_NOS_MT_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.52.0" 	/* chassisNoOfMTsInstalled, MediaTray */
#define SNMP_BC_BLOWER_INSTALLED ".1.3.6.1.4.1.2.3.51.2.22.4.33.0" 	/* chassisBlowersInstalled, Blower */
#define SNMP_BC_FP_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.37.0" 	/* chassisFPsinstalled, FanPack */
#define SNMP_BC_FILTER_INSTALLED ".1.3.6.1.4.1.2.3.51.2.22.4.46.0"      /* chassisNoOfFBsInstalled, FrontBezel (Filter) */
#define SNMP_BC_AP_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.47.0" /* chassisNoOfAPsInstalled, AlarmPanel */
#define SNMP_BC_NC_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.48.0" /* chassisNoOfNCsInstalled, NetworkClock Card */
#define SNMP_BC_MX_INSTALLED  ".1.3.6.1.4.1.2.3.51.2.22.4.49.0" /* chassisNoOfMXsInstalled, Multiplexer Expansion Mod */
#define SNMP_BC_MMI_INSTALLED ".1.3.6.1.4.1.2.3.51.2.22.4.50.0" /* chassisNoOfMMIsInstalled. MM Interposer*/
#define SNMP_BC_SMI_INSTALLED ".1.3.6.1.4.1.2.3.51.2.22.4.51.0" /* chassisNoOfSMIsInstalled, Switch Interposer */

#define SNMP_BC_NC_VPD_BAY_NUMBER ".1.3.6.1.4.1.2.3.51.2.2.21.16.1.1.2.x" /* ncHardwareVpdBayNumber, */
#define SNMP_BC_MX_VPD_BAY_NUMBER ".1.3.6.1.4.1.2.3.51.2.2.21.17.1.1.2.x" /* mxHardwareVpdBayNumber,  */
#define SNMP_BC_SMI_VPD_BAY_NUMBER ".1.3.6.1.4.1.2.3.51.2.2.21.6.2.1.2.x" /* smInpHardwareVpdBayNumber, Switch Interposer VpsBayNumber */ 

#define SNMP_BC_BLADE_EXPANSION_VECTOR  ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.x" /* bladeServerExpansion */
#define SNMP_BC_BLADE_EXP_BLADE_BAY	".1.3.6.1.4.1.2.3.51.2.2.21.4.3.1.19.x" /* bladeExpBoardVpdBladeBayNumber */
#define SNMP_BC_BLADE_EXP_TYPE		".1.3.6.1.4.1.2.3.51.2.2.21.4.3.1.20.x" /* bladeExpBoardVpdCardType */
#define SNMP_BC_MGMNT_ACTIVE            ".1.3.6.1.4.1.2.3.51.2.22.4.34.0" 	/* chassisActiveMM */
#define SNMP_BC_DST                     ".1.3.6.1.4.1.2.3.51.2.4.4.2.0"         /* spClockTimezoneSetting */

#define SNMP_BC_DST_RSA                 ".1.3.6.1.4.1.2.3.51.1.4.4.2.0"
#define SNMP_BC_CPU_OID_RSA             ".1.3.6.1.4.1.2.3.51.1.2.20.1.5.1.1.3.x"
#define SNMP_BC_DASD_OID_RSA            ".1.3.6.1.4.1.2.3.51.1.2.20.1.6.1.1.3.x"
#define SNMP_BC_FAN_OID_RSA             ".1.3.6.1.4.1.2.3.51.1.2.3.x.0"

/* OID definitions for System Event Log */
#define SNMP_BC_DATETIME_OID            ".1.3.6.1.4.1.2.3.51.2.4.4.1.0"    /* spClockDateAndTimeSetting */
#define SNMP_BC_DATETIME_OID_RSA        ".1.3.6.1.4.1.2.3.51.1.4.4.1.0"    
#define SNMP_BC_SEL_INDEX_OID           ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1"  /* readEventLogIndex */
#define SNMP_BC_SEL_INDEX_OID_RSA       ".1.3.6.1.4.1.2.3.51.1.3.4.2.1.1"
#define SNMP_BC_SEL_ENTRY_OID           ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2"  /* readEventLogString */
#define SNMP_BC_SEL_ENTRY_OID_RSA       ".1.3.6.1.4.1.2.3.51.1.3.4.2.1.2"
#define SNMP_BC_SEL_CLEAR_OID           ".1.3.6.1.4.1.2.3.51.2.3.4.3.0"    /* clearEventLog */
#define SNMP_BC_SEL_CLEAR_OID_RSA       ".1.3.6.1.4.1.2.3.51.1.3.4.3.0"

/* mmHeathState OID */
#define SNMP_BC_MM_HEALTH_OID           ".1.3.6.1.4.1.2.3.51.2.22.5.1.1.5.1" /* mmHealthState */

/* Slot ResourceTag */
#define SNMP_BC_PHYSICAL_SLOT  	      "Blade Slot"
#define SNMP_BC_SWITCH_SLOT           "I/O Module Slot"
#define SNMP_BC_POWER_SUPPLY_SLOT     "Power Module Slot"
#define SNMP_BC_PERIPHERAL_BAY_SLOT   "Media Tray Slot"
#define SNMP_BC_SYS_MGMNT_MODULE_SLOT "Management Module Slot"
#define SNMP_BC_BLOWER_SLOT 	      "Blower Slot"
#define SNMP_BC_ALARM_PANEL_SLOT      "Alarm Panel Slot"
#define SNMP_BC_MUX_SLOT              "Multiplexer Expansion Module Slot"
#define SNMP_BC_CLOCK_SLOT            "Network Clock Module Slot"

/* Slot Power OIDs  */
#define SNMP_BC_PD1POWERCURRENT ".1.3.6.1.4.1.2.3.51.2.2.10.2.1.1.7" /* pd1ModuleAllocatedPowerCurrent */
#define SNMP_BC_PD1POWERMAX	".1.3.6.1.4.1.2.3.51.2.2.10.2.1.1.8" /* pd1ModuleAllocatedPowerMax */ 
#define SNMP_BC_PD1POWERMIN	".1.3.6.1.4.1.2.3.51.2.2.10.2.1.1.9" /* pd1ModuleAllocatedPowerMin */
#define SNMP_BC_PD2POWERCURRENT	".1.3.6.1.4.1.2.3.51.2.2.10.3.1.1.7" /* pd2ModuleAllocatedPowerCurrent */
#define SNMP_BC_PD2POWERMAX	".1.3.6.1.4.1.2.3.51.2.2.10.3.1.1.8" /* pd2ModuleAllocatedPowerMax */
#define SNMP_BC_PD2POWERMIN	".1.3.6.1.4.1.2.3.51.2.2.10.3.1.1.9" /* pd2ModuleAllocatedPowerMin */
#define SNMP_BC_PD1STATE	".1.3.6.1.4.1.2.3.51.2.2.10.2.1.1.6" /* pd1ModuleState */
#define SNMP_BC_PD2STATE	".1.3.6.1.4.1.2.3.51.2.2.10.3.1.1.6" /* pd2ModuleState: standby(0), on(1), notPresent(2),notApplicable(255)*/
#define SNMP_BC_PMSTATE		".1.3.6.1.4.1.2.3.51.2.2.4.1.1.3"    /* powerModuleState: unknown(0), good(1), warning(2), not available(3) */

/* Sensor and Control Numbers defined for Redundancy MM Implementation */
#define BLADECENTER_SENSOR_NUM_MGMNT_REDUNDANCY  	(SaHpiSensorNumT) 0x1001
#define BLADECENTER_SENSOR_NUM_MGMNT_ACTIVE  		(SaHpiSensorNumT) 0x1002
#define BLADECENTER_SENSOR_NUM_MGMNT_STANDBY  		(SaHpiSensorNumT) 0x1003
#define BLADECENTER_CTRL_NUM_MGMNT_FAILOVER  		(SaHpiCtrlNumT)   0x1010
#define BLADECENTER_CTRL_NUM_FAILED_RESOURCE_EXTRACT  	(SaHpiCtrlNumT)   0x101E

/**********************
 * Resource Definitions
 **********************/

struct ResourceMibInfo {
        const char *OidReset;
        const char *OidPowerState;
        const char *OidPowerOnOff;
	const char *OidUuid;
	const char *OidResourceWidth; /* OID specifying how many physical slots a blade occupies */
};

/* SNMP_BC_MAX_RESOURCE_EVENT_ARRAY_SIZE includes an ending NULL entry */
#define SNMP_BC_MAX_EVENTS_PER_RESOURCE 10
#define SNMP_BC_MAX_RESOURCE_EVENT_ARRAY_SIZE (SNMP_BC_MAX_EVENTS_PER_RESOURCE + 1)

/* For BladeCenter resources, some managed hot swap state events
  (e.g. INSERTION_PENDING and EXTRACTION_PENDING) are automatically generated.
  The "auto" fields below determine the auto-generated events. It's assummed 
  that the INACTIVE state (0) never needs to be auto generated. */
struct res_event_map {
        char *event;
	SaHpiBoolT event_res_failure;
	SaHpiBoolT event_res_failure_unexpected;
        SaHpiHsStateT event_state;
	SaHpiHsStateT event_auto_state;
        SaHpiHsStateT recovery_state;
	SaHpiHsStateT recovery_auto_state;	
};

struct ResourceInfo {
        struct ResourceMibInfo mib;
	unsigned int resourcewidth;
        SaHpiHsStateT cur_state;
	SaHpiHsStateT prev_state; /* Needed to handle events that re-announce current hot swap state */
        struct res_event_map event_array[SNMP_BC_MAX_RESOURCE_EVENT_ARRAY_SIZE];
};

struct snmp_rpt {
        SaHpiRptEntryT rpt;
        struct ResourceInfo res_info;
        const  char *comment;
	const  char *OidResourceTag;
};

extern struct snmp_rpt snmp_bc_rpt_array[];
extern struct snmp_rpt snmp_bc_rpt_array_rsa[];

/********************
 * Sensor Definitions
 ********************/

struct SnmpSensorThresholdOids {
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

struct SnmpSensorWritableThresholdOids {
        const char *LowMinor;
        const char *LowMajor;
        const char *LowCritical;
        const char *UpMinor;
        const char *UpMajor;
        const char *UpCritical;
        const char *PosThdHysteresis;
        const char *NegThdHysteresis;
};

struct SensorMibInfo {
        unsigned int not_avail_indicator_num; /* 0 for none, n>0 otherwise */
        SaHpiBoolT write_only; /* TRUE - Write-only SNMP command */
        const char *oid;
        SaHpiEntityLocationT loc_offset;
        struct SnmpSensorThresholdOids threshold_oids;
	struct SnmpSensorWritableThresholdOids threshold_write_oids;
};

/*  Size definitions include an ending NULL entry */
#define SNMP_BC_MAX_EVENTS_PER_SENSOR 128
#define SNMP_BC_MAX_READING_MAPS_PER_SENSOR 6
#define SNMP_BC_MAX_SENSOR_EVENT_ARRAY_SIZE  (SNMP_BC_MAX_EVENTS_PER_SENSOR + 1)
#define SNMP_BC_MAX_SENSOR_READING_MAP_ARRAY_SIZE (SNMP_BC_MAX_READING_MAPS_PER_SENSOR + 1)

/* If you add to this structure, you may also have to change EventMapInfoT 
   and event discovery in snmp_bc_event.c */
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
        struct SensorMibInfo mib;
        SaHpiEventStateT cur_state; /* This really records the last state read from the SEL */
	                            /* Which may not be the current state of the sensor */
	SaHpiResourceIdT cur_child_rid;				    
	SaHpiBoolT sensor_enabled;
	SaHpiBoolT events_enabled;
        SaHpiEventStateT assert_mask;
	SaHpiEventStateT deassert_mask;
        struct sensor_event_map event_array[SNMP_BC_MAX_SENSOR_EVENT_ARRAY_SIZE];
	struct sensor_reading_map reading2event[SNMP_BC_MAX_SENSOR_READING_MAP_ARRAY_SIZE];
};

/* Usually sensor.Num = index in snmp_bc_resources.c. But to support HPI-defined
   sensor numbers (e.g. aggregate sensors), they can be different. sensor.Num 
   supports the HPI sensor number while index is used to search through the 
   plugin's sensor definition arrays */
struct snmp_bc_sensor {
	int index;
        SaHpiSensorRecT sensor;
        struct SensorInfo sensor_info;
        const char *comment;
};

struct snmp_bc_ipmi_sensor {
	const char *ipmi_tag;
	const char *ipmi_tag_alias1;
	struct snmp_bc_sensor ipmi;
};

extern struct snmp_bc_sensor      snmp_bc_chassis_sensors[];
extern struct snmp_bc_sensor      snmp_bc_chassis_sensors_bct_filter[];
extern struct snmp_bc_sensor      snmp_bc_blade_sensors[];
extern struct snmp_bc_ipmi_sensor snmp_bc_blade_ipmi_sensors[];
extern struct snmp_bc_sensor      snmp_bc_bem_sensors[];
extern struct snmp_bc_ipmi_sensor snmp_bc_bem_ipmi_sensors[];
extern struct snmp_bc_sensor      snmp_bc_bse_dasd_sensors[];
extern struct snmp_bc_sensor      snmp_bc_bse3_dasd_sensors[];
extern struct snmp_bc_sensor      snmp_bc_mgmnt_sensors[];
extern struct snmp_bc_sensor      snmp_bc_mgmnt_health_sensors[];
extern struct snmp_bc_sensor      snmp_bc_virtual_mgmnt_sensors[];
extern struct snmp_bc_sensor      snmp_bc_mediatray_sensors[];
extern struct snmp_bc_sensor      snmp_bc_mediatray_sensors_faultled[];
extern struct snmp_bc_sensor      snmp_bc_mediatray_sensors_nofaultled[];
extern struct snmp_bc_sensor      snmp_bc_mediatray2_sensors[];
extern struct snmp_bc_sensor      snmp_bc_blower_sensors[];
extern struct snmp_bc_sensor      snmp_bc_blower_sensors_bch[];
extern struct snmp_bc_sensor      snmp_bc_power_sensors[];
extern struct snmp_bc_sensor      snmp_bc_power_sensors_bch[];
extern struct snmp_bc_sensor      snmp_bc_switch_sensors[];
extern struct snmp_bc_sensor      snmp_bc_slot_sensors[];
extern struct snmp_bc_sensor      snmp_bc_alarm_sensors[];
extern struct snmp_bc_sensor      snmp_bc_mux_sensors[];
extern struct snmp_bc_sensor      snmp_bc_clock_sensors[];
extern struct snmp_bc_sensor      snmp_bc_filter_sensors[];

extern struct snmp_bc_sensor      snmp_bc_chassis_sensors_rsa[];
extern struct snmp_bc_sensor      snmp_bc_cpu_sensors_rsa[];
extern struct snmp_bc_sensor      snmp_bc_dasd_sensors_rsa[];
extern struct snmp_bc_sensor      snmp_bc_fan_sensors_rsa[];

/*********************
 * Control Definitions
 *********************/

struct ControlMibInfo {
        unsigned int not_avail_indicator_num; /* 0 for none, n>0 otherwise */
        int write_only; /* Write-only SNMP command; 0 no; 1 yes */
        const char *oid;
	SaHpiEntityLocationT loc_offset;
        int digitalmap[OH_MAX_CTRLSTATEDIGITAL];  /* Readable digital controls */
	int digitalwmap[OH_MAX_CTRLSTATEDIGITAL]; /* Writable digital controls */
	SaHpiBoolT isDigitalReadStateConstant;
	SaHpiCtrlStateDigitalT DigitalStateConstantValue;
};

struct ControlInfo {
        struct ControlMibInfo mib;
	SaHpiCtrlModeT cur_mode;
	SaHpiCtrlStateUnionT valid_states_get;  /* Only meaningful for Digital Controls */
	SaHpiCtrlStateUnionT allowed_states_set;  /* Only meaningful for Digital Controls */
};

/* Usually control.Num = index in snmp_bc_resources.c. But to support ATCA/HPI defined
   control numbers, they can be different. control.Num 
   supports the HPI control number while index is used to search through the 
   plugin's control definition arrays */
struct snmp_bc_control {
	int index;
        SaHpiCtrlRecT control;
        struct ControlInfo control_info;
        const char *comment;
};

extern struct snmp_bc_control snmp_bc_chassis_controls_bc[];
extern struct snmp_bc_control snmp_bc_chassis_controls_bct[];
extern struct snmp_bc_control snmp_bc_blade_controls[];
extern struct snmp_bc_control snmp_bc_bem_controls[];
extern struct snmp_bc_control snmp_bc_mgmnt_controls[];
extern struct snmp_bc_control snmp_bc_virtual_mgmnt_controls[];
extern struct snmp_bc_control snmp_bc_mediatray_controls[];
extern struct snmp_bc_control snmp_bc_mediatray2_controls[];
extern struct snmp_bc_control snmp_bc_blower_controls[];
extern struct snmp_bc_control snmp_bc_power_controls[];
extern struct snmp_bc_control snmp_bc_switch_controls[];
extern struct snmp_bc_control snmp_bc_slot_controls[];
extern struct snmp_bc_control snmp_bc_bem_dasd_controls[];
extern struct snmp_bc_control snmp_bc_alarm_controls[];
extern struct snmp_bc_control snmp_bc_mux_controls[];
extern struct snmp_bc_control snmp_bc_clock_controls[];
extern struct snmp_bc_control snmp_bc_filter_controls[];

extern struct snmp_bc_control snmp_bc_chassis_controls_rsa[];
extern struct snmp_bc_control snmp_bc_cpu_controls_rsa[];
extern struct snmp_bc_control snmp_bc_dasd_controls_rsa[];
extern struct snmp_bc_control snmp_bc_fan_controls_rsa[];

/***********************
 * Inventory Definitions
 ***********************/

struct SnmpInventoryOids {
        const char *OidChassisType;
        const char *OidMfgDateTime;
        const char *OidManufacturer;
        const char *OidProductName;
        const char *OidProductVersion;
        const char *OidSerialNumber;
        const char *OidPartNumber;
        const char *OidFileId;
        const char *OidAssetTag;
};

struct InventoryMibInfo {
        unsigned int not_avail_indicator_num; /* 0 for none, n>0 otherwise */
        int write_only; /* Write-only SNMP command; 0 no; 1 yes  */
        SaHpiIdrAreaTypeT  area_type;
        struct SnmpInventoryOids oid;
};

struct InventoryInfo {
        struct InventoryMibInfo hardware_mib;
	struct InventoryMibInfo firmware_mib;
};

struct snmp_bc_inventory {
        SaHpiInventoryRecT  inventory;
        struct InventoryInfo inventory_info;
        const char *comment;
};

extern struct snmp_bc_inventory snmp_bc_chassis_inventories[];
extern struct snmp_bc_inventory snmp_bc_blower_inventories[];
extern struct snmp_bc_inventory snmp_bc_mgmnt_inventories[];
extern struct snmp_bc_inventory snmp_bc_virtual_mgmnt_inventories[];
extern struct snmp_bc_inventory snmp_bc_switch_inventories[];
extern struct snmp_bc_inventory snmp_bc_blade_inventories[];
extern struct snmp_bc_inventory snmp_bc_bem_inventories[];
extern struct snmp_bc_inventory snmp_bc_mediatray_inventories[];
extern struct snmp_bc_inventory snmp_bc_mediatray2_inventories[];
extern struct snmp_bc_inventory snmp_bc_power_inventories[];
extern struct snmp_bc_inventory snmp_bc_slot_inventories[];
extern struct snmp_bc_inventory snmp_bc_bem_dasd_inventories[];
extern struct snmp_bc_inventory snmp_bc_alarm_inventories[];
extern struct snmp_bc_inventory snmp_bc_mux_inventories[];
extern struct snmp_bc_inventory snmp_bc_clock_inventories[];
extern struct snmp_bc_inventory snmp_bc_filter_inventories[];
extern struct snmp_bc_inventory snmp_bc_interposer_switch_inventories[];
extern struct snmp_bc_inventory snmp_bc_interposer_mm_inventories[];

extern struct snmp_bc_inventory snmp_bc_chassis_inventories_rsa[];
extern struct snmp_bc_inventory snmp_bc_cpu_inventories_rsa[];
extern struct snmp_bc_inventory snmp_bc_dasd_inventories_rsa[];
extern struct snmp_bc_inventory snmp_bc_fan_inventories_rsa[];

#endif

