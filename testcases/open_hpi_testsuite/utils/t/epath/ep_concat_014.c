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

/* oh_concat_ep: concatenate two one less than full entity path */
int main(int argc, char **argv)
{
        int i;
	SaErrorT err;
        SaHpiEntityPathT ep1;
        SaHpiEntityPathT ep2;
        SaHpiEntityPathT ep3;

        for (i=0; i<SAHPI_MAX_ENTITY_PATH-1; i++) {
                ep1.Entry[i].EntityType = SAHPI_ENT_GROUP;
                ep1.Entry[i].EntityLocation = 202;
                ep2.Entry[i].EntityType = SAHPI_ENT_BATTERY;
                ep2.Entry[i].EntityLocation = 404;
                ep3.Entry[i].EntityType = SAHPI_ENT_GROUP;
                ep3.Entry[i].EntityLocation = 202;
        }
        ep1.Entry[SAHPI_MAX_ENTITY_PATH-1].EntityType = SAHPI_ENT_ROOT;
        ep1.Entry[SAHPI_MAX_ENTITY_PATH-1].EntityLocation = 0;
        ep2.Entry[SAHPI_MAX_ENTITY_PATH-1].EntityType = SAHPI_ENT_ROOT;
        ep2.Entry[SAHPI_MAX_ENTITY_PATH-1].EntityLocation = 0;
        ep3.Entry[SAHPI_MAX_ENTITY_PATH-1].EntityType = SAHPI_ENT_BATTERY;
        ep3.Entry[SAHPI_MAX_ENTITY_PATH-1].EntityLocation = 404;

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
