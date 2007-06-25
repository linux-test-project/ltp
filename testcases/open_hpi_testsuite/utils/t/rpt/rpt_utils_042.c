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
 * main: Starting with an empty RPTable, adds 10 resources to it
 * and then adds 1 rdr to it using a Resource Id not present in the table.
 * Passes the test if the interface returns an error, else it fails.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        guint i;

        for (i = 0; rptentries[i].ResourceId; i++) {
                if (oh_add_resource(rptable, rptentries+i, NULL, 0))
                        return 1;
        }        

        if (!oh_add_rdr(rptable, 1234567, sensors, NULL, 1))
                return 1;

        return 0;
}
