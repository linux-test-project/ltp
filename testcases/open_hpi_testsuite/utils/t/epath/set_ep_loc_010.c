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
 *     Steve Sherman <stevees@us.ibm.com>
 */

#include <string.h>
#include <stdio.h>

#include <SaHpi.h>
#include <oh_utils.h>

/* oh_set_ep_location: Entity type not in multi-element entity path.
 * OK but nothing changed */
int main(int argc, char **argv)
{
	SaErrorT err;
        SaHpiEntityPathT ep = {{{SAHPI_ENT_GROUP, 100},
                                {SAHPI_ENT_REMOTE, 200},
                                {SAHPI_ENT_EXTERNAL_ENVIRONMENT, 300},
                                {SAHPI_ENT_BATTERY, 400},
                                {SAHPI_ENT_CHASSIS_SPECIFIC, 500},
                                {SAHPI_ENT_BOARD_SET_SPECIFIC, 600},
                                {SAHPI_ENT_OEM_SYSINT_SPECIFIC, 700},
                                {SAHPI_ENT_ROOT, 800},
                                {SAHPI_ENT_RACK, 900},
                                {SAHPI_ENT_SUBRACK, 1000},
                                {0}}};
        SaHpiEntityLocationT x = 11000;

	err = oh_set_ep_location(&ep, SAHPI_ENT_FAN, x);
        if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
        }
        if((ep.Entry[0].EntityLocation != 100) ||
           (ep.Entry[0].EntityType != SAHPI_ENT_GROUP) ||
           (ep.Entry[1].EntityLocation != 200) ||
           (ep.Entry[1].EntityType != SAHPI_ENT_REMOTE) ||
           (ep.Entry[2].EntityLocation != 300) ||
           (ep.Entry[2].EntityType != SAHPI_ENT_EXTERNAL_ENVIRONMENT) ||
           (ep.Entry[3].EntityLocation != 400) ||
           (ep.Entry[3].EntityType != SAHPI_ENT_BATTERY) ||
           (ep.Entry[4].EntityLocation != 500) ||
           (ep.Entry[4].EntityType != SAHPI_ENT_CHASSIS_SPECIFIC) ||
           (ep.Entry[5].EntityLocation != 600) ||
           (ep.Entry[5].EntityType != SAHPI_ENT_BOARD_SET_SPECIFIC) ||
           (ep.Entry[6].EntityLocation != 700) ||
           (ep.Entry[6].EntityType != SAHPI_ENT_OEM_SYSINT_SPECIFIC) ||
           (ep.Entry[7].EntityLocation != 800) ||
           (ep.Entry[7].EntityType != SAHPI_ENT_ROOT) ||
           (ep.Entry[8].EntityLocation != 900) ||
           (ep.Entry[8].EntityType != SAHPI_ENT_RACK) ||
           (ep.Entry[9].EntityLocation != 1000) ||
           (ep.Entry[9].EntityType != SAHPI_ENT_SUBRACK)) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }

        return 0;
}
