/* -*- linux-c -*-
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
 *     Renier Morales <renierm@users.sf.net>
 *
 */

#include <SaHpi.h>
#include <string.h>
#include <glib.h>
#include <rpt_utils.h>
#include <rpt_resources.h>

/**
 * main: Starts with an RPTable of 10 resources, multiple rdrs
 * on some resources. Remove rdr. Check if resource was removed
 * searching for it in sequence. If not fail, else passed test.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        SaHpiRdrT *tmprdr = NULL;
        guint i = 0;

        for (i = 0; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries + i, NULL, 0))
                        return 1;
        }

        for (i = 0; i < 5; i++) {
                if (oh_add_rdr(rptable, RPT_ENTRY_BEGIN, rdrs + i, NULL,0))
                        return 1;
        }

        for (; i < 7; i++) {
                if (oh_add_rdr(rptable, rptentries[9].ResourceId, rdrs + i, NULL,0))
                        return 1;
        }

        oh_remove_rdr(rptable, rptentries[0].ResourceId, rdrs[1].RecordId);
        for (tmprdr = oh_get_rdr_by_id(rptable, rptentries[0].ResourceId, RDR_BEGIN);
             tmprdr;
             tmprdr = oh_get_rdr_next(rptable, rptentries[0].ResourceId, tmprdr->RecordId)) {
                if (tmprdr->RecordId == rdrs[1].RecordId) return 1;
        }             

        return 0;
}
