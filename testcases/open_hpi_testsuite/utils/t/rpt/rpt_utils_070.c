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
 * main: Starting with an empty RPTable, adds 1 resource to it.
 * Checks rpt info to see if update count was updated, but it passes
 * NULL for a table.
 * If oh_get_rpt_info returns error, the test passes, otherwise it failed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        SaHpiUint32T update_count, update_count_new;
	SaHpiTimeT update_timestamp;

        update_count = rptable->update_count;
	if (oh_get_rpt_info(rptable, &update_count, &update_timestamp))
		return 1;
        
        if (oh_add_resource(rptable, rptentries, NULL, 0))
                return 1;

        if (!oh_get_rpt_info(NULL, &update_count_new, &update_timestamp))
		return 1;
		
        return 0;
}
