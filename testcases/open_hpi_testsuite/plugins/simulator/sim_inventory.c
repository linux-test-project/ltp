/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005, 2006
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
 *	Renier Morales <renier@openhpi.org>
 *
 */

#include <sim_init.h>


static SaErrorT new_inventory(struct oh_handler_state *state,
                              struct oh_event *e,
                              struct sim_inventory *myinv) {
        SaHpiRdrT *rdr;
        struct sim_inventory_info *info;
	SaErrorT error = SA_OK;

        rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
        // Copy information from rdr array to res_rdr
        rdr->RdrType = SAHPI_INVENTORY_RDR;
        memcpy(&rdr->RdrTypeUnion.InventoryRec, &myinv->invrec,
               sizeof(SaHpiInventoryRecT));

        oh_init_textbuffer(&rdr->IdString);
        oh_append_textbuffer(&rdr->IdString, myinv->comment);
        rdr->RecordId =
                oh_get_rdr_uid(SAHPI_INVENTORY_RDR, rdr->RdrTypeUnion.InventoryRec.IdrId);


	rdr->Entity = e->resource.ResourceEntity;

        //set up our private data
        info = (struct sim_inventory_info *)g_malloc(sizeof(struct sim_inventory_info));
        memcpy(info, &myinv->info, sizeof(struct sim_inventory_info));

        // everything ready so add the rdr and extra info to the rptcache
	error = sim_inject_rdr(state, e, rdr, info);
        if (error) {
                g_free(rdr);
                g_free(info);
        }

        return error;
}


SaErrorT sim_discover_chassis_inventory(struct oh_handler_state *state,
                                        struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_chassis_inventory[i].invrec.IdrId != 0) {
                rc = new_inventory(state, e, &sim_chassis_inventory[i]);
                if (rc) {
                        err("Error %d returned when adding chassis inventory", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d chassis inventory injected", j, i);

        return 0;
}


SaErrorT sim_discover_cpu_inventory(struct oh_handler_state *state,
                                    struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_cpu_inventory[i].invrec.IdrId != 0) {
                rc = new_inventory(state, e, &sim_cpu_inventory[i]);
                if (rc) {
                        err("Error %d returned when adding cpu inventory", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d cpu inventory injected", j, i);

        return 0;
}


SaErrorT sim_discover_dasd_inventory(struct oh_handler_state *state,
                                     struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_dasd_inventory[i].invrec.IdrId != 0) {
                rc = new_inventory(state, e, &sim_dasd_inventory[i]);
                if (rc) {
                        err("Error %d returned when adding dasd inventory", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d dasd inventory injected", j, i);

        return 0;
}


SaErrorT sim_discover_hs_dasd_inventory(struct oh_handler_state *state,
                                        struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_hs_dasd_inventory[i].invrec.IdrId != 0) {
                rc = new_inventory(state, e, &sim_hs_dasd_inventory[i]);
                if (rc) {
                        err("Error %d returned when adding hs dasd inventory", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d hs dasd inventory injected", j, i);

        return 0;
}


SaErrorT sim_discover_fan_inventory(struct oh_handler_state *state,
                                    struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_fan_inventory[i].invrec.IdrId != 0) {
                rc = new_inventory(state, e, &sim_fan_inventory[i]);
                if (rc) {
                        err("Error %d returned when adding fan inventory", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d fan inventory injected", j, i);

        return 0;
}

SaErrorT sim_get_idr_info(void *hnd,
                          SaHpiResourceIdT        rid,
                          SaHpiIdrIdT             IdrId,
                          SaHpiIdrInfoT          *IdrInfo)
{
        struct sim_inventory_info *info;

        if (!hnd || !IdrInfo) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

        /* Check if resource exists and has inventory capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Find inventory and its data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(state->rptcache, rid, SAHPI_INVENTORY_RDR, IdrId);
        if (rdr == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        info = (struct sim_inventory_info *)oh_get_rdr_data(state->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* return the data */
        memcpy(IdrInfo, &info->idrinfo, sizeof(SaHpiIdrInfoT));
        return SA_OK;
}


SaErrorT sim_get_idr_area_header(void *hnd,
                                 SaHpiResourceIdT         rid,
                                 SaHpiIdrIdT              IdrId,
                                 SaHpiIdrAreaTypeT        AreaType,
                                 SaHpiEntryIdT            AreaId,
                                 SaHpiEntryIdT           *NextAreaId,
                                 SaHpiIdrAreaHeaderT     *Header)
{
        struct sim_inventory_info *info;
        int i;
        int found = SAHPI_FALSE;

        if (!hnd || !NextAreaId || !Header) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

        /* Check if resource exists and has inventory capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Find inventory and its data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(state->rptcache, rid, SAHPI_INVENTORY_RDR, IdrId);
        if (rdr == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        info = (struct sim_inventory_info *)oh_get_rdr_data(state->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* find the corresponding area */
        if (info->idrinfo.NumAreas == 0) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        if (AreaId == SAHPI_FIRST_ENTRY) {
                for (i = 0; i < SIM_INVENTORY_AREAS && i < info->idrinfo.NumAreas; i++) {
                        if (AreaType == SAHPI_IDR_AREATYPE_UNSPECIFIED ||
                            AreaType == info->area[i].idrareahead.Type) {
                                /* found the next entry */
                                if (found == TRUE) {
                                        *NextAreaId = info->area[i].idrareahead.AreaId;
                                        break;
                                }
                                /* found what we are looking for */
                                memcpy(Header, &info->area[i].idrareahead,
                                       sizeof(SaHpiIdrAreaHeaderT));
                                found = SAHPI_TRUE;
                                *NextAreaId = SAHPI_LAST_ENTRY;
                        }
                }
        }
        else for (i = 0; i < SIM_INVENTORY_AREAS && i < info->idrinfo.NumAreas; i++) {
                if (AreaType == SAHPI_IDR_AREATYPE_UNSPECIFIED ||
                    AreaType == info->area[i].idrareahead.Type) {
                        /* found the next entry */
                        if (found == TRUE) {
                                *NextAreaId = info->area[i].idrareahead.AreaId;
                                break;
                        }
                        if (AreaId != info->area[i].idrareahead.AreaId) {
                                continue;
                        }
                        /* found what we are looking for */
                        memcpy(Header, &info->area[i].idrareahead,
                               sizeof(SaHpiIdrAreaHeaderT));
                        found = SAHPI_TRUE;
                        *NextAreaId = SAHPI_LAST_ENTRY;
                }
        }
        if (found == SAHPI_TRUE) {
                return SA_OK;
        }

        return SA_ERR_HPI_NOT_PRESENT;
}


SaErrorT sim_add_idr_area(void *hnd,
                          SaHpiResourceIdT         rid,
                          SaHpiIdrIdT              IdrId,
                          SaHpiIdrAreaTypeT        AreaType,
                          SaHpiEntryIdT           *AreaId)

{
        struct sim_inventory_info *info;

        if (!hnd || !AreaId) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        
        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

        /* Check if resource exists and has inventory capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Find inventory and its data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(state->rptcache, rid, SAHPI_INVENTORY_RDR, IdrId);
        if (rdr == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        
        info = (struct sim_inventory_info *)oh_get_rdr_data(state->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* check to see if space is available for the new area */
        if (info->idrinfo.NumAreas == SIM_INVENTORY_AREAS) {
                return SA_ERR_HPI_OUT_OF_SPACE;
        } else if (info->idrinfo.ReadOnly) {
        	return SA_ERR_HPI_READ_ONLY;
        }

        /* add the area */
        info->area[info->idrinfo.NumAreas].idrareahead.AreaId = info->nextareaid;
        info->area[info->idrinfo.NumAreas].idrareahead.Type = AreaType;
        info->area[info->idrinfo.NumAreas].idrareahead.ReadOnly = SAHPI_FALSE;
        info->area[info->idrinfo.NumAreas].idrareahead.NumFields = 0;

        /* increment our counters and set return info */
        info->idrinfo.NumAreas++;
        *AreaId = info->nextareaid;
        info->nextareaid++;

        return SA_OK;
}


SaErrorT sim_del_idr_area(void *hnd,
                          SaHpiResourceIdT       rid,
                          SaHpiIdrIdT            IdrId,
                          SaHpiEntryIdT          AreaId)
{
        struct sim_inventory_info *info;
        int i;
        int found = SAHPI_FALSE;

        if (!hnd) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

        /* Check if resource exists and has inventory capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                return SA_ERR_HPI_CAPABILITY ;
        }

        /* Find inventory and its data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(state->rptcache, rid, SAHPI_INVENTORY_RDR, IdrId);
        if (rdr == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        info = (struct sim_inventory_info *)oh_get_rdr_data(state->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* find the entry to delete */
        for (i = 0; i < info->idrinfo.NumAreas; i++) {
                if (AreaId != info->area[i].idrareahead.AreaId) {
                        continue;
                }
                /* found what we are looking for */
                found = SAHPI_TRUE;
                break;
        }
        if (found == SAHPI_FALSE) {
                err("Went through the list and could not find it");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* can we delete the area? */
        if (info->area[i].idrareahead.ReadOnly == SAHPI_TRUE) {
                return SA_ERR_HPI_READ_ONLY;
        }

        /* delete the area entry by moving the remaining array members up */
        if (i < info->idrinfo.NumAreas - 2) {
                for (i++; i < info->idrinfo.NumAreas; i++) {
                        memcpy(&info->area[i - 1], &info->area[i],
                               sizeof(struct sim_idr_area));
                }
        }
        info->idrinfo.NumAreas--;

        return SA_OK;
}


SaErrorT sim_get_idr_field(void *hnd,
                           SaHpiResourceIdT       rid,
                           SaHpiIdrIdT            IdrId,
                           SaHpiEntryIdT          AreaId,
                           SaHpiIdrFieldTypeT     FieldType,
                           SaHpiEntryIdT          FieldId,
                           SaHpiEntryIdT          *NextFieldId,
                           SaHpiIdrFieldT         *Field)
{
        struct sim_inventory_info *info;
        int i, j;
        int found = SAHPI_FALSE;

        if (!hnd || !NextFieldId || !Field) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

        /* Check if resource exists and has inventory capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Find inventory and its data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(state->rptcache, rid, SAHPI_INVENTORY_RDR, IdrId);
        if (rdr == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        info = (struct sim_inventory_info *)oh_get_rdr_data(state->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* find the corresponding area */
        if (info->idrinfo.NumAreas == 0) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        for (i = 0; i < info->idrinfo.NumAreas; i++) {
                if (AreaId != info->area[i].idrareahead.AreaId) {
                        continue;
                }
                /* found the area we are looking for */
                found = SAHPI_TRUE;
                break;
        }
        if (found == SAHPI_FALSE) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* find the corresponding field */
        found = SAHPI_FALSE;
        if (info->area[i].idrareahead.NumFields == 0) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        for (j = 0; j < info->area[i].idrareahead.NumFields; j++) {

                if ( ((info->area[i].field[j].FieldId == FieldId) ||
                        (SAHPI_FIRST_ENTRY == FieldId))
                       && ((info->area[i].field[j].Type == FieldType) ||
                           (SAHPI_IDR_FIELDTYPE_UNSPECIFIED == FieldType)) )
                {

                        memcpy(Field, &info->area[i].field[j],
                               sizeof(SaHpiIdrFieldT));
                        found = SAHPI_TRUE;
                        *NextFieldId = SAHPI_LAST_ENTRY;
                        break;
                }
        }

        j++;

        if (found) {
                if (j < info->area[i].idrareahead.NumFields) {
                        do {
                                if ((info->area[i].field[j].Type == FieldType) ||
                                                (SAHPI_IDR_FIELDTYPE_UNSPECIFIED == FieldType))
                                {
                                        *NextFieldId = info->area[i].field[j].FieldId;
                                        break;
                                }
                                j++;

                        } while (j < info->area[i].idrareahead.NumFields);
                }

        }








#if 0
                        /* found the next field entry */
                        if (found == TRUE) {
                                *NextFieldId = info->area[i].field[j].FieldId;
                                break;
                        }
                        /* found the field we are looking for */
                        memcpy(Field, &info->area[i].field[j],
                               sizeof(SaHpiIdrFieldT));
                        found = SAHPI_TRUE;
                        *NextFieldId = SAHPI_LAST_ENTRY;
                }
        }

#endif
        if (found == SAHPI_FALSE) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        return SA_OK;
}


SaErrorT sim_add_idr_field(void *hnd,
                           SaHpiResourceIdT         rid,
                           SaHpiIdrIdT              IdrId,
                           SaHpiIdrFieldT           *Field)
{
        struct sim_inventory_info *info;
        int i;
        int found = SAHPI_FALSE;

        if (!hnd || !Field) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

        /* Check if resource exists and has inventory capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Find inventory and its data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(state->rptcache, rid, SAHPI_INVENTORY_RDR, IdrId);
        if (rdr == NULL) {
        	err("Inventory RDR %d for resource %d was not found",
        	    IdrId, rid);
                return SA_ERR_HPI_NOT_PRESENT;
        }
        
        info = (struct sim_inventory_info *)oh_get_rdr_data(state->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }
        
        if (info->idrinfo.ReadOnly) {
        	return SA_ERR_HPI_READ_ONLY;
        }

        /* find the corresponding area */
        if (info->idrinfo.NumAreas == 0) {
        	err("No areas in the specified IDR");
                return SA_ERR_HPI_NOT_PRESENT;
        }
        
        for (i = 0; i < info->idrinfo.NumAreas; i++) {
                if (Field->AreaId == info->area[i].idrareahead.AreaId) {
                	/* found the area we are looking for */
                	found = SAHPI_TRUE;
                        break;
                }
        }
        
        if (found == SAHPI_FALSE) {
        	err("Specified area was not found in IDR");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* can we add a new field? */
        if (info->area[i].idrareahead.ReadOnly == SAHPI_TRUE) {
                return SA_ERR_HPI_READ_ONLY;
        }

        /* try to add the new field */
        if (info->area[i].idrareahead.NumFields == SIM_INVENTORY_FIELDS) {
                return SA_ERR_HPI_OUT_OF_SPACE;
        }
        
        memcpy(&info->area[i].field[info->area[i].idrareahead.NumFields],
               Field, sizeof(SaHpiIdrFieldT));
        info->area[i].field[info->area[i].idrareahead.NumFields].AreaId = info->area[i].idrareahead.AreaId;
        info->area[i].field[info->area[i].idrareahead.NumFields].FieldId = info->area[i].nextfieldid;
        Field->FieldId = info->area[i].nextfieldid;
        info->area[i].nextfieldid++;
        info->area[i].field[info->area[i].idrareahead.NumFields].ReadOnly = SAHPI_FALSE;
        info->area[i].idrareahead.NumFields++;

        return SA_OK;
}


SaErrorT sim_set_idr_field(void *hnd,
                           SaHpiResourceIdT         rid,
                           SaHpiIdrIdT              IdrId,
                           SaHpiIdrFieldT           *Field)
{
        struct sim_inventory_info *info;
        int i, j;
        int found = SAHPI_FALSE;
        char * type;

        if (!hnd || !Field) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        type = oh_lookup_idrfieldtype(Field->Type);
        if (type == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        if (strcmp(type, "UNSPECIFIED") == 0) {
                return SA_ERR_HPI_INVALID_DATA;
        }

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

        /* Check if resource exists and has inventory capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Find inventory and its data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(state->rptcache, rid, SAHPI_INVENTORY_RDR, IdrId);
        if (rdr == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        info = (struct sim_inventory_info *)oh_get_rdr_data(state->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* find the corresponding area */
        if (info->idrinfo.NumAreas == 0) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        for (i = 0; i < info->idrinfo.NumAreas; i++) {
                if (Field->AreaId != info->area[i].idrareahead.AreaId) {
                        continue;
                }
                /* found the area we are looking for */
                found = SAHPI_TRUE;
                break;
        }
        if (found == SAHPI_FALSE) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* can we set a field? */
        if (info->area[i].idrareahead.ReadOnly == SAHPI_TRUE) {
                return SA_ERR_HPI_READ_ONLY;
        }

        /* find the corresponding field */
        if (info->area[i].idrareahead.NumFields == 0) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        for (j = 0; j < info->area[i].idrareahead.NumFields; j++) {
                if (Field->FieldId == info->area[i].field[j].FieldId) {
                        /* found the field we are looking for */
                        if (info->area[i].field[j].ReadOnly == SAHPI_TRUE) {
                                return SA_ERR_HPI_READ_ONLY;
                        }
                        info->area[i].field[j].Type = Field->Type;
                        memcpy(&info->area[i].field[j].Field, &Field->Field,
                               sizeof(SaHpiTextBufferT));
                        return SA_OK;
                }
        }

        return SA_ERR_HPI_NOT_PRESENT;
}


SaErrorT sim_del_idr_field(void *hnd,
                           SaHpiResourceIdT         rid,
                           SaHpiIdrIdT              IdrId,
                           SaHpiEntryIdT            AreaId,
                           SaHpiEntryIdT            FieldId)
{
        struct sim_inventory_info *info;
        int i, j;
        int found = SAHPI_FALSE;

        if (!hnd) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        struct oh_handler_state *state = (struct oh_handler_state *)hnd;

        /* Check if resource exists and has inventory capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(state->rptcache, rid);
        if (!rpt) {
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                return SA_ERR_HPI_CAPABILITY;
        }

        /* Find inventory and its data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(state->rptcache, rid, SAHPI_INVENTORY_RDR, IdrId);
        if (rdr == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        info = (struct sim_inventory_info *)oh_get_rdr_data(state->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* find the corresponding area */
        if (info->idrinfo.NumAreas == 0) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        for (i = 0; i < info->idrinfo.NumAreas; i++) {
                if (AreaId != info->area[i].idrareahead.AreaId) {
                        continue;
                }
                /* found the area we are looking for */
                found = SAHPI_TRUE;
                break;
        }
        if (found == SAHPI_FALSE) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* can we delete a field? */
        if (info->area[i].idrareahead.ReadOnly == SAHPI_TRUE) {
                return SA_ERR_HPI_READ_ONLY;
        }

        /* find the corresponding field */
        if (info->area[i].idrareahead.NumFields == 0) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        found = SAHPI_FALSE;
        for (j = 0; j < info->area[i].idrareahead.NumFields; j++) {
                if (FieldId == info->area[i].field[j].FieldId) {
                        /* found the field we are looking for */
                        found = SAHPI_TRUE;
                        break;
                }
        }
        if (found == SAHPI_FALSE) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* can we delete the field? */
        if (info->area[i].field[j].ReadOnly == SAHPI_TRUE) {
                return SA_ERR_HPI_READ_ONLY;
        }

        /* delete the area entry by moving the remaining array members up */
        if (j < info->area[i].idrareahead.NumFields - 2) {
                for (j++; j < SIM_INVENTORY_AREAS && j < info->area[i].idrareahead.NumFields; j++) {
                        memcpy(&info->area[i].field[j - 1], &info->area[i].field[j],
                               sizeof(SaHpiIdrFieldT));
                }
        }
        info->area[i].idrareahead.NumFields--;

        return SA_OK;
}


void * oh_get_idr_info (void *hnd, SaHpiResourceIdT, SaHpiIdrIdT,SaHpiIdrInfoT)
                __attribute__ ((weak, alias("sim_get_idr_info")));

void * oh_get_idr_area_header (void *, SaHpiResourceIdT, SaHpiIdrIdT,
                                SaHpiIdrAreaTypeT, SaHpiEntryIdT, SaHpiEntryIdT,
                                SaHpiIdrAreaHeaderT)
                __attribute__ ((weak, alias("sim_get_idr_area_header")));

void * oh_add_idr_area (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrAreaTypeT,
                        SaHpiEntryIdT)
                __attribute__ ((weak, alias("sim_add_idr_area")));

void * oh_del_idr_area (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT)
                __attribute__ ((weak, alias("sim_del_idr_area")));

void * oh_get_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT,
                         SaHpiIdrFieldTypeT, SaHpiEntryIdT, SaHpiEntryIdT,
                         SaHpiIdrFieldT)
                __attribute__ ((weak, alias("sim_get_idr_field")));

void * oh_add_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrFieldT)
                __attribute__ ((weak, alias("sim_add_idr_field")));

void * oh_set_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrFieldT)
                __attribute__ ((weak, alias("sim_set_idr_field")));

void * oh_del_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT,
                         SaHpiEntryIdT)
                __attribute__ ((weak, alias("sim_del_idr_field")));


