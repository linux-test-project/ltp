/*******************************************************************************
**
** FILE:
**   SaHpiAtca.h
**
** DESCRIPTION: 
**   This file provides the C language binding for the Service 
**   Availability(TM) Forum HPI-to-AdvancedTCA Mapping Specification.
**   It contains all of the prototypes and type definitions. Note, this 
**   file was generated from the HPI-to-AdvancedTCA mapping specification 
**   document.
**   
** SPECIFICATION VERSION:
**   SAIM-HPI-B.01.01-ATCA
**
** DATE: 
**   Fri  Nov   18  2005  010:11
**
** LEGAL:
**   OWNERSHIP OF SPECIFICATION AND COPYRIGHTS. 
**   The Specification and all worldwide copyrights therein are
**   the exclusive property of Licensor.  You may not remove, obscure, or
**   alter any copyright or other proprietary rights notices that are in or
**   on the copy of the Specification you download.  You must reproduce all
**   such notices on all copies of the Specification you make.  Licensor
**   may make changes to the Specification, or to items referenced therein,
**   at any time without notice.  Licensor is not obligated to support or
**   update the Specification. 
**   
**   Copyright 2004 by the Service Availability Forum. All rights reserved.
**
**   Permission to use, copy, modify, and distribute this software for any
**   purpose without fee is hereby granted, provided that this entire notice
**   is included in all copies of any software which is or includes a copy
**   or modification of this software and in all copies of the supporting
**   documentation for such software.
**
**   THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
**   WARRANTY.  IN PARTICULAR, THE SERVICE AVAILABILITY FORUM DOES NOT MAKE ANY
**   REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
**   OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
**
*******************************************************************************/

#ifndef __SAHPIATCA_H
#define __SAHPIATCA_H


/*
 * SaHpi.h contains the HPI data types that are used in this section
*/
#include "SaHpi.h"

/***********************************************************************
********
************************************************************************
********
**********                                                            
**********
**********                 Basic Data Types and Values                
**********
**********                                                            
**********
************************************************************************
********
************************************************************************
*******/

/* PICMG manufacturer identifier */
#define ATCAHPI_PICMG_MID       0x00315A

/***********************************************************************
********
************************************************************************
********
**********                                                            
**********
**********                 LED Specific Data Types                    
**********
**********                                                            
**********
************************************************************************
********
************************************************************************
*******/

/*
** Color Capabilities
**
** This definition defines the possible color capabilities for an 
individual 
** LED. One LED may support any number of colors using the bit mask. 
**
** ATCAHPI_LED_WHITE
** Indicates that the LED supports illumination in WHITE.
** ATCAHPI_LED_ORANGE
** Indicates that the LED supports illumination in ORANGE.
** ATCAHPI_LED_AMBER
** Indicates that the LED supports illumination in AMBER.
** ATCAHPI_LED_GREEN
** Indicates that the LED supports illumination in GREEN.
** ATCAHPI_LED_RED
** Indicates that the LED supports illumination in RED.
** ATCAHPI_LED_BLUE
** Indicates that the LED supports illumination in BLUE.
** 
*/
typedef SaHpiUint8T AtcaHpiColorCapabilitiesT;

#define ATCAHPI_BLINK_COLOR_LED    (AtcaHpiColorCapabilitiesT)0X80
#define ATCAHPI_LED_WHITE          (AtcaHpiColorCapabilitiesT)0x40
#define ATCAHPI_LED_ORANGE         (AtcaHpiColorCapabilitiesT)0x20
#define ATCAHPI_LED_AMBER          (AtcaHpiColorCapabilitiesT)0x10
#define ATCAHPI_LED_GREEN          (AtcaHpiColorCapabilitiesT)0x08
#define ATCAHPI_LED_RED            (AtcaHpiColorCapabilitiesT)0x04
#define ATCAHPI_LED_BLUE           (AtcaHpiColorCapabilitiesT)0x02

typedef enum {
    ATCAHPI_LED_COLOR_RESERVED = 0,
    ATCAHPI_LED_COLOR_BLUE,
    ATCAHPI_LED_COLOR_RED,
    ATCAHPI_LED_COLOR_GREEN,
    ATCAHPI_LED_COLOR_AMBER,
    ATCAHPI_LED_COLOR_ORANGE,
    ATCAHPI_LED_COLOR_WHITE,
    ATCAHPI_LED_COLOR_NO_CHANGE = 0x0E,
    ATCAHPI_LED_COLOR_USE_DEFAULT = 0x0F
} AtcaHpiLedColorT;

typedef enum {
    ATCAHPI_LED_AUTO,       /*Set all LEDs under this Resource to Auto 
mode */
    ATCAHPI_LED_MANUAL,     /*Set all LEDs under this Resource to Manual 
mode*/
    ATCAHPI_LED_LAMP_TEST   /*Put all LEDs under this Resource in Lamp 
Test */
} AtcaHpiResourceLedModeT;


typedef enum {
ATCAHPI_LED_BR_SUPPORTED,     /*Blinking Rates are Supported */
ATCAHPI_LED_BR_NOT_SUPPORTED, /*Blinking Rates are Not Supported */
ATCAHPI_LED_BR_UNKNOWN        /*The HPI Implementation cannot determine
                                whether or not
                                Blinking Rates are supported. */
} AtcaHpiLedBrSupportT;

/*
 * Control numbers for LED Controls.
 * Note:
 * 1. The Control numbers from ATCAHPI_CTRL_NUM_BLUE_LED to
 * ATCAHPI_CTRL_NUM_APP_LED + 0xFA are reserved for Application 
 * Specific LED Controls.
*/
#define ATCAHPI_CTRL_NUM_BLUE_LED   (SaHpiCtrlNumT)0x00
#define ATCAHPI_CTRL_NUM_LED1       (SaHpiCtrlNumT)0x01
#define ATCAHPI_CTRL_NUM_LED2       (SaHpiCtrlNumT)0x02
#define ATCAHPI_CTRL_NUM_LED3       (SaHpiCtrlNumT)0x03
#define ATCAHPI_CTRL_NUM_APP_LED    (SaHpiCtrlNumT)0x04

/*
 * Indicator in the Resource level LED Control RDR that indicates
 * that this is a Control that aggregates all LEDs of a particular 
Resource.
 */
#define ATCAHPI_RESOURCE_AGGREGATE_LED 0xFF

/***********************************************************************
********
************************************************************************
********
**********                                                            
**********
**********                 Entity Data Types                          
**********
**********                                                            
**********
************************************************************************
********
************************************************************************
*******/

/*
** ATCA Entity Definitions
** These definitions describe the ATCA specific Entity Types defined
** in this specification.
** Note that the entity types from, SAHPI_ENT_PHYSICAL_SLOT + 1 to 
** SAHPI_ENT_PHYSICAL_SLOT + 20h and SAHPI_ENT_BATTERY + 1 to 
** SAHPI_ENT_CHASSIS_SPECIFIC + Fh are reserved by this specification.      
**

** ATCAHPI_ENT_POWER_ENTRY_MODULE_SLOT - Power Entry Module slot
** ATCAHPI_ENT_SHELF_FRU_DEVICE_SLOT   - ATCA Shelf FRU information 
device slot
** ATCAHPI_ENT_SHELF_MANAGER_SLOT      - Dedicated Shelf Managemer Slot
** ATCAHPI_ENT_FAN_TRAY_SLOT           - Fan Tray slot
** ATCAHPI_ENT_FAN_FILTER_TRAY_SLOT    - Fan Tray Filter slot
** ATCAHPI_ENT_ALARM_SLOT              - Alarm Module slot
** ATCAHPI_ENT_AMC_SLOT                - ATCA AMC card slot
** ATCAHPI_ENT_PMC_SLOT                - PMC card slot
** ATCAHPI_ENT_RTM_SLOT                - Rear Transition Module slot
**
** ATCAHPI_ENT_PICMG_FRONT_BLADE       - ATCA Field Replaceable Front 
Board
** ATCAHPI_ENT_SHELF_FRU_DEVICE        - ATCA Shelf FRU Information 
Device
** ATCAHPI_ENT_FILTRATION_UNIT         - Air Filtration module                   
** ATCAHPI_ENT_AMC                     - ATCA AMC card
** 
*/

typedef SaHpiEntityTypeT AtcaHpiEntityTypeT;

#define ATCAHPI_ENT_POWER_ENTRY_MODULE_SLOT \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_CHASSIS_SPECIFIC + 1)
#define ATCAHPI_ENT_SHELF_FRU_DEVICE_SLOT   \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_CHASSIS_SPECIFIC + 2)
#define ATCAHPI_ENT_SHELF_MANAGER_SLOT      \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_CHASSIS_SPECIFIC + 3)
#define ATCAHPI_ENT_FAN_TRAY_SLOT           \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_CHASSIS_SPECIFIC + 4)
#define ATCAHPI_ENT_FAN_FILTER_TRAY_SLOT    \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_CHASSIS_SPECIFIC + 5)
#define ATCAHPI_ENT_ALARM_SLOT              \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_CHASSIS_SPECIFIC + 6)
#define ATCAHPI_ENT_AMC_SLOT                \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_CHASSIS_SPECIFIC + 7)
#define ATCAHPI_ENT_PMC_SLOT                \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_CHASSIS_SPECIFIC + 8)
#define ATCAHPI_ENT_RTM_SLOT                \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_CHASSIS_SPECIFIC + 9)

#define ATCAHPI_ENT_PICMG_FRONT_BLADE       \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 1)
#define ATCAHPI_ENT_SHELF_FRU_DEVICE        \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 2)
#define ATCAHPI_ENT_FILTRATION_UNIT         \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 3)
#define ATCAHPI_ENT_AMC                     \
                        (AtcaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 4)


/***********************************************************************
********
************************************************************************
********
**********                                                            
**********
**********                 Shelf Resource Data Types                  
**********
**********                                                            
**********
************************************************************************
********
************************************************************************
*******/

/*
 * Sensor/Control numbers for:
 * Shelf FRU Info Valid Sensor
 * Shelf Address Control
 * Shelf Manager IP Address Control
 * Chassis Status Control
 */
#define ATCAHPI_SENSOR_NUM_SHELF_INFO_VALID (SaHpiSensorNumT)0x1000
#define ATCAHPI_CTRL_NUM_SHELF_ADDRESS      (SaHpiCtrlNumT)0x1000
#define ATCAHPI_CTRL_NUM_SHELF_IP_ADDRESS   (SaHpiCtrlNumT)0x1001
#define ATCAHPI_CTRL_NUM_SHELF_STATUS       (SaHpiCtrlNumT)0x1002

/*
 * Sensor/Control numbers for:
 * FRU Power On Sequence Control
 * Note that the Control numbers from 
ATCAHPI_CTRL_NUM_FRU_POWER_ON_SEQUENCE to 
 * ATCAHPI_CTRL_NUM_FRU_POWER_ON_SEQUENCE + FEh are reserved for FRU 
Power On 
 * Sequence Controls.
 */
#define ATCAHPI_SENSOR_NUM_PWRONSEQ_COMMIT_STATUS       (SaHpiSensorNumT)0x1300
#define ATCAHPI_CTRL_NUM_FRU_POWER_ON_SEQUENCE_COMMIT   (SaHpiCtrlNumT)0x1300
#define ATCAHPI_CTRL_NUM_FRU_POWER_ON_SEQUENCE          (SaHpiCtrlNumT)0x1301

/*
 * Chassis Status Control
 */

/* Bitmask for Current Power State */
typedef SaHpiUint8T AtcaHpiCsCurrentPwrState;    /* Maps to OEM Control 
state  
                                                    byte [2] for the 
Chassis 
                                                    Status Control */

#define ATCAHPI_CS_CPS_PWR_ON                    (AtcaHpiCsCurrentPwrState)0x01 
/* If bit is set, powered on, powered off otherwise */ 
#define ATCAHPI_CS_CPS_PWR_OVERLOAD              (AtcaHpiCsCurrentPwrState)0x02 
/* If bit is set, chassis shut down due to power overload */
#define ATCAHPI_CS_CPS_INTERLOCK                 (AtcaHpiCsCurrentPwrState)0x04 
/* If bit is set, chassis shut down due to interlock switch being active 
*/
#define ATCAHPI_CS_CPS_PWR_FAULT                 (AtcaHpiCsCurrentPwrState)0x08 
/* If bit is set, fault detected in main power subsystem */
#define ATCAHPI_CS_CPS__PWR_CTRL_FAULT           (AtcaHpiCsCurrentPwrState)0x10 
/* If bit is set, fault in power Control to power up/down */
#define ATCAHPI_CS_CPS_PWR_RESTORE_PWR_UP_PREV   (AtcaHpiCsCurrentPwrState)0x20 
/* If bit is set, after power restoration, chassis is restored 
   to the state it was in, else chassis stays powered off after 
   power restoration */
#define ATCAHPI_CS_CPS_PWR_RESTORE_PWR_UP        (AtcaHpiCsCurrentPwrState)0x40 
/* If bit is set, chassis powers up after power restoration */
#define ATCAHPI_CS_CPS_PWR_RESTORE_UNKNOWN       (AtcaHpiCsCurrentPwrState)0x60 
/* If bits are set, unknown */

/* Bitmask for Last Power Event */
typedef SaHpiUint8T AtcaHpiCsLastPwrEvent;       /* maps to OEM Control 
state 
                                                    byte [3]for the 
Chassis 
                                                    Status Control */

#define ATCAHPI_CS_LPEVT_AC_FAILED               (AtcaHpiCsLastPwrEvent)0x01 
/* If bit is set, AC failed */
#define ATCAHPI_CS_LPEVT_PWR_OVERLOAD            (AtcaHpiCsLastPwrEvent)0x02 
/* If bit is set, last power down caused by a Power overload */
#define ATCAHPI_CS_LPEVT_PWR_INTERLOCK           (AtcaHpiCsLastPwrEvent)0x04 
/* If bit is set, last power down caused by a power interlock being 
activated */
#define ATCAHPI_CS_LPEVT_PWR_FAULT               (AtcaHpiCsLastPwrEvent)0x08 
/* If bit is set, last power down caused by power fault */
#define ATCAHPI_CS_LPEVT_PWRON_IPMI              (AtcaHpiCsLastPwrEvent)0x10 
/* If bit is set, last 'Power is on' state was entered via IPMI command 
*/

/* Bitmask for Miscellaneous Chassis State */
typedef SaHpiUint8T AtcaHpiCsMiscChassisState; 
/* maps to OEM Control state byte [4] for the Chassis Status Control */

#define ATCAHPI_CS_MISC_CS_INTRUSION_ACTIVE     (AtcaHpiCsMiscChassisState)0x01 
/* If bit is set, Chassis intrusion is active */
#define ATCAHPI_CS_MISC_CS_FP_LOCKOUT_ACTIVE    (AtcaHpiCsMiscChassisState)0x02 
/* If bit is set, Front Panel Lockout is active (power off and 
   reset via chassis push-buttons disabled) */
#define ATCAHPI_CS_MISC_CS_DRIVE_FAULT          (AtcaHpiCsMiscChassisState)0x04 
/* If bit is set, indicates Drive Fault */
#define ATCAHPI_CS_MISC_CS_COOLING_FAULT        (AtcaHpiCsMiscChassisState)0x08 
/* If bit is set, Cooling/fan fault detected */

/* Bitmask for Front Panel Button Capabilities and disable/enable status 
*/
typedef SaHpiUint8T AtcaHpiCsFpButtonCap;  
/* maps to OEM Control state byte [5] for the Chassis Status Control */

#define ATCAHPI_CS_FP_BUTTON_PWR_OFF                (AtcaHpiCsFpButtonCap)0x01 
/* If bit is set, power off button is disabled */
#define ATCAHPI_CS_FP_BUTTON_RESET_OFF              (AtcaHpiCsFpButtonCap)0x02 
/* If bit is set, reset button is disabled */
#define ATCAHPI_CS_FP_BUTTON_DIAGINTR_OFF           (AtcaHpiCsFpButtonCap)0x04 
/* If bit is set, diagnostic interrupt button is disabled */
#define ATCAHPI_CS_FP_BUTTON_STANDBY_OFF            (AtcaHpiCsFpButtonCap)0x08 
/* If bit is set, standby button is disabled */
#define ATCAHPI_CS_FP_BUTTON_ALLOW_PWR_OFF          (AtcaHpiCsFpButtonCap)0x10 
/* If bit is set, power off button disable is allowed */
#define ATCAHPI_CS_FP_BUTTON_ALLOW_RESET_OFF        (AtcaHpiCsFpButtonCap)0x20 
/* If bit is set, reset button disable is allowed */
#define ATCAHPI_CS_FP_BUTTON_ALLOW_DIAGINTR_OFF     (AtcaHpiCsFpButtonCap)0x40 
/* If bit is set, diagnostic interrupt button disable is allowed */
#define ATCAHPI_CS_FP_BUTTON_ALLOW_STANDBY_OFF      (AtcaHpiCsFpButtonCap)0x80 
/* If bit is set, standby button disable is allowed */

/***********************************************************************
********
************************************************************************
********
**********                                                            
**********
**********              Shelf Manager Resource Data Types             
**********
**********                                                            
**********
************************************************************************
********
************************************************************************
*******/
/*
 * Sensor/Control numbers for:
 * Shelf Manager Failover Control
 * Shelf Manager Redundancy Sensor
 * Active Shelf Manager Sensor
 * Standby Shelf Manager Sensor
 */

#define ATCAHPI_SENSOR_NUM_SHMGR_REDUNDANCY             (SaHpiSensorNumT)0x1001
#define ATCAHPI_SENSOR_NUM_SHMGR_ACTIVE                 (SaHpiSensorNumT)0x1002
#define ATCAHPI_SENSOR_NUM_SHMGR_STANDBY                (SaHpiSensorNumT)0x1003
#define ATCAHPI_CTRL_NUM_SHMGR_FAILOVER                 (SaHpiCtrlNumT)0x1010
#define ATCAHPI_CTRL_NUM_FAILED_RESOURCE_EXTRACT        (SaHpiCtrlNumT)0x101E

/***********************************************************************
********
************************************************************************
********
**********                                                            
**********
**********               FRU Slot Resource Data Types                 
**********
**********                                                            
**********
************************************************************************
********
************************************************************************
*******/
/*
 * Control/Sensor numbers for:
 * FRU Activation Control
 * Slot State Sensor
 * Assigned Power Sensor
 * Maximum Power Sensor
 */
#define ATCAHPI_SENSOR_NUM_SLOT_STATE                   (SaHpiSensorNumT)0x1010
#define ATCAHPI_SENSOR_NUM_ASSIGNED_PWR                 (SaHpiSensorNumT)0x1011
#define ATCAHPI_SENSOR_NUM_MAX_PWR                      (SaHpiSensorNumT)0x1012
#define ATCAHPI_CTRL_NUM_FRU_ACTIVATION                 (SaHpiCtrlNumT)0x1020

/***********************************************************************
********
************************************************************************
********
**********                                                            
**********
**********                 FRU Resource Data Types                    
**********
**********                                                            
**********
************************************************************************
********
************************************************************************
*******/
/*
 * Sensor/Control numbers for:
 * IPMB0 Sensor
 * - Note that the Sensor numbers from ATCAHPI_SENSOR_NUM_IPMB0 to
 *   ATCAHPI_SENSOR_NUM_IPMB0 + 0x5F are reserved for systems supporting 
a 
 *   radial IPMB requiring multiple IPMB0 Sensors.
 * Desired Power Control
 * IPMI A/B State Control
 */

#define ATCAHPI_SENSOR_NUM_IPMB0                        (SaHpiSensorNumT)0x1100
#define ATCAHPI_CTRL_NUM_DESIRED_PWR                    (SaHpiCtrlNumT)0x1030
#define ATCAHPI_CTRL_NUM_IPMB_A_STATE                   (SaHpiCtrlNumT)0x1101 
#define ATCAHPI_CTRL_NUM_IPMB_B_STATE                   (SaHpiCtrlNumT)0x1102

/*
 * Control numbers for FRU Control and FRU IPM Controller reset.

 */

#define ATCAHPI_CTRL_NUM_FRU_CONTROL                    (SaHpiCtrlNumT)0x1200
#define ATCAHPI_CTRL_NUM_FRU_IPMC_RESET                 (SaHpiCtrlNumT)0x1201

/*
 * Sensor/Control numbers for:
 * AMC Power On Sequence Control
 * Note that the Control numbers from 
ATCAHPI_CTRL_NUM_AMC_POWER_ON_SEQUENCE to 
 * ATCAHPI_CTRL_NUM_AMC_POWER_ON_SEQUENCE + Fh are reserved for AMC 
Power On 
 * Sequence Controls.
 */
#define ATCAHPI_SENSOR_NUM_AMC_PWRONSEQ_COMMIT_STATUS   (SaHpiSensorNumT)0x1500
#define ATCAHPI_CTRL_NUM_AMC_POWER_ON_SEQUENCE_COMMIT   (SaHpiCtrlNumT)0x1500
#define ATCAHPI_CTRL_NUM_AMC_POWER_ON_SEQUENCE          (SaHpiCtrlNumT)0x1501

/***********************************************************************
********
************************************************************************
********
**********                                                            
**********
**********                 Fan Control Data Types                     
**********
**********                                                            
**********
************************************************************************
********
************************************************************************
*******/
/*
 * Control number for fan speed.
 */
#define ATCAHPI_CTRL_NUM_FAN_SPEED                      (SaHpiCtrlNumT)0x1400


/***********************************************************************
********
************************************************************************
********
**********                                                            
**********
**********                OEM Control Data Types                      
**********
**********                                                            
**********
************************************************************************
********
************************************************************************
*******/

/*
 * Well-known OEM values assigned to RDR entries for OEM Controls 
defined by 
 * this document.
 */
#define ATCAHPI_PICMG_CT_CHASSIS_STATUS         0x0100315A
#define ATCAHPI_PICMG_CT_ATCA_LED               0x0200315A
 
#endif // __SAHPIATCA_H
