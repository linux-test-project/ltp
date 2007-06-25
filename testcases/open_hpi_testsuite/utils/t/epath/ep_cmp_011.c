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

/* oh_cmp_ep: unidentical multi-element entity path testcase. */
int main(int argc, char **argv)
{
        SaHpiEntityPathT ep1 = {{{SAHPI_ENT_ADD_IN_CARD,51},
                                 {SAHPI_ENT_FRONT_PANEL_BOARD,52},
                                 {SAHPI_ENT_BACK_PANEL_BOARD,53},
                                 {SAHPI_ENT_POWER_SYSTEM_BOARD,54},
                                 {SAHPI_ENT_DRIVE_BACKPLANE,55},
                                 {SAHPI_ENT_SYS_EXPANSION_BOARD,56},
                                 {SAHPI_ENT_OTHER_SYSTEM_BOARD,57},
                                 {SAHPI_ENT_PROCESSOR_BOARD,58},
                                 {SAHPI_ENT_ROOT,0}}};
        SaHpiEntityPathT ep2 = {{{SAHPI_ENT_ADD_IN_CARD,51},
                                 {SAHPI_ENT_FRONT_PANEL_BOARD,52},
                                 {SAHPI_ENT_BACK_PANEL_BOARD,53},
                                 {SAHPI_ENT_POWER_SYSTEM_BOARD,54},
                                 {SAHPI_ENT_DRIVE_BACKPLANE,55},
                                 {SAHPI_ENT_SYS_EXPANSION_BOARD,56},
                                 {SAHPI_ENT_POWER_MODULE,57},
                                 {SAHPI_ENT_PROCESSOR_BOARD,58},
                                 {SAHPI_ENT_ROOT,0}}};

        if (oh_cmp_ep(&ep1, &ep2)) {
 		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }

        return 0;
}
