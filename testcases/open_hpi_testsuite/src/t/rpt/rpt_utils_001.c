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
 * main: Starting with an empty RPTable, adds 1 resource to it
 * and tries to fetch it by the entity path and compare it against
 * the original resource. A failed comparison means the test
 * failed, otherwise the test passed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        SaHpiRptEntryT *tmpentry = NULL;

        if (oh_add_resource(rptable, rptentries, NULL, 0))
                return 1;

        tmpentry = oh_get_resource_by_ep(rptable, &(rptentries[0].ResourceEntity));
        if (!tmpentry || memcmp(rptentries, tmpentry, sizeof(SaHpiRptEntryT)))
                return 1;

        return 0;
}
