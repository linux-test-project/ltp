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
 *      W. David Ashley <dashley@us.ibm.com>
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <snmp_bc_plugin.h>

SaErrorT snmp_bc_discover_rsa(struct oh_handler_state *handle,
			      SaHpiEntityPathT *ep_root)
{
	
	int i;
	SaErrorT err;
        struct oh_event *e;
	struct snmp_value get_value;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;

	if (!handle || !ep_root) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

        /******************
	 * Discover chassis
         ******************/
 	err = snmp_bc_snmp_get(custom_handle, SNMP_BC_PLATFORM_OID_RSA, &get_value, SAHPI_TRUE);
        if (err || get_value.type != ASN_INTEGER) {
		err("Cannot get OID=%s; Received Type=%d; Error=%s.",
		     SNMP_BC_PLATFORM_OID_RSA, get_value.type, oh_lookup_error(err));
		if (err) { return(err); }
		else { return(SA_ERR_HPI_INTERNAL_ERROR); }
        }

	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	e->resource = snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_CHASSIS].rpt;

	e->resource.ResourceEntity = *ep_root;
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_CHASSIS].comment,
				   ep_root->Entry[0].EntityLocation);

	dbg("Discovered resource=%s.", e->resource.ResourceTag.Data);

	/* Create platform-specific info space to add to infra-structure */
	res_info_ptr = g_memdup(&(snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_CHASSIS].res_info),
				sizeof(struct ResourceInfo));
	if (!res_info_ptr) {
		err("Out of memory.");
		g_free(e);
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to temporary event cache/queue */
	err = oh_add_resource(handle->rptcache, 
			      &(e->resource), 
			      res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		g_free(e);
		return(err);
	}

	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	snmp_bc_discover_sensors(handle, snmp_bc_chassis_sensors_rsa, e);
 	snmp_bc_discover_controls(handle, snmp_bc_chassis_controls_rsa, e);
	snmp_bc_discover_inventories(handle, snmp_bc_chassis_inventories_rsa, e);

	/* ---------------------------------------- */
	/* Construct .event of struct oh_event      */	
	/* ---------------------------------------- */
	snmp_bc_set_resource_add_oh_event(e, res_info_ptr);		

	e->hid = handle->hid;
        oh_evt_queue_push(handle->eventq, e);
	
        /***************
	 * Discover CPUs
         ***************/
        for (i=0; i<RSA_MAX_CPU; i++) {
		e = snmp_bc_alloc_oh_event();
		if (e == NULL) {
			err("Out of memory.");
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}
		
		e->resource = snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_CPU].rpt;
		
		oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
		oh_set_ep_location(&(e->resource.ResourceEntity),
				   SAHPI_ENT_PROCESSOR, i + SNMP_BC_HPI_LOCATION_BASE);

		/* See if CPU exists */
		if (!rdr_exists(custom_handle, &(e->resource.ResourceEntity), 0,
				SNMP_BC_CPU_OID_RSA, 0, 0 )) {
			snmp_bc_free_oh_event(e);
			continue;
		}

		e->resource.ResourceId = 
			oh_uid_from_entity_path(&(e->resource.ResourceEntity));
		snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
					   snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_CPU].comment,
					   i + SNMP_BC_HPI_LOCATION_BASE);
		
		dbg("Discovered resource=%s.", e->resource.ResourceTag.Data);
		
		/* Create platform-specific info space to add to infra-structure */
		res_info_ptr = g_memdup(&(snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_CPU].res_info),
					sizeof(struct ResourceInfo));
		if (!res_info_ptr) {
			err("Out of memory.");
			snmp_bc_free_oh_event(e);
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}

		res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;
		
		/* Get UUID and convert to GUID */
                err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);
		
		/* Add resource to temporary event cache/queue */
		err = oh_add_resource(handle->rptcache,
				      &(e->resource), 
				      res_info_ptr, 0);
		if (err) {
			err("Failed to add resource. Error=%s.", oh_lookup_error(err));
			snmp_bc_free_oh_event(e);
			return(err);
		}

		
		/* Find resource's events, sensors, controls, etc. */
		snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
		snmp_bc_discover_sensors(handle, snmp_bc_cpu_sensors_rsa, e);
		snmp_bc_discover_controls(handle, snmp_bc_cpu_controls_rsa, e);
		snmp_bc_discover_inventories(handle, snmp_bc_cpu_inventories_rsa, e);

		/* ---------------------------------------- */
		/* Construct .event of struct oh_event      */	
		/* ---------------------------------------- */
		snmp_bc_set_resource_add_oh_event(e, res_info_ptr);
				
		e->hid = handle->hid;
                oh_evt_queue_push(handle->eventq, e);
	}

        /****************
	 * Discover DASDs
         ****************/
        for (i=0; i<RSA_MAX_DASD; i++) {
		e = snmp_bc_alloc_oh_event();
		if (e == NULL) {
			err("Out of memory.");
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}
		
		e->resource = snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_DASD].rpt;
		
		oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
		oh_set_ep_location(&(e->resource.ResourceEntity),
				   SAHPI_ENT_DISK_BAY, i + SNMP_BC_HPI_LOCATION_BASE);

		/* See if DASD exists */
		if (!rdr_exists(custom_handle, &(e->resource.ResourceEntity), 0,
				SNMP_BC_DASD_OID_RSA, 0, 0 )) {
			snmp_bc_free_oh_event(e);
			continue;
		}

		e->resource.ResourceId = 
			oh_uid_from_entity_path(&(e->resource.ResourceEntity));
		snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
					   snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_DASD].comment,
					   i + SNMP_BC_HPI_LOCATION_BASE);
		
		dbg("Discovered resource=%s.", e->resource.ResourceTag.Data);
		
		/* Create platform-specific info space to add to infra-structure */
		res_info_ptr = g_memdup(&(snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_DASD].res_info),
					sizeof(struct ResourceInfo));
		if (!res_info_ptr) {
			err("Out of memory.");
			snmp_bc_free_oh_event(e);
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}

		res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;
		
		/* Get UUID and convert to GUID */
                err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);
		
		/* Add resource to temporary event cache/queue */
		err = oh_add_resource(handle->rptcache,
				      &(e->resource), 
				      res_info_ptr, 0);
		if (err) {
			err("Failed to add resource. Error=%s.", oh_lookup_error(err));
			snmp_bc_free_oh_event(e);
			return(err);
		}
		
		/* Find resource's events, sensors, controls, etc. */
		snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
		snmp_bc_discover_sensors(handle, snmp_bc_dasd_sensors_rsa, e);
		snmp_bc_discover_controls(handle, snmp_bc_dasd_controls_rsa, e);
		snmp_bc_discover_inventories(handle, snmp_bc_dasd_inventories_rsa, e);
		
		/* ---------------------------------------- */
		/* Construct .event of struct oh_event      */	
		/* ---------------------------------------- */
		snmp_bc_set_resource_add_oh_event(e, res_info_ptr);
		
		e->hid = handle->hid;
                oh_evt_queue_push(handle->eventq, e);
	}

        /***************
	 * Discover Fans
         ***************/
        for (i=0; i<RSA_MAX_FAN; i++) {
		e = snmp_bc_alloc_oh_event();
		if (e == NULL) {
			err("Out of memory.");
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}
		
		e->resource = snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_FAN].rpt;
		
		oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
		oh_set_ep_location(&(e->resource.ResourceEntity),
				   SAHPI_ENT_FAN, i + SNMP_BC_HPI_LOCATION_BASE);

		/* See if fan exists */
		if (!rdr_exists(custom_handle, &(e->resource.ResourceEntity), 0,
				SNMP_BC_FAN_OID_RSA, 0, 0 )) {
			snmp_bc_free_oh_event(e);
			continue;
		}

		e->resource.ResourceId = 
			oh_uid_from_entity_path(&(e->resource.ResourceEntity));
		snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
					   snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_FAN].comment,
					   i + SNMP_BC_HPI_LOCATION_BASE);
		
		dbg("Discovered resource=%s.", e->resource.ResourceTag.Data);
		
		/* Create platform-specific info space to add to infra-structure */
		res_info_ptr = g_memdup(&(snmp_bc_rpt_array_rsa[RSA_RPT_ENTRY_FAN].res_info),
					sizeof(struct ResourceInfo));
		if (!res_info_ptr) {
			err("Out of memory.");
			snmp_bc_free_oh_event(e);
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}

		res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;
		
		/* Get UUID and convert to GUID */
                err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);
		
		/* Add resource to temporary event cache/queue */
		err = oh_add_resource(handle->rptcache,
				      &(e->resource), 
				      res_info_ptr, 0);
		if (err) {
			err("Failed to add resource. Error=%s.", oh_lookup_error(err));
			snmp_bc_free_oh_event(e);
			return(err);
		}
		
		/* Find resource's events, sensors, controls, etc. */
		snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
		snmp_bc_discover_sensors(handle, snmp_bc_fan_sensors_rsa, e);
		snmp_bc_discover_controls(handle, snmp_bc_fan_controls_rsa, e);
		snmp_bc_discover_inventories(handle, snmp_bc_fan_inventories_rsa, e);

		/* ---------------------------------------- */
		/* Construct .event of struct oh_event      */	
		/* ---------------------------------------- */
		snmp_bc_set_resource_add_oh_event(e, res_info_ptr);
				
		e->hid = handle->hid;
                oh_evt_queue_push(handle->eventq, e);
	}

  return(SA_OK);
}
