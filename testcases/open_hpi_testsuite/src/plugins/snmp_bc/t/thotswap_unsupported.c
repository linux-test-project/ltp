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

/*	struct snmp_bc_hnd snmp_handle; */
	struct oh_handler_state hnd = {
		.rptcache = NULL,
		.eventq = NULL,
		.config = NULL,
		.data = NULL,
	};

	SaHpiResourceIdT        id = 1;
	SaHpiHsStateT           state = SAHPI_HS_STATE_NOT_PRESENT;
	SaHpiHsIndicatorStateT  ind_state = SAHPI_HS_INDICATOR_OFF;
	SaHpiHsActionT          act = SAHPI_HS_ACTION_INSERTION;
	SaErrorT                err;

	err = snmp_bc_get_indicator_state((void *)&hnd, id, &ind_state);
	if (err != SA_ERR_HPI_INVALID_CMD) {
		printf("snmp_bc_get_indicator_state should return SA_ERR_HPI_INVALID_CMD\n");
		return -1; 
	}

	err = snmp_bc_set_indicator_state((void *)&hnd, id, ind_state);
	if (err != SA_ERR_HPI_INVALID_CMD) {
		printf("snmp_bc_set_indicator_state should return SA_ERR_HPI_INVALID_CMD\n");
		return -1; }
	    
	err = snmp_bc_set_hotswap_state((void *)&hnd, id, state);
	if (err != SA_ERR_HPI_UNSUPPORTED_API) {
		printf("snmp_bc_set_hotswap_state should return SA_ERR_HPI_UNSUPPORTED_API\n");
		return -1;
	}

	err = snmp_bc_request_hotswap_action((void *)&hnd, id, act);
	if (err != SA_ERR_HPI_UNSUPPORTED_API) {
		printf("snmp_bc_request_hotswap_action should return SA_ERR_HPI_UNSUPPORTED_API\n");
		return -1;
	}

	return 0;
}

/****************
 * Stub Functions
 ****************/
#include <tstubs_res.c>
#include <tstubs_snmp.c>
