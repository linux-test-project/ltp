/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      David Ashley<dashley@us.ibm.com>
 *
 */

#ifndef SEL_UTILS_H
#define SEL_UTILS_H
#include <SaHpi.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif 

/* this struct encapsulates all the data for a system event log */
/* the log records themselves are stored in the sel GList */
typedef struct {
        SaHpiBoolT       enabled; // log enabled?
        SaHpiBoolT       overflow; // log overflowed?
        SaHpiBoolT       deletesupported; // delete operation supported?
        SaHpiTimeT       lastUpdate; // last entry's timestamp
        SaHpiTimeT       offset; // offset to be added when generating a timestamp
        SaHpiSelEntryIdT nextId; // next generated Id i.e. number of entries
        GList            *selentries; // list of SaHpiSelEntryT structs
} oh_sel;

/* General SEL utility calls */
oh_sel *oh_sel_create(void);
SaErrorT oh_sel_close(oh_sel *sel);
SaErrorT oh_sel_add(oh_sel *sel, SaHpiSelEntryT *entry);
SaErrorT oh_sel_delete(oh_sel *sel, SaHpiEntryIdT *entryid);
SaErrorT oh_sel_clear(oh_sel *sel);
SaErrorT oh_sel_get(oh_sel *sel, SaHpiSelEntryIdT entryid, SaHpiSelEntryIdT *prev,
                    SaHpiSelEntryIdT *next, SaHpiSelEntryT **entry);
SaErrorT oh_sel_info(oh_sel *sel, SaHpiSelInfoT *info);
SaErrorT oh_sel_map_to_file(oh_sel *sel, char *filename);
SaErrorT oh_sel_map_from_file(oh_sel *sel, char *filename);
SaErrorT oh_sel_timeset(oh_sel *sel, SaHpiTimeT timestamp);


#ifdef __cplusplus
}
#endif

#endif

