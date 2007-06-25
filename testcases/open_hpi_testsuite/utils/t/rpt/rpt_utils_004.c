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
#include <stdlib.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <rpt_resources.h>

/**
 * main: Starts with an RPTable of 10 resources and fetches
 * them randomly by the Entity Path and compares them against the original
 * resource. A failed comparison means the test failed,
 * otherwise the test passed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        guint i = 0, k = 0;
        GSList *resources = NULL;

        for (i = 0; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries + i, NULL, 0))
                        return 1;
                else
                        resources = g_slist_append(resources, rptentries + i);
        }

        for (; resources; i--) {
                SaHpiRptEntryT *randentry = NULL, *tmpentry = NULL;
                GSList *tmpnode = NULL;

                k = (guint) (((gfloat)i)*rand()/(RAND_MAX+1.0));
                tmpnode = g_slist_nth(resources, k);
                randentry = (SaHpiRptEntryT *)tmpnode->data;

                tmpentry =
                    oh_get_resource_by_ep(rptable, &(randentry->ResourceEntity));

                if (!tmpentry ||
                    memcmp(randentry, tmpentry, sizeof(SaHpiRptEntryT)))
                        return 1;
                else {
                        resources = g_slist_remove_link(resources, tmpnode);
                        g_slist_free_1(tmpnode);
                }
        }

        return 0;
}
