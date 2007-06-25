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

/* oh_concat_ep: concatenate two 4 element entity path */
int main(int argc, char **argv)
{
	SaErrorT err;
        SaHpiEntityPathT ep1 = {{{SAHPI_ENT_ADD_IN_CARD,111},
                                 {SAHPI_ENT_FRONT_PANEL_BOARD,122},
                                 {SAHPI_ENT_BACK_PANEL_BOARD,133},
                                 {SAHPI_ENT_POWER_SYSTEM_BOARD,144},
                                 {SAHPI_ENT_ROOT,0}}};
        SaHpiEntityPathT ep2 = {{{SAHPI_ENT_DRIVE_BACKPLANE,155},
                                 {SAHPI_ENT_SYS_EXPANSION_BOARD,166},
                                 {SAHPI_ENT_OTHER_SYSTEM_BOARD,177},
                                 {SAHPI_ENT_PROCESSOR_BOARD,188},
                                 {SAHPI_ENT_ROOT,0}}};
        SaHpiEntityPathT ep3 = {{{SAHPI_ENT_ADD_IN_CARD,111},
                                 {SAHPI_ENT_FRONT_PANEL_BOARD,122},
                                 {SAHPI_ENT_BACK_PANEL_BOARD,133},
                                 {SAHPI_ENT_POWER_SYSTEM_BOARD,144},
                                 {SAHPI_ENT_DRIVE_BACKPLANE,155},
                                 {SAHPI_ENT_SYS_EXPANSION_BOARD,166},
                                 {SAHPI_ENT_OTHER_SYSTEM_BOARD,177},
                                 {SAHPI_ENT_PROCESSOR_BOARD,188},
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
