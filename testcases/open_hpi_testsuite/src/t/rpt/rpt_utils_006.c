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
 * main: Starts with an RPTable of 10 resources, starting at
 * the beginning on going on to the next, compares
 * resource ids against the originals. A failed comparison
 * means the test failed, otherwise the test passed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        SaHpiRptEntryT *tmpentry = NULL;
        guint i = 0;
        
        for (i = 0; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries + i, NULL, 0))
                        return 1;                
        }

        for (i = 0, tmpentry = oh_get_resource_by_id(rptable, RPT_ENTRY_BEGIN);
             tmpentry;
             tmpentry = oh_get_resource_next(rptable, tmpentry->ResourceId)) {
                if (rptentries[i++].ResourceId != tmpentry->ResourceId)
                        return 1;
        }

        return 0;
}
