/* -*- linux-c -*-
 * 
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *     Steve Sherman <stevees@us.ibm.com>
 */

#include <glib.h>
#include <SaHpi.h>

#include <oh_plugin.h>
#include <rpt_utils.h>
#include <snmp_util.h>
#include <snmp_bc.h>
#include <bc_resources.h>
#include <snmp_bc_utils.h>
#include <snmp_bc_hotswap.h>

#include <tstubs_res.h>
#include <tstubs_snmp.h>
#include <thotswap.h> 

int main(int argc, char **argv) 
{

	struct snmp_bc_hnd snmp_handle;
	struct oh_handler_state hnd = {
		.rptcache = (RPTable *)&test_rpt,
		.eventq = NULL,
		.config = NULL,
		.data = (void *)&snmp_handle,
	};

#if 0
	/* Fill in RPT Entry */
	test_rpt.rpt.ResourceTag.DataType = SAHPI_TL_TYPE_LANGUAGE;
	test_rpt.rpt.ResourceTag.Language = SAHPI_LANG_ENGLISH;
	test_rpt.rpt.ResourceTag.DataLength = strlen(test_rpt.comment);
	strcpy(test_rpt.rpt.ResourceTag.Data, test_rpt.comment);
#endif

	SaHpiResourceIdT    id = 1;
	SaHpiHsPowerStateT  state;
	SaErrorT            err;

	/***************************** 
	 * Get Resource Error TestCase
         *****************************/
	state = SAHPI_HS_POWER_OFF;
	ifobj_force_error = 1;

	err = snmp_bc_set_power_state((void *)&hnd, id, state);
	if (err != SA_ERR_HPI_NOT_PRESENT) {
		printf("Error! Get Resource Error TestCase\n");
		printf("Error! snmp_bc_set_power_state returned err=%d\n", err);
		return -1; 
	}

	ifobj_force_error = 0;

	/********************************** 
	 * Get Resource Data Error TestCase
         **********************************/
	state = SAHPI_HS_POWER_OFF;
	ifobj_data_force_error = 1;

	err = snmp_bc_set_power_state((void *)&hnd, id, state);
	if (err != -1) {
		printf("Error! Get Resource Data Error TestCase\n");
		printf("Error! snmp_bc_set_power_state returned err=%d\n", err);
		return -1; 
	}

	ifobj_data_force_error = 0;
	
	/************************************* 
	 *  No Hot Swap PowerOnOffOID TestCase
	 *************************************/
	state = SAHPI_HS_POWER_OFF;
	test_rpt.mib.OidPowerOnOff = '\0';

	err = snmp_bc_set_power_state((void *)&hnd, id, state);
	if (err != SA_ERR_HPI_INVALID_CMD) {
		printf("Error! No Hot Swap PowerOnOffOID TestCase\n");
		printf("Error! snmp_bc_set_power_state returned err=%d\n", err);
		return -1;
	}
	
	test_rpt.mib.OidPowerOnOff = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.x";

	/**************************************** 
	 * SNMP Error SAHPI_HS_POWER_OFF TestCase
         ****************************************/
	snmp_force_error = 1;
	state = SAHPI_HS_POWER_OFF;

	err = snmp_bc_set_power_state((void *)&hnd, id, state);
	if (err == 0) {
		printf("Error! SNMP Error SAHPI_HS_POWER_OFF TestCase\n");
		printf("Error! snmp_bc_set_power_state returned err=%d\n", err);
		return -1; 
	}

	snmp_force_error = 0;

	/**************************************** 
	 * SNMP Error SAHPI_HS_POWER_ON TestCase
         ****************************************/
	snmp_force_error = 1;
	state = SAHPI_HS_POWER_ON;

	err = snmp_bc_set_power_state((void *)&hnd, id, state);
	if (err == 0) {
		printf("Error! SNMP Error SAHPI_HS_POWER_ON TestCase\n");
		printf("Error! snmp_bc_set_power_state returned err=%d\n", err);
		return -1; 
	}

	snmp_force_error = 0;

	/*************************** 
	 * Derive OID Error TestCase
         ***************************/
	state = SAHPI_HS_POWER_OFF;
	test_rpt.rpt.ResourceEntity.Entry[0].EntityType = 0;

	err = snmp_bc_set_power_state((void *)&hnd, id, state);
	if (err == 0) {
		printf("Error! Derive OID Error TestCase\n");
		printf("Error! snmp_bc_set_power_state returned err=%d\n", err);
		return -1; 
	}

	test_rpt.rpt.ResourceEntity.Entry[0].EntityType = SAHPI_ENT_SBC_BLADE;

	/************************************ 
	 * Unrecognized Power Action TestCase
         ************************************/
	state = -1;

	err = snmp_bc_set_power_state((void *)&hnd, id, state);
	if (err == 0) {
		printf("Error!  Unrecognized Power Action TestCase\n");
		printf("Error! snmp_bc_set_power_state returned err=%d\n", err);
		return -1; 
	}

	return 0;
}

/****************
 * Stub Functions
 ****************/
#include <tstubs_res.c>
#include <tstubs_snmp.c>
