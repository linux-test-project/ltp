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
 *     Renier Morales <renier@openhpi.org>
 */

#include <glib.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <rpt_resources.h>

/**
 * main: Starts with an RPTable of 10 resources, adds 1 rdr
 * to first resource. Fetches rdr by record id and compares
 * with original. A failed comparison means the test failed,
 * otherwise the test passed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        SaHpiEntryIdT record_id;
        SaHpiRdrT *tmprdr = NULL;
        guint i = 0;
        
        for (i = 0; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries + i, NULL, 0))                        
                        return 1;
        }

        if (oh_add_rdr(rptable, SAHPI_FIRST_ENTRY, sensors, NULL,0)) {
                return 1;
        }
        
        record_id =
                oh_get_rdr_uid(sensors[0].RdrType, sensors[0].RdrTypeUnion.SensorRec.Num);
        sensors[0].RecordId = record_id;        
                        
        tmprdr = oh_get_rdr_by_id(rptable, SAHPI_FIRST_ENTRY, record_id);
        if (!tmprdr ||
            memcmp(sensors, tmprdr, sizeof(SaHpiRdrT))) {
                return 1;
        }

        return 0;
}
