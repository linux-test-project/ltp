/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Renier Morales <renier@openhpi.org>
 */

#include <string.h>
#include <sys/time.h>

#include <oh_utils.h>
#include <oh_error.h>

typedef struct {
        SaHpiRptEntryT rpt_entry;
        int owndata;
        void *data; /* private data for the owner of the RPTable */
        GSList *rdrlist; /* Contains RDRecords for sequence lookups */
        GHashTable *rdrtable; /* Contains RDRecords for fast RecordId lookups */
} RPTEntry;

typedef struct {
       SaHpiRdrT rdr;
       int owndata;
       void *data; /* private data for the owner of the rpt entry. */
} RDRecord;


static RPTEntry *get_rptentry_by_rid(RPTable *table, SaHpiResourceIdT rid)
{
        GSList *rptnode = NULL;
        RPTEntry *rptentry = NULL;

        if (!table) {
                err("ERROR: Cannot work on a null table pointer.");
                return NULL;
        }

        if (!(table->rptlist)) {
                /*dbg("Info: RPT is empty.");*/
                return NULL;
        }

        if (rid == SAHPI_FIRST_ENTRY) {
                rptentry = (RPTEntry *) (table->rptlist->data);
        } else {
                rptnode = (GSList *)g_hash_table_lookup(table->rptable, &rid);
                rptentry = rptnode ? (RPTEntry *)rptnode->data : NULL;
        }

        return rptentry;
}

static GSList *get_rptnode_by_rid(RPTable *table, SaHpiResourceIdT rid)
{
        GSList *rptnode = NULL;

        if (!table) {
                err("ERROR: Cannot work on a null table pointer.");
                return NULL;
        }

        if (!(table->rptlist)) {
                /*dbg("Info: RPT is empty.");*/
                return NULL;
        }

        if (rid == SAHPI_FIRST_ENTRY) {
                rptnode = (GSList *) (table->rptlist);
        } else {
                rptnode = (GSList *)g_hash_table_lookup(table->rptable, &rid);
        }

        return rptnode;
}

static RDRecord *get_rdrecord_by_id(RPTEntry *rptentry, SaHpiEntryIdT id)
{
        GSList *rdrnode = NULL;
        RDRecord *rdrecord = NULL;

        if (!rptentry) {
                err("ERROR: Cannot lookup rdr inside null resource.");
                return NULL;
        }

        if (!rptentry->rdrlist) {
                /*dbg("Info: RDR repository is empty.");*/
                return NULL;
        }

        if (id == SAHPI_FIRST_ENTRY) {
                rdrecord = (RDRecord *) (rptentry->rdrlist->data);
        } else {
                rdrnode = (GSList *)g_hash_table_lookup(rptentry->rdrtable, &id);
                rdrecord = rdrnode ? (RDRecord *)rdrnode->data : NULL;
        }

        return rdrecord;
}

static GSList *get_rdrnode_by_id(RPTEntry *rptentry, SaHpiEntryIdT id)
{
        GSList *rdrnode = NULL;

        if (!rptentry) {
                err("ERROR: Cannot lookup rdr inside null resource.");
                return NULL;
        }

        if (!rptentry->rdrlist) {
                /*dbg("Info: RPT is empty.");*/
                return NULL;
        }

        if (id == SAHPI_FIRST_ENTRY) {
                rdrnode = (GSList *) (rptentry->rdrlist);
        } else {
                rdrnode = (GSList *)g_hash_table_lookup(rptentry->rdrtable, &id);
        }

        return rdrnode;
}

static int check_instrument_id(SaHpiRptEntryT *rptentry, SaHpiRdrT *rdr)
{
        int result = 0;
        static const SaHpiInstrumentIdT SENSOR_AGGREGATE_MAX = 0x0000010F;

        switch (rdr->RdrType) {
                case SAHPI_SENSOR_RDR:
                        if (rdr->RdrTypeUnion.SensorRec.Num >= SAHPI_STANDARD_SENSOR_MIN &&
                            rdr->RdrTypeUnion.SensorRec.Num <= SAHPI_STANDARD_SENSOR_MAX) {
                                if (rdr->RdrTypeUnion.SensorRec.Num > SENSOR_AGGREGATE_MAX) {
                                        result = -1;
                                } else if (rptentry->ResourceCapabilities &
                                           SAHPI_CAPABILITY_AGGREGATE_STATUS) {
                                        result = 0;
                                } else {
                                        result = -1;
                                }
                        } else {
                                result = 0;
                        }
                        break;
                default:
                        result = 0;
        }

        return result;
}

static SaHpiInstrumentIdT get_rdr_type_num(SaHpiRdrT *rdr)
{
        SaHpiInstrumentIdT num = 0;

        switch (rdr->RdrType) {
                case SAHPI_CTRL_RDR:
                        num = rdr->RdrTypeUnion.CtrlRec.Num;
                        break;
                case SAHPI_SENSOR_RDR:
                        num = rdr->RdrTypeUnion.SensorRec.Num;
                        break;
                case SAHPI_INVENTORY_RDR:
                        num = rdr->RdrTypeUnion.InventoryRec.IdrId;
                        break;
                case SAHPI_WATCHDOG_RDR:
                        num = rdr->RdrTypeUnion.WatchdogRec.WatchdogNum;
                        break;
                case SAHPI_ANNUNCIATOR_RDR:
                        num = rdr->RdrTypeUnion.AnnunciatorRec.AnnunciatorNum;
                        break;
		case SAHPI_DIMI_RDR:
			num = rdr->RdrTypeUnion.DimiRec.DimiNum;
			break;
		case SAHPI_FUMI_RDR:
			num = rdr->RdrTypeUnion.FumiRec.Num;
			break;
                default:
                        num = 0;
        }

        return num;
}

static void update_rptable(RPTable *table) {
        struct timeval tv;
        SaHpiTimeT time;

        if (!table) {
                err("ERROR: Cannot work on a null table pointer.");
                return;
        }

        gettimeofday(&tv, NULL);
        time = (SaHpiTimeT) tv.tv_sec * 1000000000 + tv.tv_usec * 1000;

        table->update_timestamp = time;
        table->update_count++;
}

/**
 * oh_get_rdr_uid
 * @type: type of rdr
 * @num: id number of the RDR unique withing the RDR type for that resource
 *
 * Helper function to derive the Record id of an rdr from its @type and @num
 *
 * Returns: a derived Record Id used to identify RDRs within Resources
 */
SaHpiEntryIdT oh_get_rdr_uid(SaHpiRdrTypeT type, SaHpiInstrumentIdT num)
{
        SaHpiEntryIdT uid;

        uid = ((SaHpiEntryIdT)type) << 16;
        uid = uid + (SaHpiEntryIdT)num;

        return uid;
}

SaHpiInstrumentIdT oh_get_rdr_num(SaHpiEntryIdT rdrid) {
        return rdrid & 0x0000ffff;
}

/**
 * General RPT calls
 **/

/**
 * oh_init_rpt
 * @table: Pointer to RPTable structure to be initialized.
 *
 *
 * Returns: SA_OK on success Or minus SA_OK on error.
 **/
SaErrorT oh_init_rpt(RPTable *table)
{
        if (!table) {
                err("ERROR: Cannot work on a null table pointer.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        table->update_timestamp = SAHPI_TIME_UNSPECIFIED;
        table->update_count = 0;
        table->rptlist = NULL;
        table->rptable = NULL;

        return SA_OK;
}

/**
 * oh_flush_rpt
 * @table: Pointer to the RPT to flush.
 *
 * Cleans RPT from all entries and RDRs and frees the memory
 * associated with them.
 *
 * Returns: SA_OK on success Or minus SA_OK on error.
 **/
SaErrorT oh_flush_rpt(RPTable *table)
{
        SaHpiRptEntryT *tmp_entry;

        while ((tmp_entry = oh_get_resource_by_id(table, SAHPI_FIRST_ENTRY)) != NULL) {
                oh_remove_resource(table, SAHPI_FIRST_ENTRY);
        }

        return SA_OK;
}

/**
 * rpt_diff
 * @cur_rpt: IN. Pointer to RPTable that represents the current state of resources
 * and rdrs.
 * @new_rpt: IN. Pointer to RPTable that contains rpt entries and rdrs just recently
 * discovered.
 * @res_new: OUT. List of new or changed rpt entries
 * @rdr_new: OUT. List of new or changed rdrs
 * @res_gone: OUT. List of old and not present resources.
 * @rdr_gone: OUT. List of old and not present rdrs.
 *
 * Extracts from current the resources and rdrs that are not found
 * in new and puts them in res_gone and rdr_gone. Also, puts in res_new and rdr_new
 * the resources and rdrs that are not already in current Or that are not identical
 * to the ones in current.
 *
 * Returns: SA_ERR_HPI_INVALID_PARAMS if any argument is NULL, otherwise SA_OK.
 **/
SaErrorT rpt_diff(RPTable *cur_rpt, RPTable *new_rpt,
                  GSList **res_new, GSList **rdr_new,
                  GSList **res_gone, GSList **rdr_gone) {

        SaHpiRptEntryT *res = NULL;

        if (!cur_rpt || !new_rpt ||
            !res_new || !rdr_new || !res_gone || !rdr_gone)
                return SA_ERR_HPI_INVALID_PARAMS;

        /* Look for absent resources and rdrs */
        for (res = oh_get_resource_by_id(cur_rpt, SAHPI_FIRST_ENTRY);
             res != NULL;
             res = oh_get_resource_next(cur_rpt, res->ResourceId)) {

                SaHpiRptEntryT *tmp_res = oh_get_resource_by_id(new_rpt, res->ResourceId);

                if (tmp_res == NULL) *res_gone = g_slist_append(*res_gone, (gpointer)res);
                else {
                        SaHpiRdrT *rdr = NULL;

                        for (rdr = oh_get_rdr_by_id(cur_rpt, res->ResourceId, SAHPI_FIRST_ENTRY);
                             rdr != NULL;
                             rdr = oh_get_rdr_next(cur_rpt, res->ResourceId, rdr->RecordId)) {

                                SaHpiRdrT *tmp_rdr =
                                        oh_get_rdr_by_id(new_rpt, res->ResourceId, rdr->RecordId);

                                if (tmp_rdr == NULL)
                                        *rdr_gone = g_slist_append(*rdr_gone, (gpointer)rdr);
                        }
                }
        }

        /* Look for new resources and rdrs*/
        for (res = oh_get_resource_by_id(new_rpt, SAHPI_FIRST_ENTRY);
             res != NULL;
             res = oh_get_resource_next(new_rpt, res->ResourceId)) {

                SaHpiRptEntryT *tmp_res = oh_get_resource_by_id(cur_rpt, res->ResourceId);
                SaHpiRdrT *rdr = NULL;
                if (tmp_res == NULL || memcmp(res, tmp_res, sizeof(SaHpiRptEntryT))) {
                        *res_new = g_slist_append(*res_new, (gpointer)res);
                }

                for (rdr = oh_get_rdr_by_id(new_rpt, res->ResourceId, SAHPI_FIRST_ENTRY);
                     rdr != NULL;
                     rdr = oh_get_rdr_next(new_rpt, res->ResourceId, rdr->RecordId)) {

                        SaHpiRdrT *tmp_rdr = NULL;

                        if (tmp_res != NULL)
                                tmp_rdr = oh_get_rdr_by_id(cur_rpt, res->ResourceId, rdr->RecordId);

                        if (tmp_rdr == NULL || memcmp(rdr, tmp_rdr, sizeof(SaHpiRdrT)))
                                *rdr_new = g_slist_append(*rdr_new, (gpointer)rdr);

                }
        }
        
        return SA_OK;
}

/**
 * oh_get_rpt_info
 * @table: pointer to RPT
 * @update_count: pointer of where to place the rpt's update count
 * @update_timestamp: pointer of where to place the rpt's update timestamp
 *
 * Returns: SA_OK on success Or minus SA_OK on error.
 **/
SaErrorT oh_get_rpt_info(RPTable *table,
                         SaHpiUint32T *update_count,
                         SaHpiTimeT *update_timestamp)
{
        if (!table || !update_count || !update_timestamp) {
                err("ERROR: Invalid parameters.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        *update_count = table->update_count;
        *update_timestamp = table->update_timestamp;

        return SA_OK;
}

/**
 * Resource interface functions
 */

/**
 * oh_add_resource
 * @table: Pointer to the RPT to which the RPT entry will be added.
 * @entry: The RPT entry (resource) to be added to the RPT.
 * @data: Pointer to private data for storing along with the RPT entry.
 * @owndata: boolean flag. true (%KEEP_RPT_DATA) to tell the interface *not*
 * to free the data when the resource is removed. false (%FREE_RPT_DATA) to tell
 * the interface to free the data when the resource is removed.
 *
 * Add a RPT entry to the RPT along with some private data.
 * If an RPT entry with the same resource id exists int the RPT, it will be
 * overlayed with the new RPT entry. Also, this function will assign the
 * resource id as its entry id since it is expected to be unique for the table.
 * The update count and timestamp will not be updated if the entry being added
 * already existed in the table and was the same.
 *
 * Returns: SA_OK on success Or minus SA_OK on error. SA_ERR_HPI_INVALID_PARAMS will
 * be returned if @table is NULL, @entry is NULL, ResourceId in @entry equals
 * SAHPI_FIRST_ENTRY, ResourceId in @entry equals SAHPI_UNSPECIFIED_RESOURCE_ID,
 * or ResourceEntity in @entry has a malformed entity path (look at the
 * entity path utils documentation).
 **/
SaErrorT oh_add_resource(RPTable *table, SaHpiRptEntryT *entry, void *data, int owndata)
{
        RPTEntry *rptentry;
        int update_info = 0;

        if (!table) {
                err("ERROR: Cannot work on a null table pointer.");
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (!entry) {
                err("Failed to add. RPT entry is NULL.");
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (entry->ResourceId == SAHPI_FIRST_ENTRY) {
                err("Failed to add. RPT entry needs a resource id before being added");
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (entry->ResourceId == SAHPI_UNSPECIFIED_RESOURCE_ID) {
                err("Failed to add. RPT entry has an invalid/reserved id assigned. (SAHPI_UNSPECIFIED_RESOURCE_ID)");
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (!oh_valid_ep(&(entry->ResourceEntity))) {
                err("Failed to add RPT entry. Entity path does not contain root element.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        entry->EntryId = entry->ResourceId;
        /* Check to see if the entry is in the RPTable already */
        rptentry = get_rptentry_by_rid(table, entry->ResourceId);
        /* If not, create new RPTEntry */
        if (!rptentry) {
                rptentry = (RPTEntry *)g_malloc0(sizeof(RPTEntry));
                if (!rptentry) {
                        err("Not enough memory to add RPT entry.");
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
                update_info = 1; /* Have a new changed entry */
                /* Put new RPTEntry in RPTable */
                table->rptlist = g_slist_append(table->rptlist, (gpointer)rptentry);

                /* Add to rpt hash table */
                if (!table->rptable) /* Create hash table if it doesn't exist */
                        table->rptable = g_hash_table_new(g_int_hash, g_int_equal);

                rptentry->rpt_entry.EntryId = entry->ResourceId;
                g_hash_table_insert(table->rptable,
                                    &(rptentry->rpt_entry.EntryId),
                                    g_slist_last(table->rptlist));
        }
        /* Else, modify existing RPTEntry */
        if (rptentry->data && rptentry->data != data && !rptentry->owndata)
                g_free(rptentry->data);
        rptentry->data = data;
        rptentry->owndata = owndata;
        /* Check if we really have a new/changed entry */
        if (update_info || memcmp(entry, &(rptentry->rpt_entry), sizeof(SaHpiRptEntryT))) {
                update_info = 1;
                rptentry->rpt_entry = *entry;
        }

        if (update_info) update_rptable(table);

        return SA_OK;
}

/**
 * oh_remove_resource
 * @table: Pointer to the RPT from which an RPT entry will be removed.
 * @rid: Resource id of the RPT entry to be removed.
 *
 * Remove a resource from the RPT. If the @rid is
 * %SAHPI_FIRST_ENTRY, the first RPT entry in the table will be removed.
 * The void data will be freed if @owndata was false (%FREE_RPT_DATA) when adding
 * the resource, otherwise if @owndata was true (%KEEP_RPT_DATA) it will not be freed.
 *
 * Returns: SA_OK on success Or minus SA_OK on error.
 **/
SaErrorT oh_remove_resource(RPTable *table, SaHpiResourceIdT rid)
{
        RPTEntry *rptentry;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                err("Failed to remove RPT entry. No Resource found by that id");
                return SA_ERR_HPI_NOT_PRESENT;
        } else {
                SaHpiRdrT *tmp_rdr;
                /* Remove all RDRs for the resource first */
                while ((tmp_rdr = oh_get_rdr_by_id(table, rid, SAHPI_FIRST_ENTRY)) != NULL) {
                        oh_remove_rdr(table, rid, SAHPI_FIRST_ENTRY);
                }
                /* then remove the resource itself. */
                table->rptlist = g_slist_remove(table->rptlist, (gpointer)rptentry);
                if (!rptentry->owndata) g_free(rptentry->data);
                g_hash_table_remove(table->rptable, &(rptentry->rpt_entry.EntryId));
                g_free((gpointer)rptentry);
                if (!table->rptlist) {
                        g_hash_table_destroy(table->rptable);
                        table->rptable = NULL;
                }
        }

        update_rptable(table);

        return SA_OK;
}

/**
 * oh_get_resource_data
 * @table: Pointer to the RPT for looking up the RPT entry's private data.
 * @rid: Resource id of the RPT entry that holds the private data.
 *
 * Get the private data for a RPT entry.  If the @rid is
 * %SAHPI_FIRST_ENTRY, the first RPT entry's data in the table will be returned.
 *
 * Returns: A void pointer to the private data for the RPT entry requested, or NULL
 * if the RPT entry was not found or the table was a NULL pointer.
 **/
void *oh_get_resource_data(RPTable *table, SaHpiResourceIdT rid)
{

        RPTEntry *rptentry;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                /*dbg("Warning: RPT entry not found. Returning NULL.");*/
                return NULL;
        }

        return rptentry->data;
}

/**
 * oh_get_resource_by_id
 * @table: Pointer to the RPT for looking up the RPT entry.
 * @rid: Resource id of the RPT entry to be looked up.
 *
 * Get a RPT entry from the RPT by using the resource id.
 * If @rid is %SAHPI_FIRST_ENTRY, the first RPT entry in the table will be returned.
 *
 * Returns:
 * Pointer to the RPT entry found or NULL if an RPT entry by that
 * id was not found or the table was a NULL pointer.
 **/
SaHpiRptEntryT *oh_get_resource_by_id(RPTable *table, SaHpiResourceIdT rid)
{
        RPTEntry *rptentry;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                /*dbg("Warning: RPT entry not found. Returning NULL.");*/
                return NULL;
        }

        return &(rptentry->rpt_entry);
}

/**
 * oh_get_resource_by_ep
 * @table: Pointer to the RPT for looking up the RPT entry.
 * @ep: Entity path of the RPT entry to be looked up.
 *
 * Get a RPT entry from the RPT by using the entity path.
 *
 * Returns:
 * Pointer to the RPT entry found or NULL if an RPT entry by that
 * entity path was not found or the table was a NULL pointer.
 **/
SaHpiRptEntryT *oh_get_resource_by_ep(RPTable *table, SaHpiEntityPathT *ep)
{
        RPTEntry *rptentry = NULL;
        GSList *node = NULL;
        SaHpiResourceIdT rid = 0;

        if (!table) {
                err("ERROR: Cannot work on a null table pointer.");
                return NULL;
        }
        /* Check the uid database first */
        rid = oh_uid_is_initialized() ? oh_uid_lookup(ep) : 0;
        if (rid > 0) {
                /* Found it in uid database */
                return oh_get_resource_by_id(table, rid);
        } else {
                dbg("Didn't find the EP in the Uid table so "
                    "looking manually in the RPTable");
        }

        for (node = table->rptlist; node != NULL; node = node->next) {
                rptentry = (RPTEntry *) node->data;
                if (oh_cmp_ep(&(rptentry->rpt_entry.ResourceEntity), ep))
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
 * oh_get_resource_next
 * @table: Pointer to the RPT for looking up the RPT entry.
 * @rid_prev: Resource id of the RPT entry previous to the one being looked up.
 *
 * Get the RPT entry next to the specified RPT entry
 * from the RPT. If @rid_prev is %SAHPI_FIRST_ENTRY, the first RPT entry
 * in the table will be returned.
 *
 * Returns:
 * Pointer to the RPT entry found or NULL if the previous RPT entry by that
 * id was not found or the table was a NULL pointer.
 **/
SaHpiRptEntryT *oh_get_resource_next(RPTable *table, SaHpiResourceIdT rid_prev)
{
        RPTEntry *rptentry = NULL;
        GSList *rptnode = NULL;

        if (rid_prev == SAHPI_FIRST_ENTRY) {
                rptentry = get_rptentry_by_rid(table, rid_prev);
        } else {
                rptnode = get_rptnode_by_rid(table, rid_prev);
                if (rptnode && rptnode->next) {
                        rptentry = (RPTEntry *)rptnode->next->data;
                }
        }

        return rptentry ? &(rptentry->rpt_entry) : NULL;
}

/**
 * RDR interface functions
 */

/**
 * oh_add_rdr
 * @table: Pointer to RPT table containig the RPT entry to which the RDR will belong.
 * @rid: Id of the RPT entry that will own the RDR to be added.
 * @rdr: RDR to be added to an RPT entry's RDR repository.
 * @data: Pointer to private data belonging to the RDR that is being added.
 * @owndata: boolean flag. true (%KEEP_RPT_DATA) to tell the interface *not*
 * to free the data when the rdr is removed. false (%FREE_RPT_DATA) to tell
 * the interface to free the data when the rdr is removed.
 *
 * Add an RDR to a RPT entry's RDR repository.
 * If an RDR is found with the same record id as the one being added, the RDR being
 * added will overlay the existing one. Also, a unique record id will be assigned
 * to it based on the RDR type and its type's numeric id.
 * All rdr interface funtions, except oh_add_rdr() will act in the context of
 * the first RPT entry in the table, if @rid is %SAHPI_FIRST_ENTRY.
 *
 * Returns: SA_OK on success Or minus SA_OK on error. Will return
 * SA_ERR_HPI_INVALID_PARAMS if instrument id is invalid. An invalid
 * intrument id for a sensor is in the range of 0x100-0x1FF. An aggregate type
 * of sensor can have its instrument id in the range of 0x100-0x10F, but
 * the resource must have the aggregate sensor capabilitiy bit set.
 **/
SaErrorT oh_add_rdr(RPTable *table, SaHpiResourceIdT rid, SaHpiRdrT *rdr, void *data, int owndata)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord;
        SaHpiInstrumentIdT type_num;

        if (!rdr) {
                err("Failed to add. RDR is NULL.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry){
                err("Failed to add RDR. Parent RPT entry was not found in table.");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (check_instrument_id(&(rptentry->rpt_entry), rdr)) {
                err("Invalid instrument id found in RDR.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        type_num = get_rdr_type_num(rdr);

        /* Form correct RecordId. */
        rdr->RecordId = oh_get_rdr_uid(rdr->RdrType, type_num);
        /* Check if record exists */
        rdrecord = get_rdrecord_by_id(rptentry, rdr->RecordId);
        /* If not, create new rdr */
        if (!rdrecord) {
                rdrecord = (RDRecord *)g_malloc0(sizeof(RDRecord));
                if (!rdrecord) {
                        err("Not enough memory to add RDR.");
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
                /* Put new rdrecord in rdr repository */
                rptentry->rdrlist = g_slist_append(rptentry->rdrlist, (gpointer)rdrecord);
                /* Create rdr hash table if first rdr here */
                if (!rptentry->rdrtable)
                        rptentry->rdrtable = g_hash_table_new(g_int_hash, g_int_equal);

                rdrecord->rdr.RecordId = rdr->RecordId;
                g_hash_table_insert(rptentry->rdrtable,
                                    &(rdrecord->rdr.RecordId),
                                    g_slist_last(rptentry->rdrlist));
        }
        /* Else, modify existing rdrecord */
        if (rdrecord->data && rdrecord->data != data && !rdrecord->owndata)
                g_free(rdrecord->data);
        rdrecord->data = data;
        rdrecord->owndata = owndata;
        rdrecord->rdr = *rdr;

        return SA_OK;
}

/**
 * oh_remove_rdr
 * @table: Pointer to RPT table containig the RPT entry from which the RDR will
 * be removed.
 * @rid: Id of the RPT entry from which the RDR will be removed.
 * @rdrid: Record id of the RDR to remove.
 *
 *
 * Remove an RDR from a RPT entry's RDR repository.
 * If @rdrid is %SAHPI_FIRST_ENTRY, the first RDR in the repository will be removed.
 * If @owndata was set to false (%FREE_RPT_DATA) on the rdr when it was added,
 * the data will be freed, otherwise if it was set to true (%KEEP_RPT_DATA),
 * it will not be freed.
 * All rdr interface funtions, except oh_add_rdr() will act in the context of
 * the first RPT entry in the table, if @rid is %SAHPI_FIRST_ENTRY.
 *
 * Returns: SA_OK on success Or minus SA_OK on error.
 **/
SaErrorT oh_remove_rdr(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                err("Failed to remove RDR. Parent RPT entry was not found.");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        rdrecord = get_rdrecord_by_id(rptentry, rdrid);
        if (!rdrecord) {
                err("Failed to remove RDR. Could not be found.");
                return SA_ERR_HPI_NOT_PRESENT;
        } else {
                rptentry->rdrlist = g_slist_remove(rptentry->rdrlist, (gpointer)rdrecord);
                if (!rdrecord->owndata) g_free(rdrecord->data);
                g_hash_table_remove(rptentry->rdrtable, &(rdrecord->rdr.RecordId));
                g_free((gpointer)rdrecord);
                if (!rptentry->rdrlist) {
                        g_hash_table_destroy(rptentry->rdrtable);
                        rptentry->rdrtable = NULL;
                }
        }

        return SA_OK;
}

/**
 * oh_get_rdr_data
 * @table: Pointer to RPT table containig the RPT entry from which the RDR's data
 * will be read.
 * @rid: Id of the RPT entry from which the RDR's data will be read.
 * @rdrid: Record id of the RDR to read data from.
 *
 *
 * Get the private data associated to an RDR.
 * If @rdrid is %SAHPI_FIRST_ENTRY, the first RDR's data in the repository will be returned.
 * All rdr interface funtions, except oh_add_rdr() will act in the context of
 * the first RPT entry in the table, if @rid is %SAHPI_FIRST_ENTRY.
 *
 * Returns: A void pointer to the RDR data, or NULL if no data for that RDR was found or
 * the table pointer is NULL.
 **/
void *oh_get_rdr_data(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                err("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */
        }

        rdrecord = get_rdrecord_by_id(rptentry, rdrid);
        if (!rdrecord) {
                /*dbg("Warning: RDR not found. Returning NULL.");*/
                return NULL;
        }

        return rdrecord->data;
}

/**
 * oh_get_rdr_by_id
 * @table: Pointer to RPT table containig the RPT entry tha has the RDR
 * being looked up.
 * @rid: Id of the RPT entry containing the RDR being looked up.
 * @rdrid: Record id of the RDR being looked up.
 *
 * Get a reference to an RDR by its record id.
 * If @rdrid is %SAHPI_FIRST_ENTRY, the first RDR in the repository will be returned.
 * All rdr interface funtions, except oh_add_rdr() will act in the context of
 * the first RPT entry in the table, if @rid is %SAHPI_FIRST_ENTRY.
 *
 * Returns:
 * Reference to the RDR looked up or NULL if no RDR was found.
 **/
SaHpiRdrT *oh_get_rdr_by_id(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid)
{
        RPTEntry *rptentry;
        RDRecord *rdrecord;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                err("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */
        }

        rdrecord = get_rdrecord_by_id(rptentry, rdrid);
        if (!rdrecord) {
                /*dbg("Warning: RDR not found. Returning NULL.");*/
                return NULL;
        }

        return &(rdrecord->rdr);
}

/**
 * oh_get_rdr_by_type
 * @table: Pointer to RPT table containig the RPT entry tha has the RDR
 * being looked up.
 * @rid: Id of the RPT entry containing the RDR being looked up.
 * @type: RDR Type of the RDR being looked up.
 * @num: RDR id within the RDR type for the specified RPT entry.
 *
 * Get a reference to an RDR by its type and number.
 * All rdr interface funtions, except oh_add_rdr() will act in the context of
 * the first RPT entry in the table, if @rid is %SAHPI_FIRST_ENTRY.
 *
 * Returns:
 * Reference to the RDR looked up or NULL if no RDR was found.
 **/
SaHpiRdrT *oh_get_rdr_by_type(RPTable *table, SaHpiResourceIdT rid,
                              SaHpiRdrTypeT type, SaHpiInstrumentIdT num)
{
        RPTEntry *rptentry = NULL;
        RDRecord *rdrecord = NULL;
        SaHpiEntryIdT rdr_uid;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                err("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */
        }
        
        /* Get rdr_uid from type/num combination */
        rdr_uid = oh_get_rdr_uid(type, num);
        rdrecord = get_rdrecord_by_id(rptentry, rdr_uid);
        if (!rdrecord) {
                /*dbg("Warning: RDR not found. Returning NULL.");*/
                return NULL;
        }

        return &(rdrecord->rdr);        
}

/**
 * oh_get_rdr_next
 * @table: Pointer to the RPT containing the RPT entry to look up the RDR in.
 * @rid: Id of the RPT entry containing the RDR being looked up.
 * @rdrid_prev: Record id of the RDR previous to the one being looked up, relative
 * to the specified RPT entry.
 *
 * Get the RDR next to the specified RDR in the specified
 * RPT entry's repository. If @rdrid_prev is %SAHPI_FIRST_ENTRY, the first RDR
 * in the repository will be returned.
 * All rdr interface funtions, except oh_add_rdr() will act in the context of
 * the first RPT entry in the table, if @rid is %SAHPI_FIRST_ENTRY.
 *
 * Returns:
 * Pointer to the RDR found or NULL if the previous RDR by that
 * id was not found. If the @rdrid_prev was %SAHPI_FIRST_ENTRY, the first RDR in the list
 * will be returned.
 **/
SaHpiRdrT *oh_get_rdr_next(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid_prev)
{
        RPTEntry *rptentry = NULL;
        RDRecord *rdrecord = NULL;
        GSList *rdrnode = NULL;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                err("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */
        }

        if (rdrid_prev == SAHPI_FIRST_ENTRY) {
                rdrecord = get_rdrecord_by_id(rptentry, rdrid_prev);
        } else {
                rdrnode = get_rdrnode_by_id(rptentry, rdrid_prev);
                if (rdrnode && rdrnode->next) {
                        rdrecord = (RDRecord *)rdrnode->next->data;
                }
        }

        return rdrecord ? &(rdrecord->rdr) : NULL;
}

SaHpiRdrT *oh_get_rdr_by_type_first(RPTable *table, SaHpiResourceIdT rid,
                                    SaHpiRdrTypeT type)
{
        RPTEntry *rptentry = NULL;
        RDRecord *rdrecord = NULL;
        GSList *node = NULL;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                err("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */
        }
        
        /* Get first RDR matching the type */
        for (node = rptentry->rdrlist; node; node = node->next) {
                RDRecord *temp = (RDRecord *)node->data;
                if (temp->rdr.RdrType == type) {
                        rdrecord = temp;
                        break;
                }
        }                
        if (!rdrecord) return NULL;

        return &(rdrecord->rdr);
}

SaHpiRdrT *oh_get_rdr_by_type_next(RPTable *table, SaHpiResourceIdT rid,
                                   SaHpiRdrTypeT type, SaHpiInstrumentIdT num)
{
        RPTEntry *rptentry = NULL;
        RDRecord *rdrecord = NULL;
        GSList *node = NULL;

        rptentry = get_rptentry_by_rid(table, rid);
        if (!rptentry) {
                err("Warning: RPT entry not found. Cannot find RDR.");
                return NULL; /* No resource found by that id */
        }
        
        /* Get rdr_uid from type/num combination */
        node = get_rdrnode_by_id(rptentry, oh_get_rdr_uid(type, num));
        if (!node) return NULL;
        
        for (node = node->next; node; node = node->next) {
                RDRecord *temp = (RDRecord *)node->data;
                if (temp->rdr.RdrType == type) {
                        rdrecord = temp;
                        break;
                }
        }
        if (!rdrecord) return NULL;

        return &(rdrecord->rdr);
}
