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
 *	  Christina Hernandez <hernanc@us.ibm.com>
 *        W. David Ashley <dashley@us.ibm.com>
 */


#ifndef __SIM_RESOURCES_H
#define __SIM_RESOURCES_H

#include <SaHpi.h>
#include <sim_sensors.h>

/* Start HPI location numbers from 0 */
#define SIM_HPI_LOCATION_BASE 1

/* IBM Manufacturing Number */
/* Corresponding to change made in snmp_bc_resources.h */
#define IBM_MANUFACTURING_ID 20944

/* Resource Ids for SIM resources
   if the sim_rpt_array order changes this will need to change also */
typedef enum {
	SIM_RPT_ENTRY_CHASSIS = 1,
	SIM_RPT_ENTRY_CPU,
        SIM_RPT_ENTRY_DASD,
        SIM_RPT_ENTRY_HS_DASD,
	SIM_RPT_ENTRY_FAN
} SimRptEntryT;

/* Maximum number of SIM resources */
#define SIM_MAX_CPU    8
#define SIM_MAX_FAN    8
#define SIM_MAX_DASD   4

/* Maximum entries in eventlog */
#define SIM_EL_MAX_SIZE 256

/**********************
 * Resource Definitions
 **********************/

#define SIM_MAX_EVENTS_PER_RESOURCE 10
#define SIM_MAX_RESOURCE_EVENT_ARRAY_SIZE (SIM_MAX_EVENTS_PER_RESOURCE + 1)
                                       /* Includes an ending NULL entry */

struct simResourceInfo {
        SaHpiHsStateT cur_hsstate;
        SaHpiHsIndicatorStateT cur_indicator_hsstate;
        SaHpiPowerStateT cur_powerstate;
        SaHpiTimeoutT ae_timeout;
};

struct sim_rpt {
        SaHpiRptEntryT rpt;
        const  char *comment;
};

extern struct sim_rpt sim_rpt_array[];
extern struct sim_sensor sim_chassis_sensors[];
extern struct sim_sensor sim_cpu_sensors[];
extern struct sim_sensor sim_dasd_sensors[];
extern struct sim_sensor sim_hs_dasd_sensors[];
extern struct sim_sensor sim_fan_sensors[];

#endif

