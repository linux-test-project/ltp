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
 * main: Starting with an empty RPTable, adds 10 resources to it.
 * Fetches the resource back by id using a Resource Id not present in the table.
 * Passes the test if the interface returns NULL, else it fails.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        guint i;

        for (i = 0; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries+i, NULL, 1))
                        return 1;                
        }        

        if (oh_get_resource_by_id(rptable, 1234567))
                return 1;

        return 0;
}
