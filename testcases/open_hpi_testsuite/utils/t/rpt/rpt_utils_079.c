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
 * main: Starts with an RPTable of 10 resources, adds 5 control rdr
 * to first resource. Fetches controls ++randomly by record id and compares
 * with original. A failed comparison means the test failed,
 * otherwise the test passed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        GSList *records = NULL;
        guint i = 0;

        for (i = 0; rptentries[i].ResourceId != 0; i++) {
                if (oh_add_resource(rptable, rptentries + i, NULL, 0))
                        return 1;
        }

        for (i = 0; i < 5; i++) {
                if (oh_add_rdr(rptable, SAHPI_FIRST_ENTRY, controls + i, NULL,0))
                        return 1;
                else
                        records = g_slist_append(records, controls + i);
        }

        for (; records; i--) {
                SaHpiRdrT *tmprdr = NULL, *randrdr = NULL;
                GSList *tmpnode = NULL;
                guint k = (guint) (((gfloat)i)*rand()/(RAND_MAX+1.0));

                tmpnode = g_slist_nth(records, k);
                randrdr = (SaHpiRdrT *)tmpnode->data;
                randrdr->RecordId =
                        oh_get_rdr_uid(randrdr->RdrType,
                                    randrdr->RdrTypeUnion.CtrlRec.Num);

                tmprdr = oh_get_rdr_by_id(rptable, SAHPI_FIRST_ENTRY,
                                          randrdr->RecordId);

                if (!tmprdr ||
                    memcmp(randrdr, tmprdr, sizeof(SaHpiRdrT)))
                        return 1;
                else {
                        records = g_slist_remove_link(records, tmpnode);
                        g_slist_free_1(tmpnode);
                }
        }

        return 0;
}
