/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      Renier Morales <renierm@users.sf.net>
 *
 */

#include <stdlib.h>
#include <string.h>
#include <rpt_utils.h>
#include <openhpi.h>

/* declare Rptable object */		     
RPTable *default_rpt = NULL; 


#if 0
/* declare hotswap state list */
 GSList *managed_hs_resources = NULL;
#endif

static RPTEntry *get_rptentry_by_rid(RPTable *table, SaHpiResourceIdT rid)
{
        RPTEntry *rptentry = NULL;
        GSList *node;

        if (!(table)) {
                dbg("ERROR: Cannot work on a null table pointer.");
                return NULL;
        }
        if (!(table->rptable)) {
                /*dbg("Info: RPT is empty.");*/
                return NULL;
        }
        
        if (rid == RPT_ENTRY_BEGIN) {
                rptentry = (RPTEntry *) (table->rptable->data);
        } else {
                for (node = table->rptable; node != NULL; node = node->next) {
                        rptentry = (RPTEntry *) node->data;
                        if (rptentry->rpt_entry.ResourceId == rid) break;
                        else rptentry = NULL;
                }        
        }

        return rptentry;
}

static RDRecord *get_rdrecord_by_id(GSList *records, SaHpiEntryIdT id)
{
        RDRecord *rdrecord = NULL;
        GSList *node;

        if (!records) {
                /*dbg("Info: RDR repository is empty.");*/
                return NULL;                
        }
        
        if (id == RDR_BEGIN) {
                rdrecord = (RDRecord *) records->data;
        } else {
                for (node = records; node != NULL; node = node->next) {
                        rdrecord = (RDRecord *) node->data;
                        if (rdrecord->rdr.RecordId == id) break;
                        else rdrecord = NULL;
                }                
        }        

        return rdrecord;
}

static SaHpiUint8T get_rdr_type_num(SaHpiRdrT *rdr)
{
        SaHpiUint8T num = 0;
        
        switch (rdr->RdrType) {
                case SAHPI_CTRL_RDR:
                        num = rdr->RdrTypeUnion.CtrlRec.Num;
			break;
                case SAHPI_SENSOR_RDR:
                        num = rdr->RdrTypeUnion.SensorRec.Num;
			break;
                case SAHPI_INVENTORY_RDR:
                        num = rdr->RdrTypeUnion.InventoryRec.EirId;
			break;
                case SAHPI_WATCHDOG_RDR:
                        num = rdr->RdrTypeUnion.WatchdogRec.WatchdogNum;
			break;
		default:
                        num = 0;
        }

        return num;
}

static int check_ep(SaHpiEntityPathT ep)
{
        int check = -1; 
	int i;

        for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
                if (ep.Entry[i].EntityType == SAHPI_ENT_ROOT) {
                        check = 0;
                        break;
                }
        }

        return check;
}

static void update_rptable(RPTable *table, guint modifier) {
        struct timeval tv;
        SaHpiTimeT time;

        if (!table) {dbg("ERROR: Cannot work on a null table pointer."); return;}

        gettimeofday(&tv, NULL);
        time = (SaHpiTimeT) tv.tv_sec * 1000000000 + tv.tv_usec * 1000;

        table->rpt_info.UpdateTimestamp = time;

        if (modifier == RPT_INCREMENT) {
                table->rpt_info.UpdateCount = table->rpt_info.UpdateCount + 1;
        } else if (modifier == RPT_DECREMENT) {
                table->rpt_info.UpdateCount = table->rpt_info.UpdateCount - 1;
        }
}

/* Public function. Helps derive the Record id from type and num. */
SaHpiUint32T get_rdr_uid(SaHpiRdrTypeT type, SaHpiUint32T num)
{
        SaHpiUint32T uid;

        uid = ((SaHpiUint32T)type) << 16;
        uid = uid + (SaHpiUint32T)num;

        return uid;
}

/**
 * General RPT calls
 **/
/**
 * oh_flush_rpt: Cleans RPT from all entries and RDRs and frees the memory
 * associated with them.
 * @table: Pointer to the RPT to flush.
 **/
void oh_flush_rpt(RPTable *table)
{
        SaHpiRptEntryT *tmp_entry;

        if (!(table)) {
                dbg("ERROR: Cannot work on a null table pointer.");
                return;
        }

        while ((tmp_entry = oh_get_resource_by_id(table, RPT_ENTRY_BEGIN)) != NULL) {
                oh_remove_resource(table, RPT_ENTRY_BEGIN);
        }
}

/**
 * rpt_diff: Extracts from current the resources and rdrs that are not found
 * in new and puts them in res_gone and rdr_gone. Also, puts in res_new and rdr_new
 * the resources and rdrs that are not already in current Or that are not identical
 * to the ones in current
 * @current: IN. Pointer to RPTable that represents the current state of resources and rdrs.
 * @new: IN. Pointer to RPTable that contains rpt entries and rdrs just recently discovered.
 * @res_new: OUT. List of new or changed rpt entries
 * @rdr_new: OUT. List of new or changed rdrs
 * @res_gone: OUT. List of old and not present resources.
 * @rdr_gone: OUT. List of old and not present rdrs.
 **/
void rpt_diff(RPTable *current, RPTable *new,
              GSList **res_new, GSList **rdr_new,
              GSList **res_gone, GSList **rdr_gone) {

        SaHpiRptEntryT *res = NULL;

        /* Look for absent resources and rdrs */
        for (res = oh_get_resource_by_id(current, RPT_ENTRY_BEGIN);
             res != NULL;
             res = oh_get_resource_next(current, res->ResourceId)) {
                
                SaHpiRptEntryT *tmp_res = oh_get_resource_by_id(new, res->ResourceId);
                
                if (tmp_res == NULL) *res_gone = g_slist_append(*res_gone, (gpointer)res);
                else {
                        SaHpiRdrT *rdr = NULL;
                        
                        for (rdr = oh_get_rdr_by_id(current, res->ResourceId, RDR_BEGIN);
                             rdr != NULL;
                             rdr = oh_get_rdr_next(current, res->ResourceId, rdr->RecordId)) {

                                SaHpiRdrT *tmp_rdr =
                                        oh_get_rdr_by_id(new, res->ResourceId, rdr->RecordId);
                                        
                                if (tmp_rdr == NULL)
                                        *rdr_gone = g_slist_append(*rdr_gone, (gpointer)rdr);
                        }
                }
        }

        /* Look for new resources and rdrs*/
        for (res = oh_get_resource_by_id(new, RPT_ENTRY_BEGIN);
             res != NULL;
             res = oh_get_resource_next(new, res->ResourceId)) {

                SaHpiRptEntryT *tmp_res = oh_get_resource_by_id(current, res->ResourceId);
                SaHpiRdrT *rdr = NULL;
                if (!tmp_res || memcmp(res, tmp_res, sizeof(SaHpiRptEntryT))) {
                        *res_new = g_slist_append(*res_new, (gpointer)res);
                }

                
                        
                for (rdr = oh_get_rdr_by_id(new, res->ResourceId, RDR_BEGIN);
                     rdr != NULL;
                     rdr = oh_get_rdr_next(new, res->ResourceId, rdr->RecordId)) {

                        SaHpiRdrT *tmp_rdr = NULL;
                                
                        if (tmp_res != NULL) 
                                tmp_rdr = oh_get_rdr_by_id(current, res->ResourceId, rdr->RecordId);                        

                        if (tmp_rdr == NULL || memcmp(rdr, tmp_rdr, sizeof(SaHpiRdrT)))
                                *rdr_new = g_slist_append(*rdr_new, (gpointer)rdr);
                
                }
        }
}

/**
 * Resource interface functions
 */

/**
 * oh_add_resource: Add a RPT entry to the RPT along with some private data.
 * If an RPT entry with the same resource id exists int the RPT, it will be
 * overlayed with the new RPT entry. Also, this function will assign the
 * resource id as its entry id since it is expected to be unique for the table.
 * Note - If the RPT entry has a resource id of RPT_ENTRY_BEGIN (0xffffffff),
 * the first RPT entry in the table will be overlayed.
 * @table: Pointer to the RPT to which the RPT entry will be added.
 * @entry: The RPT entry (resource) to be added to the RPT.
 * @data: Pointer to private data for storing along with the RPT entry.
 * @owndata: boolean flag. true (KEEP_RPT_DATA) to tell the interface *not*
 * to free the data when the resource is removed. false (FREE_RPT_DATA) to tell
 * the interface to free the data when the resource is removed.
 *
 * Return value:
 * 0 - successful addition to the RPT.
 * -1 - table pointer is NULL.
 * -2 - entry is NULL.
 * -3 - entry does not have an id assigned.
 * -4 - entry has an invalid/reserved id assinged.
 * -5 - entity path does not contain root element.
 * -6 - failure and not enough memory could be allocated. 
 * for a new position in the RPT.
 **/
int oh_add_resource(RPTable *table, SaHpiRptEntryT *entry, void *data, int owndata)
{
        RPTEntry *rptentry;
        guint update_flag = RPT_KEEP_COUNT;

        if (!table) {
                dbg("ERROR: Cannot work on a null table pointer.");
                return -1;
        } else if (!entry) {
                dbg("Failed to add. RPT entry is NULL.");
                return -2;
        } else if (entry->ResourceId == 0) {
                dbg("Failed to add. RPT entry needs a resource id before being added");
                return -3;                
        } else if (entry->ResourceId == RPT_ENTRY_BEGIN) {
                dbg("Failed to add. RPT entry has an invalid/reserved id assigned (RPT_ENTRY_BEGIN).");
                return -4;
        } else if (check_ep(entry->ResourceEntity)) {
                dbg("Failed to add RPT entry. Entity path does not contain root element.");
                return -5;                
        }

        /* Check to see if the entry is in the RPTable already */
        rptentry = get_rptentry_by_rid(table, entry->ResourceId);        
        /* If not, create new RPTEntry */
        if (!rptentry) {
                rptentry = (RPTEntry *)g_malloc0(sizeof(RPTEntry));
                if (!rptentry) {
                        dbg("Not enough memory to add RPT entry.");
                        return -6;
                }
                /* Put new RPTEntry in RPTable */
                table->rptable = g_slist_append(table->rptable, (gpointer)rptentry);
                update_flag = RPT_INCREMENT;
        }
        /* Else, modify existing RPTEntry */
        rptentry->owndata = owndata;
        rptentry->data = data;
        rptentry->rpt_entry = *entry;
        rptentry->rpt_entry.EntryId = entry->ResourceId;

        update_rptable(table, update_flag);
                       
        return 0; 
}

/**
 * oh_remove_resource: Remove a resource from the RPT. If the rid is
 * RPT_ENTRY_BEGIN (0xffffffff), the first RPT entry in the table will be removed.
 * The void data will be freed if owndata was false (FREE_RPT_DATA) when adding
 * the resource, otherwise if owndata was true (KEEP_RPT_DATA) it will not be freed.
 * @table: Pointer to the RPT from which an RPT entry will be removed.
 * @rid: Resource id of the RPT entry to be removed.
 *
 * Return value:
 * 0 - Successful removal from the RPT.
 * -1 - table pointer is NULL.
 * -2 - Failure. No resource found by that id.
 **/
int oh_remove_resource(RPTable *table, SaHpiResourceIdT rid)
{
        RPTEntry *rptentry;

        if (!table) {
                dbg("ERROR: Cannot work on a null table pointer.");
                return -1;
        }

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                dbg("Failed to remove RPT entry. No Resource found by that id");
                return -2;
        } else {
                SaHpiRdrT *tmp_rdr;
                /* Remove all RDRs for the resource first */
                while ((tmp_rdr = oh_get_rdr_by_id(table, rid, RDR_BEGIN)) != NULL) {
                        oh_remove_rdr(table, rid, RDR_BEGIN);
                }
                /* then remove the resource itself. */
                table->rptable = g_slist_remove(table->rptable, (gpointer)rptentry);
                if (!rptentry->owndata) g_free(rptentry->data);
                g_free((gpointer)rptentry);
        }

        update_rptable(table, RPT_DECREMENT);

        return 0;
}

/**
 * oh_get_resource_data: Get the private data for a RPT entry.  If the rid is
 * RPT_ENTRY_BEGIN (0xffffffff), the first RPT entry's data in the table will be returned.
 * @table: Pointer to the RPT for looking up the RPT entry's private data.
 * @rid: Resource id of the RPT entry that holds the private data.
 *
 * Return value:
 * A void pointer to the private data for the RPT entry requested, or NULL
 * if the RPT entry was not found or the table was a NULL pointer.
 **/
void *oh_get_resource_data(RPTable *table, SaHpiResourceIdT rid)
{
        
        RPTEntry *rptentry;

        if (!table) {
                dbg("ERROR: Cannot work on a null table pointer.");
                return NULL;
        }

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                /*dbg("Warning: RPT entry not found. Returning NULL.");*/
                return NULL;
        }

        return rptentry->data;
}

/**
 * oh_get_resource_by_id: Get a RPT entry from the RPT by using the resource id.
 * If rid is RPT_ENTRY_BEGIN (0xffffffff), the first RPT entry in the table will be returned.
 * @table: Pointer to the RPT for looking up the RPT entry.
 * @rid: Resource id of the RPT entry to be looked up.
 *
 * Return value:
 * Pointer to the RPT entry found or NULL if an RPT entry by that
 * id was not found or the table was a NULL pointer.
 **/
SaHpiRptEntryT *oh_get_resource_by_id(RPTable *table, SaHpiResourceIdT rid)
{
        RPTEntry *rptentry;

        if (!table) {
                dbg("ERROR: Cannot work on a null table pointer.");
                return NULL;
        }

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                /*dbg("Warning: RPT entry not found. Returning NULL.");*/
                return NULL;
        }

        return &(rptentry->rpt_entry);
}

/**
 * oh_get_resource_by_ep: Get a RPT entry from the RPT by using the entity path.
 * @table: Pointer to the RPT for looking up the RPT entry.
 * @ep: Entity path of the RPT entry to be looked up.
 *
 * Return value:
 * Pointer to the RPT entry found or NULL if an RPT entry by that
 * entity path was not found or the table was a NULL pointer.
 **/
SaHpiRptEntryT *oh_get_resource_by_ep(RPTable *table, SaHpiEntityPathT *ep)
{
        RPTEntry *rptentry = NULL;
        GSList *node;

        if (!(table)) {
                dbg("ERROR: Cannot work on a null table pointer.");
                return NULL;
        }

        for (node = table->rptable; node != NULL; node = node->next) {
                rptentry = (RPTEntry *) node->data;
                if (!memcmp(&(rptentry->rpt_entry.ResourceEntity), ep, sizeof(SaHpiEntityPathT)))
                        break;
                else rptentry = NULL;
        }

        if (!rptentry) {
                /*dbg("Warning: RPT entry not found. Returning NULL.");*/
                return NULL;
        }

        return &(rptentry->rpt_entry);
}

/**
 * oh_get_resource_next: Get the RPT entry next to the specified RPT entry
 * from the RPT. If rid_prev is RPT_ENTRY_BEGIN (0xffffffff), the first RPT entry
 * in the table will be returned.
 * @table: Pointer to the RPT for looking up the RPT entry.
 * @rid_prev: Resource id of the RPT entry previous to the one being looked up.
 *
 * Return value:
 * Pointer to the RPT entry found or NULL if the previous RPT entry by that
 * id was not found or the table was a NULL pointer.
 **/
SaHpiRptEntryT *oh_get_resource_next(RPTable *table, SaHpiResourceIdT rid_prev)
{
        RPTEntry *rptentry = NULL;
        GSList *node;

        if (!(table)) {
                dbg("ERROR: Cannot work on a null table pointer.");
                return NULL;
        }

        if (rid_prev == RPT_ENTRY_BEGIN) {
                if (table->rptable) {
                        rptentry = (RPTEntry *)(table->rptable->data);
                } else {
                        /*dbg("Info: RPT is empty. Returning NULL.");*/
                        return NULL;
                }
        } else {
                for (node = table->rptable; node != NULL; node = node->next) {
                        rptentry = (RPTEntry *) node->data;
                        if (rptentry->rpt_entry.ResourceId == rid_prev) {
                                if (node->next) rptentry = (RPTEntry *)(node->next->data);
                                else {
                                        /*dbg("Info: Reached end of RPT.");*/
                                        return NULL;
                                }
                                break;
                        } else rptentry = NULL;
                }                
        }                

        if (!rptentry) {
                /*dbg("Warning: RPT entry not found. Returning NULL.");*/
                return NULL;
        }
        
        return &(rptentry->rpt_entry);
}

/**
 * RDR interface functions
 */

/**
 * oh_add_rdr: Add an RDR to a RPT entry's RDR repository.
 * If an RDR is found with the same record id as the one being added, the RDR being
 * added will overlay the existing one. Also, a unique record id will be assigned
 * to it based on the RDR type and its type's numeric id.
 * @table: Pointer to RPT table containig the RPT entry to which the RDR will belong.
 * @rid: Id of the RPT entry that will own the RDR to be added.
 * @rdr: RDR to be added to an RPT entry's RDR repository.
 * @data: Pointer to private data belonging to the RDR that is being added.
 * @owndata: boolean flag. true (KEEP_RPT_DATA) to tell the interface *not*
 * to free the data when the rdr is removed. false (FREE_RPT_DATA) to tell
 * the interface to free the data when the rdr is removed.
 *
 * All rdr interface funtions, except oh_add_rdr will act in the context of
 * the first RPT entry in the table, if rid is RPT_ENTRY_BEGIN (0xffffffff).
 *
 * Return value:
 * 0 - Successful addition of RDR.
 * -1 - table pointer is NULL.
 * -2 - Failure. RDR is NULL.
 * -3 - Failure. RPT entry for that rid was not found.
 * -4 - Failure. RDR entity path is different from parent RPT entry.
 * -5 - Failure. Could not allocate enough memory to position the new RDR in the RDR
 * repository.
 **/ 
int oh_add_rdr(RPTable *table, SaHpiResourceIdT rid, SaHpiRdrT *rdr, void *data, int owndata)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord;
        SaHpiUint8T type_num;

        if (!table) {
                dbg("Error: Cannot work on a null table pointer.");
                return -1;
        } else if (!rdr) {
                dbg("Failed to add. RDR is NULL.");
                return -2;
        }        

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry){
                dbg("Failed to add RDR. Parent RPT entry was not found in table.");
                return -3;
        }

        if (memcmp(&(rptentry->rpt_entry.ResourceEntity), &(rdr->Entity), sizeof(SaHpiEntityPathT))) {
                dbg("Failed to add RDR. Entity path is different from parent RPT entry.");
                return -4;
        }

        type_num = get_rdr_type_num(rdr);

        /* Form correct RecordId. */
        rdr->RecordId = get_rdr_uid(rdr->RdrType, type_num);
        /* Check if record exists */
        rdrecord = get_rdrecord_by_id(rptentry->rdrtable, rdr->RecordId);                
        /* If not, create new rdr */
        if (!rdrecord) {
                rdrecord = (RDRecord *)g_malloc0(sizeof(RDRecord));
                if (!rdrecord) {
                        dbg("Not enough memory to add RDR.");
                        return -5;
                }
                /* Put new rdrecord in rdr repository */
                rptentry->rdrtable = g_slist_append(rptentry->rdrtable, (gpointer)rdrecord);                        
        }
        /* Else, modify existing rdrecord */
        rdrecord->owndata = owndata;        
        rdrecord->rdr = *rdr;
        rdrecord->data = data;

        update_rptable(table, RPT_KEEP_COUNT);
        
        return 0;
}

/**
 * oh_remove_rdr: Remove an RDR from a RPT entry's RDR repository.
 * If rdrid is RDR_BEGIN (0xffffffff), the first RDR in the repository will be removed.
 * If owndata was set to false (FREE_RPT_DATA) on the rdr when it was added,
 * the data will be freed, otherwise if it was set to true (KEEP_RPT_DATA),
 * it will not be freed.
 * @table: Pointer to RPT table containig the RPT entry from which the RDR will
 * be removed.
 * @rid: Id of the RPT entry from which the RDR will be removed.
 * @rdrid: Record id of the RDR to remove.
 *
 * All rdr interface funtions, except oh_add_rdr will act in the context of
 * the first RPT entry in the table, if rid is RPT_ENTRY_BEGIN (0xffffffff).
 *
 * Return value:
 * 0 - Successful removal of RDR.
 * -1 - table pointer is NULL.
 * -2 - Failure. RPT entry for that rid was not found.
 * -3 - Failure. No RDR by that rdrid was found.
 **/
int oh_remove_rdr(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord;

        if (!table) {
                dbg("Error: Cannot work on a null table pointer.");
                return -1;
        }

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                dbg("Failed to remove RDR. Parent RPT entry was not found.");
                return -2;
        }

        rdrecord = get_rdrecord_by_id(rptentry->rdrtable, rdrid);
        if (!rdrecord) {
                dbg("Failed to remove RDR. Could not be found.");
                return -3;
        } else {
                rptentry->rdrtable = g_slist_remove(rptentry->rdrtable, (gpointer)rdrecord);
                if (!rdrecord->owndata) g_free(rdrecord->data);
                g_free((gpointer)rdrecord);                
        }

        update_rptable(table, RPT_KEEP_COUNT);
        
        return 0;
}

/**
 * oh_get_rdr_data: Get the private data associated to an RDR.
 * If rdrid is RDR_BEGIN (0xffffffff), the first RDR's data in the repository will be returned.
 * @table: Pointer to RPT table containig the RPT entry from which the RDR's data
 * will be read.
 * @rid: Id of the RPT entry from which the RDR's data will be read.
 * @rdrid: Record id of the RDR to read data from.
 *
 * All rdr interface funtions, except oh_add_rdr will act in the context of
 * the first RPT entry in the table, if rid is RPT_ENTRY_BEGIN (0xffffffff).
 *
 * Return value:
 * A void pointer to the RDR data, or NULL if no data for that RDR was found or
 * the table pointer is NULL.
 **/
void *oh_get_rdr_data(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord;

        if (!table) {
                dbg("Error: Cannot work on a null table pointer.");
                return NULL;
        }

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                dbg("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */
        }

        rdrecord = get_rdrecord_by_id(rptentry->rdrtable, rdrid);
        if (!rdrecord) {
                /*dbg("Warning: RDR not found. Returning NULL.");*/
                return NULL;
        }

        return rdrecord->data;
}

/**
 * oh_get_rdr_by_id: Get a reference to an RDR by its record id.
 * If rdrid is RDR_BEGIN (0xffffffff), the first RDR in the repository will be returned.
 * @table: Pointer to RPT table containig the RPT entry tha has the RDR
 * being looked up.
 * @rid: Id of the RPT entry containing the RDR being looked up.
 * @rdrid: Record id of the RDR being looked up.
 *
 * All rdr interface funtions, except oh_add_rdr will act in the context of
 * the first RPT entry in the table, if rid is RPT_ENTRY_BEGIN (0xffffffff).
 *
 * Return value:
 * Reference to the RDR looked up or NULL if no RDR was found.
 **/
SaHpiRdrT *oh_get_rdr_by_id(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord;

        if (!table) {
                dbg("Error: Cannot work on a null table pointer.");
                return NULL;
        }
        
        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                dbg("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */
        }

        rdrecord = get_rdrecord_by_id(rptentry->rdrtable, rdrid);
        if (!rdrecord) {
                /*dbg("Warning: RDR not found. Returning NULL.");*/
                return NULL;
        }

        return &(rdrecord->rdr);
}

/**
 * oh_get_rdr_by_type: Get a reference to an RDR by its type and number.
 * @table: Pointer to RPT table containig the RPT entry tha has the RDR
 * being looked up.
 * @rid: Id of the RPT entry containing the RDR being looked up.
 * @type: RDR Type of the RDR being looked up.
 * @num: RDR id within the RDR type for the specified RPT entry.
 *
 * All rdr interface funtions, except oh_add_rdr will act in the context of
 * the first RPT entry in the table, if rid is RPT_ENTRY_BEGIN (0xffffffff).
 *
 * Return value:
 * Reference to the RDR looked up or NULL if no RDR was found.
 **/
SaHpiRdrT *oh_get_rdr_by_type(RPTable *table, SaHpiResourceIdT rid,
                              SaHpiRdrTypeT type, SaHpiUint8T num)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord;
        SaHpiUint32T rdr_uid;

        if (!table) {
                dbg("Error: Cannot work on a null table pointer.");
                return NULL;
        }
                                                        
        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                dbg("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */
        }

        /* Get rdr_uid from type/num combination */
        rdr_uid = get_rdr_uid(type, num);
        rdrecord = get_rdrecord_by_id(rptentry->rdrtable, (SaHpiEntryIdT)rdr_uid);
        if (!rdrecord) {
                /*dbg("Warning: RDR not found. Returning NULL.");*/
                return NULL;
        }
        
        return &(rdrecord->rdr);
}

/**
 * oh_get_rdr_next: Get the RDR next to the specified RDR in the specified
 * RPT entry's repository. If rdrid_prev is RDR_BEGIN (0xffffffff), the first RDR
 * in the repository will be returned.
 * @table: Pointer to the RPT containing the RPT entry to look up the RDR in.
 * @rid_prev: Record id of the RDR previous to the one being looked up, relative
 * to the specified RPT entry.
 *
 * All rdr interface funtions, except oh_add_rdr will act in the context of
 * the first RPT entry in the table, if rid is RPT_ENTRY_BEGIN (0xffffffff).
 *
 * Return value:
 * Pointer to the RDR found or NULL if the previous RDR by that
 * id was not found. If the rdrid_prev was RDR_BEGIN, the first RDR in the list
 * will be returned.
 **/
SaHpiRdrT *oh_get_rdr_next(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid_prev)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord = NULL;
        GSList *node;

        if (!table) {
                dbg("Error: Cannot work on a null table pointer.");
                return NULL;
        }
                
        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                dbg("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */                
        }
        
        if (rdrid_prev == RDR_BEGIN) {
                if (rptentry->rdrtable) {
                        rdrecord = (RDRecord *)(rptentry->rdrtable->data);                        
                } else {
                        /*dbg("Info: RDR repository is empty. Returning NULL.");*/
                        return NULL;
                }
        } else {
                for (node = rptentry->rdrtable; node != NULL; node = node->next) {
                        rdrecord = (RDRecord *)node->data;
                        if (rdrecord->rdr.RecordId == rdrid_prev) {
                                if (node->next) rdrecord = (RDRecord *)(node->next->data);
                                else {
                                        /*dbg("Info: Reached end of RDR repository.")*/
                                        return NULL;
                                }
                                break;
                        }
                        else rdrecord = NULL;
                }                
        }

        if (!rdrecord) {
                /*dbg("Warning: RDR not found. Returning NULL.");*/
                return NULL;
        }        

        return &(rdrecord->rdr);
}


#if 0
/************************************************************************************
 *
 *  Managed Hotswap State Functions
 *
 *  Note: these might be better off in other files
 *
 ************************************************************************************/

guint32 oh_is_resource_managed(SaHpiResourceIdT rid) {
        ResourceState *r;
        GSList *i;
        g_slist_for_each(i,managed_hs_resources) {
                r = i->data;
                if(r->rid == rid) {
                        return r->state;
                }
        }
        return 0;
}

int oh_set_resource_managed(SaHpiResourceIdT rid, guint32 state) {
        ResourceState *r;
        ResourceState *rnew;
        GSList *i;
        g_slist_for_each(i,managed_hs_resources) {
                r = i->data;
                if(r->rid == rid) {
                        r->state = state;
                        return 0;
                }
        }
        rnew = calloc(1,sizeof(ResourceState));
        rnew->rid = rid;
        rnew->state = state;
        managed_hs_resources = g_slist_append(managed_hs_resources,rnew);
        return 0;
}
#endif
