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
 *
 */

#include <SaHpi.h>
#include <string.h>
#include <glib.h>
#include <oh_utils.h>
#include <rpt_resources.h>

/**
 * main: Starts with an RPTable of 10 resources, adds 5 rdr
 * to first resource. Fetches sensors ++in sequence by record id and compares
 * with original. A failed comparison means the test failed,
 * otherwise the test passed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        SaHpiRdrT *tmprdr = NULL;
        guint i = 0;

        for (i = 0; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries + i, NULL, 0))
                        return 1;
        }

        for (i = 0; i < 5; i++) {
                if (oh_add_rdr(rptable, SAHPI_FIRST_ENTRY, sensors + i, NULL,0))
                        return 1;                
        }

        for (i = 0, tmprdr = oh_get_rdr_by_id(rptable, SAHPI_FIRST_ENTRY, SAHPI_FIRST_ENTRY);
             tmprdr;
             tmprdr = oh_get_rdr_next(rptable, SAHPI_FIRST_ENTRY, tmprdr->RecordId)) {
                if (memcmp(sensors + (i++), tmprdr, sizeof(SaHpiRdrT)))
                        return 1;                
        }

        return 0;
}
