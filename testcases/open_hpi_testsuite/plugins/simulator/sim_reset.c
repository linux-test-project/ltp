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


SaErrorT sim_get_reset_state(void *hnd,
			     SaHpiResourceIdT rid,
			     SaHpiResetActionT *act)
{
	if (!hnd || !act) {
		err("Invalid parameter.");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

	/* Check if resource exists and has reset capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
		return SA_ERR_HPI_INVALID_RESOURCE;
	}

        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_RESET)) {
		return SA_ERR_HPI_CAPABILITY;
	}

	*act = SAHPI_RESET_DEASSERT;
	return SA_OK;
}


SaErrorT sim_set_reset_state(void *hnd,
			     SaHpiResourceIdT rid,
			     SaHpiResetActionT act)
{
	if (!hnd || NULL == oh_lookup_resetaction(act)){
		err("Invalid parameter.");
		return SA_ERR_HPI_INVALID_PARAMS;
	}
	if (act == SAHPI_RESET_ASSERT || act == SAHPI_RESET_DEASSERT)
                return SA_ERR_HPI_INVALID_CMD;

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

	/* Check if resource exists and has reset capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
		return SA_ERR_HPI_INVALID_RESOURCE;
	}

        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_RESET)) {
		return SA_ERR_HPI_CAPABILITY;
	}

        return SA_OK;
}


void * oh_get_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT *)
                __attribute__ ((weak, alias("sim_get_reset_state")));

void * oh_set_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT)
                __attribute__ ((weak, alias("sim_set_reset_state")));

