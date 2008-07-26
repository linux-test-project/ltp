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
 * snmp_bc_get_hotswap_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @state: Location to store resource's hotswap state.
 *
 * Retrieves a managed hotswap resource's state.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_MANAGED_HOTSWAP.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_get_hotswap_state(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiHsStateT *state)
{
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	struct ResourceInfo *resinfo;
	SaHpiRptEntryT *rpt;

	if (!hnd || !state) {
		err("Invalid parameter.");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has managed hotswap capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	resinfo = (struct ResourceInfo *)oh_get_resource_data(handle->rptcache, rid);
	if (!resinfo) {
		err("No resource data for %s", rpt->ResourceTag.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Set current hot swap state */
	*state = resinfo->cur_state;

	snmp_bc_unlock_handler(custom_handle);
	return(SA_OK);
}

/**
 * snmp_bc_set_hotswap_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @state: Hotswap state to set.
 *
 * Sets a managed hotswap resource's state.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_MANAGED_HOTSWAP.
 * SA_ERR_HPI_INVALID_REQUEST - @state invalid.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_set_hotswap_state(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiHsStateT state)
{
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	struct ResourceInfo *resinfo;
	SaHpiRptEntryT *rpt;

	if (!hnd) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if (NULL == oh_lookup_hsstate(state)) {
		err("Invalid hotswap state.");
		return(SA_ERR_HPI_INVALID_REQUEST);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has managed hotswap capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	resinfo = (struct ResourceInfo *)oh_get_resource_data(handle->rptcache, rid);
	if (!resinfo) {
		err("No resource data for %s", rpt->ResourceTag.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	if (resinfo->cur_state != SAHPI_HS_STATE_INSERTION_PENDING ||
	    resinfo->cur_state != SAHPI_HS_STATE_EXTRACTION_PENDING) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_REQUEST);
	}

	/* Don't currently support managed hotswap resources that don't have
           immediate/read-only auto insertion/extraction policies If this type
	   of hardware is supported in the future, need to add:
	   - indicators in resinfo structure for:
             - auto-insertion timeout not started or cancelled
             - auto-extraction timeout not started or cancelled
           - set previous/current hotswap states
           - generate event for transition to active state
         */
 	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INVALID_REQUEST);
}

/**
 * snmp_bc_request_hotswap_action:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @act: Hotswap state to set.
 *
 * Sets a managed hotswap resource's insertion or extraction action.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_MANAGED_HOTSWAP.
 * SA_ERR_HPI_INVALID_REQUEST - @act invalid.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_request_hotswap_action(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiHsActionT act)
{
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	struct ResourceInfo *resinfo;
	SaHpiRptEntryT *rpt;

	if (!hnd) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if (NULL == oh_lookup_hsaction(act)) {
		err("Invalid hotswap action.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has managed hotswap capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	} 
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	resinfo = (struct ResourceInfo *)oh_get_resource_data(handle->rptcache, rid);
	if (!resinfo) {
		err("No resource data for %s", rpt->ResourceTag.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Issue power-on command for insertion */
	if (act == SAHPI_HS_ACTION_INSERTION) {
		if (resinfo->cur_state == SAHPI_HS_STATE_INACTIVE) {
			SaErrorT err;
			
			err = snmp_bc_set_power_state(hnd, rid, SAHPI_POWER_ON);
			if (err) {
				err("%s resource does not support power on", rpt->ResourceTag.Data);
				snmp_bc_unlock_handler(custom_handle);
				return(SA_ERR_HPI_INTERNAL_ERROR);
			}
		}
		else {
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INVALID_REQUEST);
		}
	}

	/* Issue power-off command for extraction */
	if (act == SAHPI_HS_ACTION_EXTRACTION) {
		if (resinfo->cur_state == SAHPI_HS_STATE_ACTIVE) {
			SaErrorT err;
			
			err = snmp_bc_set_power_state(hnd, rid, SAHPI_POWER_OFF);
			if (err) {
				err("%s resource does not support power off", rpt->ResourceTag.Data);
				snmp_bc_unlock_handler(custom_handle);
				return(SA_ERR_HPI_INTERNAL_ERROR);
			}
		}
		else {
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INVALID_REQUEST);
		}
	}

	snmp_bc_unlock_handler(custom_handle);
	return(SA_OK);
}

/**
 * snmp_bc_get_indicator_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @state: Location to store the hotswap indicator state.
 *
 * Gets a managed hotswap resource's hotswap indicator state.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_MANAGED_HOTSWAP.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_get_indicator_state(void *hnd,
				     SaHpiResourceIdT rid,
				     SaHpiHsIndicatorStateT *state)
{
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;

	if (!hnd || !state) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;

	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has managed hotswap capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) ||
	    !(rpt->HotSwapCapabilities & SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	err("Hotswap indicators are not supported by platform");
	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INTERNAL_ERROR);
}

/**
 * snmp_bc_set_indicator_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @state: Hotswap indicator state to set.
 *
 * Sets a managed hotswap resource's hotswap indicator.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_MANAGED_HOTSWAP.
 * SA_ERR_HPI_INVALID_REQUEST - @state invalid.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_set_indicator_state(void *hnd,
				     SaHpiResourceIdT rid,
				     SaHpiHsIndicatorStateT state)
{
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;

	if (!hnd) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if (NULL == oh_lookup_hsindicatorstate(state)) {
		err("Invalid hotswap indicator state.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;

	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has managed hotswap capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) ||
	    !(rpt->HotSwapCapabilities & SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	err("Hotswap indicators are not supported by platform");
	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INTERNAL_ERROR);
}

/**
 * snmp_bc_set_autoinsert_timeout:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @timeout: timeout to set.
 *
 * Set hotswap autoinsert timeout.
 * 
 * Return values:
 * SA_ERR_HPI_READ_ONLY - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_set_autoinsert_timeout(void *hnd,
				     SaHpiTimeoutT Timeout)
{
	if (!hnd) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	return(SA_ERR_HPI_READ_ONLY);

}

/**
 * snmp_bc_get_autoextract_timeout:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @timeout: Storage for returned timeout value.
 *
 * Get a resource's hotswap autoextract timeout.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_get_autoextract_timeout(void *hnd,
				     SaHpiResourceIdT rid, 
				     SaHpiTimeoutT *Timeout)
{
	if (!hnd || !Timeout) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	*Timeout = SAHPI_TIMEOUT_IMMEDIATE;
	return(SA_OK);

}

/**
 * snmp_bc_set_autoextract_timeout:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @timeout: timeout to set.
 *
 * Set a resource hotswap autoextract timeout.
 * 
 * Return values:
 * SA_ERR_HPI_READ_ONLY - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_set_autoextract_timeout(void *hnd,
				     SaHpiResourceIdT rid, 
				     SaHpiTimeoutT Timeout)
{
	if (!hnd) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	return(SA_ERR_HPI_READ_ONLY);

}

void * oh_get_hotswap_state (void *, SaHpiResourceIdT, SaHpiHsStateT *)
                __attribute__ ((weak, alias("snmp_bc_get_hotswap_state")));

void * oh_set_hotswap_state (void *, SaHpiResourceIdT, SaHpiHsStateT)
                __attribute__ ((weak, alias("snmp_bc_set_hotswap_state")));

void * oh_request_hotswap_action (void *, SaHpiResourceIdT, SaHpiHsActionT)
                __attribute__ ((weak, alias("snmp_bc_request_hotswap_action")));

void * oh_set_indicator_state (void *, SaHpiResourceIdT, SaHpiHsIndicatorStateT)
                __attribute__ ((weak, alias("snmp_bc_set_indicator_state")));
		
void * oh_get_indicator_state (void *, SaHpiResourceIdT, SaHpiHsIndicatorStateT)
                __attribute__ ((weak, alias("snmp_bc_get_indicator_state")));

void * oh_set_autoinsert_timeout (void *, SaHpiTimeoutT)
                __attribute__ ((weak, alias("snmp_bc_set_autoinsert_timeout")));				
		
void * oh_get_autoextract_timeout (void *, SaHpiResourceIdT, SaHpiTimeoutT *)
                __attribute__ ((weak, alias("snmp_bc_get_autoextract_timeout")));
				
void * oh_set_autoextract_timeout (void *, SaHpiResourceIdT, SaHpiTimeoutT)
                __attribute__ ((weak, alias("snmp_bc_set_autoextract_timeout")));		
		
		


