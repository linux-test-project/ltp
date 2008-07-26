/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <snmp_bc_plugin.h>

/**
 * snmp_bc_get_next_announce:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @aid: Annunciator ID.
 * @sev: Severity.
 * @unackonly: Boolean to get unacknowledged annunicators only.
 *
 * Gets the next annunicator.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_ANNUNICATOR.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL @sev invalid.
 **/
SaErrorT snmp_bc_get_next_announce(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiAnnunciatorNumT aid,
				   SaHpiSeverityT sev,
				   SaHpiBoolT unackonly,
				   SaHpiAnnouncementT *announcement)
{

        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;

	if (!hnd || !announcement || oh_lookup_severity(sev) == NULL) {
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
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_ANNUNCIATOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}


	err("Annunciators not supported by platform");
	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INTERNAL_ERROR);
}
        
/**
 * snmp_bc_get_announce:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @aid: Annunciator ID.
 * @entry: Annunicator's announcement ID.
 * @announcment: Location to store annunicator's announcement.
 *
 * Gets the annunicator's announcement information.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_ANNUNICATOR.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_get_announce(void *hnd,
			      SaHpiResourceIdT rid,
			      SaHpiAnnunciatorNumT aid,
			      SaHpiEntryIdT entry,
			      SaHpiAnnouncementT *announcement)
 {
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
	
	if (!hnd || !announcement) {
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
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_ANNUNCIATOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	err("Annunciators not supported by platform");
	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INTERNAL_ERROR);
 }

/**
 * snmp_bc_ack_announce:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @aid: Annunciator ID.
 * @entry: Annunicator's announcement ID.
 * @sev: Severity.
 *
 * Acknowledge an annunicator's announcement(s).
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_ANNUNICATOR.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL; @sev invalid.
 **/
SaErrorT snmp_bc_ack_announce(void *hnd,
			      SaHpiResourceIdT rid,
			      SaHpiAnnunciatorNumT aid,
			      SaHpiEntryIdT entry,
			      SaHpiSeverityT sev)
{

        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
	
	if (!hnd || oh_lookup_severity(sev) == NULL) {
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
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_ANNUNCIATOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}


	err("Annunciators not supported by platform");
	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INTERNAL_ERROR);
}
 
/**
 * snmp_bc_add_announce:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @aid: Annunciator ID.
 * @announcement: Pointer to annunicator's announcement data.
 *
 * Add an announcement to an annunicator.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_ANNUNICATOR.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_add_announce(void *hnd,
			      SaHpiResourceIdT rid, 
			      SaHpiAnnunciatorNumT aid,
			      SaHpiAnnouncementT *announcement)
{

        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;

	if (!hnd || !announcement) {
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
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_ANNUNCIATOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}


	err("Annunciators not supported by platform");
	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INTERNAL_ERROR);
}

/**
 * snmp_bc_del_announce:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @aid: Annunciator ID.
 * @entry: Annunicator's announcement ID.
 * @sev: Severity.
 *
 * Delete announcement(s) from an annunicator.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_ANNUNICATOR.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL; @sev invalid.
 **/
SaErrorT snmp_bc_del_announce(void *hnd,
			      SaHpiResourceIdT rid,
			      SaHpiAnnunciatorNumT aid,
			      SaHpiEntryIdT entry,
			      SaHpiSeverityT sev)
{

        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;

	if (!hnd || oh_lookup_severity(sev) == NULL) {
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
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_ANNUNCIATOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}
	

	err("Annunciators not supported by platform");
	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INTERNAL_ERROR);
}
        
/**
 * snmp_bc_get_annunc_mode:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @aid: Annunciator ID.
 * @mode: Location to store mode information.
 *
 * Gets an annunciator's current operating mode.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_ANNUNICATOR.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_get_annunc_mode(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiAnnunciatorNumT aid,
				 SaHpiAnnunciatorModeT *mode)
{

        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;

	if (!hnd || !mode) {
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
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_ANNUNCIATOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}


	err("Annunciators not supported by platform");
	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INTERNAL_ERROR);
}

/**
 * snmp_bc_get_annunc_mode:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @aid: Annunciator ID.
 * @mode: Anninciator mode to set.
 *
 * Sets an annunciator's current operating mode.
 * 
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_ANNUNICATOR.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL; @mode invalid.
 **/
SaErrorT snmp_bc_set_annunc_mode(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiAnnunciatorNumT aid,
				 SaHpiAnnunciatorModeT mode)
{

        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
	
	if (!hnd || oh_lookup_annunciatormode(mode) == NULL) {
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
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_ANNUNCIATOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	} 
	

	err("Annunciators not supported by platform");
	snmp_bc_unlock_handler(custom_handle);
	return(SA_ERR_HPI_INTERNAL_ERROR);
}



void * oh_get_next_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                             SaHpiSeverityT, SaHpiBoolT, SaHpiAnnouncementT)
                __attribute__ ((weak, alias("snmp_bc_get_next_announce")));

void * oh_get_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                        SaHpiEntryIdT, SaHpiAnnouncementT *)
                __attribute__ ((weak, alias("snmp_bc_get_announce")));

void * oh_ack_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                        SaHpiEntryIdT, SaHpiSeverityT)
                __attribute__ ((weak, alias("snmp_bc_ack_announce")));


void * oh_add_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                        SaHpiAnnouncementT *)
                __attribute__ ((weak, alias("snmp_bc_add_announce")));

void * oh_del_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                       SaHpiEntryIdT, SaHpiSeverityT)
                __attribute__ ((weak, alias("snmp_bc_del_announce")));

void * oh_get_annunc_mode (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                           SaHpiAnnunciatorModeT *)
                __attribute__ ((weak, alias("snmp_bc_get_annunc_mode")));

void * oh_set_annunc_mode (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                           SaHpiAnnunciatorModeT)
                __attribute__ ((weak, alias("snmp_bc_set_annunc_mode")));


