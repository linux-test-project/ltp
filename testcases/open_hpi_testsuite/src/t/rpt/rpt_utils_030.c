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
 * main: Starting with an empty RPTable, adds one resource to it with NULL data
 * and another with data.
 * Fetches the resource's NULL data back.
 * Passes the test if the interface returns the NULL data, else it fails.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        gchar *data = "My data";
        
        if (oh_add_resource(rptable, rptentries, data, 1))
                return 1;

        if (oh_add_resource(rptable, rptentries+1, NULL, 1))
                return 1;

        if (oh_get_resource_data(rptable, rptentries[1].ResourceId))
                return 1;

        return 0;
}
