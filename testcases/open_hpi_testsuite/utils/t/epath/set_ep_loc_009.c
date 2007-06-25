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

/**
 * oh_set_ep_location: Entity type not in entity path. 
 * OK but no changed results.
 **/
int main(int argc, char **argv)
{
	SaErrorT err;
        SaHpiEntityPathT ep = {{{SAHPI_ENT_FAN, 494949},{0}}};
        SaHpiEntityLocationT x = 6767;

	err = oh_set_ep_location(&ep, SAHPI_ENT_DISK_BLADE, x);
        if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
        }
        if (ep.Entry[0].EntityLocation != 494949) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        if (ep.Entry[0].EntityType != SAHPI_ENT_FAN) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }

        return 0;
}
