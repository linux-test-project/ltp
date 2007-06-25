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

/* oh_concat_ep: concatenate a 4 and a 12 element entity path */
int main(int argc, char **argv)
{
	SaErrorT err;
        SaHpiEntityPathT ep1 = {{{SAHPI_ENT_POWER_UNIT,199},
                                 {SAHPI_ENT_CHASSIS_BACK_PANEL_BOARD,202},
                                 {SAHPI_ENT_SYSTEM_CHASSIS,211},
                                 {SAHPI_ENT_SUB_CHASSIS,222},
                                 {SAHPI_ENT_ROOT,0}}};
        SaHpiEntityPathT ep2 = {{{SAHPI_ENT_OTHER_CHASSIS_BOARD,233},
                                 {SAHPI_ENT_DISK_DRIVE_BAY,244},
                                 {SAHPI_ENT_PERIPHERAL_BAY_2,255},
                                 {SAHPI_ENT_DEVICE_BAY,255},
                                 {SAHPI_ENT_COOLING_DEVICE,277},
                                 {SAHPI_ENT_COOLING_UNIT,288},
                                 {SAHPI_ENT_INTERCONNECT,299},
                                 {SAHPI_ENT_MEMORY_DEVICE,303},
                                 {SAHPI_ENT_SYS_MGMNT_SOFTWARE,311},
                                 {SAHPI_ENT_BIOS,322},
                                 {SAHPI_ENT_OPERATING_SYSTEM,333},
                                 {SAHPI_ENT_SYSTEM_BUS,344},
                                 {SAHPI_ENT_ROOT,0}}};
        SaHpiEntityPathT ep3 = {{{SAHPI_ENT_POWER_UNIT,199},
                                 {SAHPI_ENT_CHASSIS_BACK_PANEL_BOARD,202},
                                 {SAHPI_ENT_SYSTEM_CHASSIS,211},
                                 {SAHPI_ENT_SUB_CHASSIS,222},
                                 {SAHPI_ENT_OTHER_CHASSIS_BOARD,233},
                                 {SAHPI_ENT_DISK_DRIVE_BAY,244},
                                 {SAHPI_ENT_PERIPHERAL_BAY_2,255},
                                 {SAHPI_ENT_DEVICE_BAY,255},
                                 {SAHPI_ENT_COOLING_DEVICE,277},
                                 {SAHPI_ENT_COOLING_UNIT,288},
                                 {SAHPI_ENT_INTERCONNECT,299},
                                 {SAHPI_ENT_MEMORY_DEVICE,303},
                                 {SAHPI_ENT_SYS_MGMNT_SOFTWARE,311},
                                 {SAHPI_ENT_BIOS,322},
                                 {SAHPI_ENT_OPERATING_SYSTEM,333},
                                 {SAHPI_ENT_SYSTEM_BUS,344}}};

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
