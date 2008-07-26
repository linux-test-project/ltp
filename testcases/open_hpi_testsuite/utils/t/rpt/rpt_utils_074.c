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
 * main: Invokes rpt_diff with new_res param as NULL.
 * If it returns error, the test passes, otherwise it failed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *curr_table = (RPTable *)g_malloc0(sizeof(RPTable));
        RPTable *new_table = (RPTable *)g_malloc0(sizeof(RPTable));
	GSList *new_rdr = NULL, *gone_res = NULL, *gone_rdr = NULL;

	oh_init_rpt(curr_table);
        oh_init_rpt(new_table);

        if (oh_add_resource(curr_table, rptentries, NULL, 0))
                return 1;
        
	if (oh_add_resource(new_table, rptentries + 1, NULL, 0))
                return 1;

	if (!rpt_diff(curr_table, new_table, NULL, &new_rdr, &gone_res, &gone_rdr))
		return 1;
	
        return 0;
}
