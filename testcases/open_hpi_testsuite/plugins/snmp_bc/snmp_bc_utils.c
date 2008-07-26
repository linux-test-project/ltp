/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Chris Chia <cchia@users.sf.net>
 */

#include <uuid/uuid.h>
#include <snmp_bc_plugin.h>
#include <snmp_bc_utils.h>

SaErrorT snmp_bc_get_guid(struct snmp_bc_hnd *custom_handle,
			  struct oh_event *e,
			  struct ResourceInfo *res_info_ptr)
{
	SaErrorT status;
	gchar  *UUID;
	gchar  *BC_UUID;
        gchar  **tmpstr;
	SaHpiGuidT guid;
        struct  snmp_value get_value;
        const   gchar  *UUID_delimiter1;
        const   gchar  *UUID_delimiter2;
        const   gchar  *UUID_delimiter;
        const   gchar  *NA;    /* not case sensitive */
        guint   UUID_cnt, i;
        uuid_t  UUID_val;
	
	UUID_delimiter1 = " ";
	UUID_delimiter2 = "-";
	UUID_delimiter  = "-";
	NA   = "NOT AVAILABLE";
	UUID_cnt = 0;
	i = 0;	
	UUID = NULL;
	BC_UUID = NULL;
	tmpstr = NULL;

        if ( (custom_handle == NULL) || (e == NULL) || (res_info_ptr == NULL)) {
                err("Invalid parameter.");
                status = SA_ERR_HPI_INVALID_PARAMS;
                goto CLEANUP2;
        }
	

	
	memset(&guid, 0, sizeof(SaHpiGuidT));  /* default to zero */
	if (res_info_ptr->mib.OidUuid == NULL) {
		dbg("NULL UUID OID");
		status = SA_OK;
		goto CLEANUP;
	}
        status = snmp_bc_oid_snmp_get(custom_handle, 
				      &(e->resource.ResourceEntity), 0,
				      res_info_ptr->mib.OidUuid,            
				      &get_value, SAHPI_TRUE);
        if(( status != SA_OK) || (get_value.type != ASN_OCTET_STR)) {
                dbg("Cannot get OID rc=%d; oid=%s type=%d.", 
                    status, res_info_ptr->mib.OidUuid, get_value.type);
                if ( status != SA_ERR_HPI_BUSY)  status = SA_ERR_HPI_NO_RESPONSE;
                goto CLEANUP;
        }

        dbg("UUID=%s.", get_value.string);
        /* rid lead+trail blanks */
        BC_UUID = g_strstrip(g_strdup(get_value.string));
        if (BC_UUID == NULL || BC_UUID[0] == '\0') {
                err("UUID is NULL.");
                status = SA_ERR_HPI_ERROR;
                goto CLEANUP;
        }
        if (g_ascii_strcasecmp( BC_UUID, NA ) == 0) {
                dbg("UUID is N/A %s, set GUID to zeros.", BC_UUID);
                for ( i=0; i<16; i++ ) UUID_val[i] = 0;
                memmove ( guid, &UUID_val, sizeof(uuid_t));
                status = SA_OK;
                goto CLEANUP;
        }
        /* separate substrings */
        tmpstr = g_strsplit(BC_UUID, UUID_delimiter1, -1);
        for ( UUID_cnt=0; tmpstr[UUID_cnt] != NULL; UUID_cnt++ );
        /* dbg("number of UUID substrings = %d, strings =", UUID_cnt); */
        /* for (i=0; i<UUID_cnt; i++) dbg(" %s", tmpstr[i]); dbg("\n"); */
        if ( UUID_cnt == 0 ) {
                err("Zero length UUID string.");
                status = SA_ERR_HPI_ERROR;
                goto CLEANUP;
        }
        if ( UUID_cnt == 1 ) { /* check with second possible substring delimiter */
                tmpstr = g_strsplit(BC_UUID, UUID_delimiter2, -1);
                for ( UUID_cnt=0; ; UUID_cnt++ ) {
                        if ( tmpstr[UUID_cnt] == NULL ) break;
                }
                /* dbg("Number of UUID substrings = %d, strings =", UUID_cnt); */
                /* for (i=0; i<UUID_cnt; i++) dbg(" %s", tmpstr[i]); dbg("\n"); */
                if ( UUID_cnt == 0 ) {
                        err("Zero length UUID string.");
                        status = SA_ERR_HPI_ERROR;
                        goto CLEANUP;
                }
        }
        if ( UUID_cnt == UUID_SUBSTRINGS_CNT1 ) {
                /* UUID has 8 four character strings 4-4-4-4-4-4-4-4
                 * convert to industry standard UUID 8-4-4-4-12 string */
                UUID = g_strconcat( tmpstr[0], tmpstr[1], UUID_delimiter,   
                                    tmpstr[2], UUID_delimiter,             
                                    tmpstr[3], UUID_delimiter,            
                                    tmpstr[4], UUID_delimiter,           
                                    tmpstr[5], tmpstr[6], tmpstr[7], NULL );
                if (UUID == NULL) {
                        err("Bad UUID string.");
                        status = SA_ERR_HPI_ERROR;
                        goto CLEANUP;
                }
                dbg("UUID string %s", UUID);
                /* convert UUID string to numeric UUID value */
                if ( (status = uuid_parse(UUID, UUID_val)) ) {
                        err("Cannot parse UUID string err=%d.", status);
                        status = SA_ERR_HPI_ERROR;
                        goto CLEANUP;
                }       
                /* dbg("GUID value  "); */
                /* for (i=0; i<16; i++) { dbg("%02x", UUID_val[i]);} dbg("\n"); */
                memmove ( guid, &UUID_val, sizeof(uuid_t));
                status = SA_OK;
        }
        else if ( UUID_cnt == UUID_SUBSTRINGS_CNT2 ) {
                /* Got a 5 substring case, just put in the delimiter */
                UUID = g_strconcat( tmpstr[0], UUID_delimiter,   
                                    tmpstr[1], UUID_delimiter,             
                                    tmpstr[2], UUID_delimiter,            
                                    tmpstr[3], UUID_delimiter,           
                                    tmpstr[4], NULL );
                if (UUID == NULL) {
                        err("Bad UUID string.");
                        status = SA_ERR_HPI_ERROR;
                        goto CLEANUP;
                }
                dbg("UUID=%s", UUID);
                /* Convert UUID string to numeric UUID value */
                if ( (status = uuid_parse(UUID, UUID_val)) ) {
                        err("Cannot parse UUID string. err=%d.", status);
                        status = SA_ERR_HPI_ERROR;
                        goto CLEANUP;
                }       
                /* dbg("GUID value  "); */
                /* for (i=0; i<16; i++) { dbg("%02x", UUID_val[i]);} dbg("\n"); */
                memmove ( guid, &UUID_val, sizeof(uuid_t));
                status = SA_OK;
        }
        else {  /* Non standard case unsupported */
                err("Non standard UUID string.");
                status = SA_ERR_HPI_ERROR;
        }

  CLEANUP:
  	memmove(e->resource.ResourceInfo.Guid, guid, sizeof(SaHpiGuidT));
  CLEANUP2:
        g_free(UUID);
        g_free(BC_UUID);
        g_strfreev(tmpstr);
                                                                                             
        /* dbg("get_guid exit status %d.", status); */
        return(status);
}


/**
 * snmp_bc_discover_resources:
 * @resource_ep: Pointer to full FRU Resource Entity Path.
 * @slot_ep    : Pointer to Slot Entity Path extracted from resource_ep
 *
 * Extract Slot Entity Path portion from the full FRU Entity Path
 *
 * Return values:
 * Slot Entity Path - normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Invalid input pointer or data
 **/
SaErrorT snmp_bc_extract_slot_ep(SaHpiEntityPathT *resource_ep, SaHpiEntityPathT *slot_ep) 
{
	guint i,j;

	if (!resource_ep || !slot_ep) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	for (i = 0; i < SAHPI_MAX_ENTITY_PATH ; i++) {
		if (	(resource_ep->Entry[i].EntityType == SAHPI_ENT_PHYSICAL_SLOT) ||
			(resource_ep->Entry[i].EntityType == BLADECENTER_SWITCH_SLOT) ||
			(resource_ep->Entry[i].EntityType == BLADECENTER_POWER_SUPPLY_SLOT) ||
			(resource_ep->Entry[i].EntityType == BLADECENTER_PERIPHERAL_BAY_SLOT) ||
			(resource_ep->Entry[i].EntityType == BLADECENTER_SYS_MGMNT_MODULE_SLOT) ||
			(resource_ep->Entry[i].EntityType == BLADECENTER_ALARM_PANEL_SLOT) ||
			(resource_ep->Entry[i].EntityType == BLADECENTER_MUX_SLOT) ||
			(resource_ep->Entry[i].EntityType == BLADECENTER_CLOCK_SLOT) ||
			(resource_ep->Entry[i].EntityType == BLADECENTER_BLOWER_SLOT) )
			break;	
	}
	
	/* There must alway be a SAHPI_ENT_ROOT, so xx_SLOT entry index must 
           always be less than SAHPI_MAX_ENTITY_PATH */
	if ( i == SAHPI_MAX_ENTITY_PATH) return(SA_ERR_HPI_INVALID_PARAMS);
	
	for ( j = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
		slot_ep->Entry[j].EntityType = resource_ep->Entry[i].EntityType;
		slot_ep->Entry[j].EntityLocation = resource_ep->Entry[i].EntityLocation;
		
	 	if (resource_ep->Entry[i].EntityType == SAHPI_ENT_ROOT) break;
		j++;
	}
	
	return(SA_OK);
}

/**
 * snmp_bc_copy_oh_event:
 * @new_event: Pointer to new oh_event.
 * @old_event: Pointer to old oh_event.
 *
 * Allocate and create a duplicate copy of old_event
 *
 * Return values:
 **/
SaErrorT snmp_bc_copy_oh_event(struct oh_event *new_event, struct oh_event *old_event)
{

	GSList *node;
	if (!new_event || !old_event) return(SA_ERR_HPI_INVALID_PARAMS);

	node = NULL;
	*new_event = *old_event;
	new_event->rdrs = NULL;
	for (node = old_event->rdrs; node; node = node->next) {
		new_event->rdrs = g_slist_append(new_event->rdrs, g_memdup(node->data,
							   sizeof(SaHpiRdrT)));
	}

	return(SA_OK);
}


/**
 * snmp_bc_alloc_oh_event:
 *
 * Allocate and create a  copy of oh_event with default data
 *
 * Return values:
 * NULL - No space or invalid parm
 * (oh_event *) - Normal
 **/
struct oh_event *snmp_bc_alloc_oh_event()
{
	struct oh_event *e = NULL;

	e = (struct oh_event *)g_malloc0(sizeof(struct oh_event));
	if (e == NULL) return(e); 
	
	e->rdrs = NULL;

	return e;
}

/**
 * snmp_bc_free_oh_event:
 * @event: Pointer to oh_event.
 *
 * Free oh_event space
 *
 * Return values:
 * NULL - No space or invalid parm
 * (oh_event *) - Normal
 **/
void snmp_bc_free_oh_event(struct oh_event *e)
{
	if (!e) return;
	
	g_slist_free(e->rdrs);	
	g_free(e);
	return;
}


/**
 * snmp_bc_set_resource_add_oh_event:
 * @e: Pointer to oh_event.
 * @res_info_ptr
 *
 * Initialize (oh_event *).event to default value for resource_add
 * e->resource must be initialized prior to using this util.
 *
 * Return values:
 * NULL - No space or invalid parm
 * (oh_event *) - Normal
 **/
SaErrorT snmp_bc_set_resource_add_oh_event(struct oh_event *e, 
					struct ResourceInfo *res_info_ptr)
{
	if (!e || !res_info_ptr) return(SA_ERR_HPI_INVALID_PARAMS);
	
	e->event.Severity = e->resource.ResourceSeverity;
	e->event.Source =   e->resource.ResourceId;	
	if (oh_gettimeofday(&e->event.Timestamp) != SA_OK)
		                    e->event.Timestamp = SAHPI_TIME_UNSPECIFIED;

	if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_FRU) {
		e->event.EventType = SAHPI_ET_HOTSWAP;
		e->event.EventDataUnion.HotSwapEvent.HotSwapState = 
			e->event.EventDataUnion.HotSwapEvent.HotSwapState = 
						res_info_ptr->cur_state;
		
	} else {
		e->event.EventType = SAHPI_ET_RESOURCE;
		e->event.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_ADDED;			
	} 				    
	return(SA_OK);
}


/**
 * snmp_bc_extend_ep:
 * @e: Pointer to oh_event.
 * @resource_index: 
 * @interposer_installed_mask: 
 *
 * If there is an interposer installed in this resource slot, 
 * insert interposer into entitypath between physical slot and resource entity.
 * 
 * Currently there are 2 types of interposer cards, Switch (smi) and Management Module (mmi)
 * interposers. 
 *
 * Return values:
 * 
 * SA_OK - Normal
 **/
SaErrorT snmp_bc_extend_ep(struct oh_event *e,
			   guint resource_index, 
			   gchar *interposer_install_mask) 
{
	guint i;

	if (interposer_install_mask[resource_index] == '1') {
        	for (i=0; i<SAHPI_MAX_ENTITY_PATH; i++) {
                	if (e->resource.ResourceEntity.Entry[i].EntityType == SAHPI_ENT_ROOT) break;
       	 	}

		do { 
			e->resource.ResourceEntity.Entry[i+1].EntityType = 
						e->resource.ResourceEntity.Entry[i].EntityType;
			e->resource.ResourceEntity.Entry[i+1].EntityLocation = 
						e->resource.ResourceEntity.Entry[i].EntityLocation;
			i--;
		} while (i > 0);

		/* i == 0 at this point; setting ep Entry[1] */
		e->resource.ResourceEntity.Entry[i+1].EntityType = SAHPI_ENT_INTERCONNECT;
		e->resource.ResourceEntity.Entry[i+1].EntityLocation = SNMP_BC_HPI_LOCATION_BASE + resource_index;

		/* Entry[0] remains untouched */
	}
	
	return(SA_OK);
}


