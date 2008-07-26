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
 *      W. David Ashley <dashley@us.ibm.com>
 */

#include <sim_init.h>


SaErrorT sim_get_power_state(void *hnd,
	        	     SaHpiResourceIdT rid,
			     SaHpiPowerStateT *pwrstate)
{
	if (!hnd || !pwrstate) {
		err("Invalid parameter");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

	/* Check if resource exists and has power capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
		return SA_ERR_HPI_INVALID_RESOURCE;
	}

        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_POWER)) {
		return SA_ERR_HPI_CAPABILITY;
	}
	
	struct simResourceInfo *sim_rinfo = oh_get_resource_data(state->rptcache, rid);
	if (!sim_rinfo) return SA_ERR_HPI_NOT_PRESENT;

        *pwrstate = sim_rinfo->cur_powerstate;
        return SA_OK;
}


SaErrorT sim_set_power_state(void *hnd,
			     SaHpiResourceIdT rid,
			     SaHpiPowerStateT pwrstate)
{
	if (!hnd || NULL == oh_lookup_powerstate(pwrstate)) {
		err("Invalid parameter.");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

	/* Check if resource exists and has power capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
		return SA_ERR_HPI_INVALID_RESOURCE;
	}

        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_POWER)) {
		return SA_ERR_HPI_CAPABILITY;
	}

	struct simResourceInfo *sim_rinfo = oh_get_resource_data(state->rptcache, rid);
	if (!sim_rinfo) return SA_ERR_HPI_NOT_PRESENT;
	
	sim_rinfo->cur_powerstate = pwrstate;

        return SA_OK;
}


void * oh_get_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT *)
                __attribute__ ((weak, alias("sim_get_power_state")));

void * oh_set_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT)
                __attribute__ ((weak, alias("sim_set_power_state")));


