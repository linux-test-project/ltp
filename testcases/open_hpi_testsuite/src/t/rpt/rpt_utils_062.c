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
 * main: Starts with an RPTable of 10 resources (one with data), adds 5 rdr
 * to first resource and 2 to the last one.
 * Invokes oh_flush on the table.
 * Should return without crashing and there should be no resources left
 * in the table.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        guint i;
        gchar *data = "My data.";

        if (oh_add_resource(rptable, rptentries, data, 1))
                return 1;

        for (i = 1; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries + i, NULL, 0))
                        return 1;
        }

        for (i = 0; i < 5; i++) {
                if (oh_add_rdr(rptable, RPT_ENTRY_BEGIN, rdrs + i, NULL, 0))
                        return 1;
        }

        for (; i < 7; i++) {
                if (oh_add_rdr(rptable, rptentries[9].ResourceId, rdrs + i, NULL, 0))
                        return 1;
        }

        oh_flush_rpt(rptable);

        if (oh_get_resource_by_id(rptable, RPT_ENTRY_BEGIN))
                return 1;

        return 0;
}
