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
 * main: resource must NOT have SAHPI_CAPABILITY_AGGREGATE_STATUS capability,
 * sensor num should be between SAHPI_STANDARD_SENSOR_MIN and
 * SAHPI_STANDARD_SENSOR_MAX and less than SENSOR_AGGREGATE_MAX.
 * With these conditions, oh_add_rdr is expected to return an error.
 * This is because for a sensor to have a num in the reserved range,
 * the resource must have SAHPI_CAPABILITY_AGGREGATE_STATUS capability
 * set.
 * If so, the test passes, otherwise it failed.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        RPTable *rptable = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(rptable);
        guint i;

        rptentries[0].ResourceCapabilities = rptentries[0].ResourceCapabilities & 0xFFFFDFFF;
        for (i = 0; rptentries[i].ResourceId; i++) {
                if (oh_add_resource(rptable, rptentries+i, NULL, 0))
                        return 1;
        }        

	sensors[0].RdrTypeUnion.SensorRec.Num = SAHPI_STANDARD_SENSOR_MIN;
        if (!oh_add_rdr(rptable, SAHPI_FIRST_ENTRY, sensors, NULL, 1))
                return 1;

        return 0;
}
