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

/* oh_set_ep_location: Entity path that has 2 elements testcase. */
int main(int argc, char **argv)
{
	SaErrorT err;
        SaHpiEntityPathT ep = {{{SAHPI_ENT_BIOS, 1},{SAHPI_ENT_UNKNOWN, 2},{0}}};
        SaHpiEntityLocationT x = 777;
         
        err = oh_set_ep_location(&ep, SAHPI_ENT_UNKNOWN, x);
        if (err) {
 		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}
        if (ep.Entry[1].EntityLocation != x) {
  		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
	}
        if (ep.Entry[1].EntityType != SAHPI_ENT_UNKNOWN) {
 		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
	}
        if (ep.Entry[0].EntityLocation != 1) {
 		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
	}
        if (ep.Entry[0].EntityType != SAHPI_ENT_BIOS) {
 		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
	}

        return 0;
}
