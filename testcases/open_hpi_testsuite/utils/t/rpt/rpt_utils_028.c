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

#include <glib.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <rpt_resources.h>

/**
 * main: Starting with an empty RPTable, adds 2 resources to it with data.
 * Fetches the data back using a Resource Id not present in the table.
 * Passes the test if the interface returns NULL, else it fails.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        gchar *data1 = "My data 1";
        gchar *data2 = "My data 2";

        if (oh_add_resource(rptable, rptentries, data1, KEEP_RPT_DATA))
                return 1;

        if (oh_add_resource(rptable, rptentries+1, data2, KEEP_RPT_DATA))
                return 1;

        if (oh_get_resource_data(rptable, rptentries[2].ResourceId))
                return 1;

        return 0;
}
