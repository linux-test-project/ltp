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

#ifndef RPT_UTILS_H
#define RPT_UTILS_H
#include <SaHpi.h>
#include <glib.h>

/* Special Resource Id and Record Id values */
#define RPT_ENTRY_BEGIN 0xffffffff
#define RDR_BEGIN       0xffffffff

/* Internally used by the interface. */
/* Says wether to increment the count in rpt_info,*/
/* decrement it, or keep it the same.*/
#define RPT_KEEP_COUNT  3
#define RPT_INCREMENT   2
#define RPT_DECREMENT   1

/* oh_add_resource/rdr free-data flag */
#define FREE_RPT_DATA SAHPI_FALSE
#define KEEP_RPT_DATA SAHPI_TRUE

#ifdef __cplusplus
extern "C" {
#endif 

extern GSList *managed_hs_resources;
/* a simple typedef to store a state flag with a resource
   as a bundle */
typedef struct {
        SaHpiResourceIdT rid;
        guint32 state;
} ResourceState;

typedef struct {
        SaHpiRptInfoT rpt_info;
        /* The structure to hold this is subject to change. */
        /* No one should touch this. */
        GSList *rptable; /* Contains RPTEntrys */
} RPTable;

typedef struct {
        SaHpiRptEntryT rpt_entry;
        int owndata;
        void *data; /* private data for the owner of the RPTable */
        GSList *rdrtable;  /* Contains RDRecords */
} RPTEntry;

typedef struct {
       SaHpiRdrT rdr;
       int owndata;
       void *data; /* private data for the owner of the rpt entry. */
} RDRecord;


/* General RPT calls */
void oh_flush_rpt(RPTable *table);
void rpt_diff(RPTable *cur_rpt, RPTable *new_rpt,
              GSList **res_new, GSList **rdr_new,
              GSList **res_gone, GSList **rdr_gone);

/* Resource calls */
int oh_add_resource(RPTable *table, SaHpiRptEntryT *entry, void *data, int owndata);

int oh_remove_resource(RPTable *table, SaHpiResourceIdT rid);

void *oh_get_resource_data(RPTable *table, SaHpiResourceIdT rid);
SaHpiRptEntryT *oh_get_resource_by_id(RPTable *table, SaHpiResourceIdT rid);
SaHpiRptEntryT *oh_get_resource_by_ep(RPTable *table, SaHpiEntityPathT *ep);
SaHpiRptEntryT *oh_get_resource_next(RPTable *table, SaHpiResourceIdT rid_prev);

/* RDR calls */
int oh_add_rdr(RPTable *table, SaHpiResourceIdT rid, SaHpiRdrT *rdr, void *data, int owndata);

int oh_remove_rdr(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid);

void *oh_get_rdr_data(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid);
SaHpiRdrT *oh_get_rdr_by_id(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid);
SaHpiRdrT *oh_get_rdr_by_type(RPTable *table, SaHpiResourceIdT rid,
                              SaHpiRdrTypeT type, SaHpiUint8T num);
SaHpiRdrT *oh_get_rdr_next(RPTable *table, SaHpiResourceIdT rid, SaHpiEntryIdT rdrid_prev);
SaHpiUint32T get_rdr_uid(SaHpiRdrTypeT type, SaHpiUint32T num);

#if 0
/* Other state information about managed resources */
guint32 oh_is_resource_managed(SaHpiResourceIdT rid);
int oh_set_resource_managed(SaHpiResourceIdT rid, guint32 i);
#endif

#ifdef __cplusplus
}
#endif

#endif
