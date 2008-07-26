/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <snmp_bc_plugin.h>

/**
 * snmp_bc_get_power_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @state: Location to store resource's power state.
 *
 * Retrieves a resource's power state.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_POWER.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_get_power_state(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiPowerStateT *state)
{
	SaErrorT err;
	struct ResourceInfo *resinfo;
        struct snmp_value get_value;
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;


	if (!hnd || !state) {
		err("Invalid parameter");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	err = SA_OK;
	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has power capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_POWER)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	resinfo = (struct ResourceInfo *)oh_get_resource_data(handle->rptcache, rid);
 	if (resinfo == NULL) {
		err("No resource data. Resource=%s", rpt->ResourceTag.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       
	if (resinfo->mib.OidPowerState == NULL) {
		err("No Power OID.");
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Read power state of resource */
	err = snmp_bc_oid_snmp_get(custom_handle, &(rpt->ResourceEntity), 0,
				   resinfo->mib.OidPowerState, &get_value, SAHPI_TRUE);
	if (!err && (get_value.type == ASN_INTEGER)) {
		switch (get_value.integer) {
		case 0:
			*state = SAHPI_POWER_OFF;
			break;
		case 1:
			*state = SAHPI_POWER_ON;
			break;
		default:
			err("Invalid power state for OID=%s.", resinfo->mib.OidPowerState);
		        err = SA_ERR_HPI_INTERNAL_ERROR;
		}
        } else {
		err("Cannot read SNMP OID=%s; Type=%d.", resinfo->mib.OidPowerState, get_value.type);
	}

	snmp_bc_unlock_handler(custom_handle);
        return(err);
}

/**
 * snmp_bc_set_power_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @state: Resource's power state to set.
 *
 * Sets a resource's power state.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_POWER.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL; @state invalid.
 **/
SaErrorT snmp_bc_set_power_state(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiPowerStateT state)
{
	SaErrorT err;
	struct ResourceInfo *resinfo;
        struct snmp_value set_value;
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;

	if (!hnd || NULL == oh_lookup_powerstate(state)) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	err = SA_OK;
	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has power capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_POWER)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	resinfo = (struct ResourceInfo *)oh_get_resource_data(handle->rptcache, rid);
 	if (resinfo == NULL) {
		err("No resource data. Resource=%s", rpt->ResourceTag.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       
	if (resinfo->mib.OidPowerOnOff == NULL) {
		err("No Power OnOff OID.");
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Set power on/off */
	set_value.type = ASN_INTEGER;
	set_value.str_len = 1;

	switch (state) {
	case SAHPI_POWER_OFF:
		set_value.integer = 0;
		err = snmp_bc_oid_snmp_set(custom_handle, &(rpt->ResourceEntity), 0,
					 resinfo->mib.OidPowerOnOff, set_value);
		if (err) {
			err("Cannot set SNMP OID=%s; Type=%d.",
			    resinfo->mib.OidPowerOnOff, set_value.type);
			if (err != SA_ERR_HPI_BUSY) err = SA_ERR_HPI_NO_RESPONSE;
		}
		break;
	case SAHPI_POWER_ON:
		set_value.integer = 1;
		err = snmp_bc_oid_snmp_set(custom_handle, &(rpt->ResourceEntity), 0,
						 resinfo->mib.OidPowerOnOff, set_value);
		if (err) {
			err("Cannot set SNMP OID=%s; Type=%d.",
			    resinfo->mib.OidPowerOnOff, set_value.type);
			if (err != SA_ERR_HPI_BUSY) err = SA_ERR_HPI_NO_RESPONSE;
		}
		break;
	case SAHPI_POWER_CYCLE:
	        {
			SaHpiResetActionT act = SAHPI_COLD_RESET;
			err = snmp_bc_set_reset_state(hnd, rid, act);
	        }
		break;
	default:
		err("Invalid Power Action Type=%d.", state);
		err = SA_ERR_HPI_INTERNAL_ERROR;
	}

	snmp_bc_unlock_handler(custom_handle);
        return(err);
}


void * oh_get_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT *)
                __attribute__ ((weak, alias("snmp_bc_get_power_state")));

void * oh_set_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT)
                __attribute__ ((weak, alias("snmp_bc_set_power_state")));
