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
 * main: Starts with an RPTable of 10 resources, adds 1 rdr
 * to first resource. Fetches rdr by its type and num and compares
 * with original. A failed comparison means the test failed,
 * otherwise the test passed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        SaHpiEntryIdT record_id;
        SaHpiRdrT *tmprdr = NULL;
        guint i = 0;

        for (i = 0; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries + i, NULL, 0))
                        return 1;
        }

        if (oh_add_rdr(rptable, RPT_ENTRY_BEGIN, rdrs, NULL,0))
                return 1;

        record_id =
                get_rdr_uid(rdrs[0].RdrType, rdrs[0].RdrTypeUnion.SensorRec.Num);
        rdrs[0].RecordId = record_id;

        tmprdr =
                oh_get_rdr_by_type(rptable, RPT_ENTRY_BEGIN,
                                   rdrs[0].RdrType,
                                   rdrs[0].RdrTypeUnion.SensorRec.Num);
        if (!tmprdr ||
            memcmp(rdrs, tmprdr, sizeof(SaHpiRdrT)))
                return 1;

        return 0;
}
