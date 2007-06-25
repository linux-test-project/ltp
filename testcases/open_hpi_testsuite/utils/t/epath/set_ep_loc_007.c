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

/* oh_set_ep_location: Entity path has 4 elements, victim element
 * in middle. Only victim element's instance number changed */
int main(int argc, char **argv)
{
	SaErrorT err;
        SaHpiEntityPathT ep = {{{SAHPI_ENT_INTERCONNECT, 1515},
                                {SAHPI_ENT_PHYSICAL_SLOT, 2525},
                                {SAHPI_ENT_SUBRACK, 3535},
                                {SAHPI_ENT_IO_SUBBOARD, 4545},
                                {0}}};
        SaHpiEntityLocationT x = 98765;
         
        err = oh_set_ep_location(&ep, SAHPI_ENT_PHYSICAL_SLOT, x);
        if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
        }
        if (ep.Entry[1].EntityLocation != x) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        if (ep.Entry[1].EntityType != SAHPI_ENT_PHYSICAL_SLOT) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        if (ep.Entry[0].EntityLocation != 1515) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        if (ep.Entry[0].EntityType != SAHPI_ENT_INTERCONNECT) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        if (ep.Entry[2].EntityLocation != 3535) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        if (ep.Entry[2].EntityType != SAHPI_ENT_SUBRACK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        if (ep.Entry[3].EntityLocation != 4545) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        if (ep.Entry[3].EntityType != SAHPI_ENT_IO_SUBBOARD) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }

        return 0;
}
