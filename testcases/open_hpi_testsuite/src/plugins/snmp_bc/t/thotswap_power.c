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

	SaHpiResourceIdT   id = 1;
	SaHpiHsPowerStateT state;
	SaErrorT           err;
	SaHpiHsPowerStateT expected_value;
	
	/******************************** 
	 * Get Power State (Off) TestCase
	 ********************************/
	snmp_value_integer = 0;
	expected_value= SAHPI_HS_POWER_OFF;

	err = snmp_bc_get_power_state((void *)&hnd, id, &state);
	if (err) {
		printf("Error! Get Power State (Off) TestCase\n");
		printf("Error! snmp_bc_get_power_state returned err=%d\n", err);
		return -1;
	}

	if (state != expected_value) {
		printf("Error! Get Power State (Off) TestCase\n");
		printf("Error! snmp_bc_get_power_state unexpected value=%d\n", state);
		return -1;
	}

	/******************************** 
	 * Get Power State (On) TestCase
	 ********************************/
	snmp_value_integer = 1;
	expected_value= SAHPI_HS_POWER_ON;

	err = snmp_bc_get_power_state((void *)&hnd, id, &state);
	if (err) {
		printf("Error! Get Power State (On) TestCase\n");
		printf("Error! snmp_bc_get_power_state returned err=%d\n", err);
		return -1;
	}

	if (state != expected_value) {
		printf("Error! Get Power State (On) TestCase\n");
		printf("Error! snmp_bc_get_power_state unexpected value=%d\n", state);
		return -1;
	}

	return 0;
}

/****************
 * Stub Functions
 ****************/
#include <tstubs_res.c>
#include <tstubs_snmp.c>
