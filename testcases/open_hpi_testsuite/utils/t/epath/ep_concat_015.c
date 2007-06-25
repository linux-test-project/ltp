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
 * Author(s):
 *     Chris Chia <cchia@users.sf.net>
 *     Steve Sherman <stevees@us.ibm.com>
 */

#include <string.h>
#include <stdio.h>

#include <SaHpi.h>
#include <oh_utils.h>

/* oh_concat_ep: concatenate a 5 element entity path that has garbage 
   beyond end element with a 5 element. Garbage should be gone
   at end result. */
int main(int argc, char **argv)
{
	SaErrorT err;
        SaHpiEntityPathT ep1 = {{{SAHPI_ENT_GROUP,1},
                                 {SAHPI_ENT_REMOTE,2},
                                 {SAHPI_ENT_EXTERNAL_ENVIRONMENT,3},
                                 {SAHPI_ENT_BATTERY,4},
                                 {SAHPI_ENT_CHASSIS_SPECIFIC,5},
                                 {SAHPI_ENT_ROOT,0},
                                 {SAHPI_ENT_FAN,11},
                                 {SAHPI_ENT_RACK,12}}};
        SaHpiEntityPathT ep2 = {{{SAHPI_ENT_GROUP,101},
                                 {SAHPI_ENT_REMOTE,102},
                                 {SAHPI_ENT_EXTERNAL_ENVIRONMENT,103},
                                 {SAHPI_ENT_BATTERY,104},
                                 {SAHPI_ENT_CHASSIS_SPECIFIC,105},
                                 {SAHPI_ENT_ROOT,0}}};
        SaHpiEntityPathT ep3 = {{{SAHPI_ENT_GROUP,1},
                                 {SAHPI_ENT_REMOTE,2},
                                 {SAHPI_ENT_EXTERNAL_ENVIRONMENT,3},
                                 {SAHPI_ENT_BATTERY,4},
                                 {SAHPI_ENT_CHASSIS_SPECIFIC,5},
                                 {SAHPI_ENT_GROUP,101},
                                 {SAHPI_ENT_REMOTE,102},
                                 {SAHPI_ENT_EXTERNAL_ENVIRONMENT,103},
                                 {SAHPI_ENT_BATTERY,104},
                                 {SAHPI_ENT_CHASSIS_SPECIFIC,105},
                                 {SAHPI_ENT_ROOT,0}}};

	err = oh_concat_ep(&ep1, &ep2);
        if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
        }
        if (!oh_cmp_ep(&ep1, &ep3)) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }

        return 0;
}
