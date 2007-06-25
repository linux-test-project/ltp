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
 * main: Starting with an empty RPTable, adds 10 resources to it.
 * Fetches the first resource back by id using SAHPI_FIRST_ENTRY for a Resource Id.
 * Passes the test if the interface returns the resource, else it fails.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        SaHpiRptEntryT *tmpentry = NULL;
        guint i;

        for (i = 0; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries+i, NULL, 1))
                        return 1;                
        }        

        tmpentry = oh_get_resource_by_id(rptable, SAHPI_FIRST_ENTRY);
        if (!tmpentry)
                return 1;

        if (tmpentry->ResourceId != rptentries[0].ResourceId)
                return 1;

        return 0;
}
