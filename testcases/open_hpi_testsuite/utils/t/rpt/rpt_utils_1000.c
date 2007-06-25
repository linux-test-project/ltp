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
#include <stdio.h>
#include <sys/time.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <rpt_resources.h>

/**
 * main: Adds 100 resources to RPTable. Fetches the hundredth resource
 * and times how long it took to fetch it. Fails if it can't get the resource.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        guint i = 0;
        struct timeval start, end;
        SaHpiRptEntryT *tmpentry = NULL;

        for (i = 1; i <= 10000; i++) {
                rptentries[0].ResourceId = i;
                rptentries[0].ResourceEntity.Entry[0].EntityLocation = i;
                oh_add_resource(rptable, rptentries, NULL, 0);
        }

        gettimeofday(&start, NULL);
        /*printf("Started at %ld.%ld\n",start.tv_sec,start.tv_usec);*/
        if (!(tmpentry = oh_get_resource_by_ep(rptable, &(rptentries[0].ResourceEntity))))
                return 1;

        gettimeofday(&end, NULL);
        /*printf("Ended at %ld.%ld\n",end.tv_sec,end.tv_usec);*/
        printf("%ld.%ld seconds elapsed.\n",(long)(end.tv_sec - start.tv_sec),(long)(end.tv_usec - start.tv_usec));
        
        return 0;
}
