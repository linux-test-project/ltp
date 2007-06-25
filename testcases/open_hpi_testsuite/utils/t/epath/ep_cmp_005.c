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

/* oh_cmp_ep: full element entity path testcase. */
int main(int argc, char **argv)
{
        int i;
        SaHpiEntityPathT ep1;
        SaHpiEntityPathT ep2;

        for (i=0; i<SAHPI_MAX_ENTITY_PATH; i++) {
                ep1.Entry[i].EntityType = SAHPI_ENT_IO_SUBBOARD;
                ep1.Entry[i].EntityLocation = i+10;
                ep2.Entry[i].EntityType = SAHPI_ENT_IO_SUBBOARD;
                ep2.Entry[i].EntityLocation = i+10;
        }
        if (!oh_cmp_ep(&ep1, &ep2)) {
 		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }

        return 0;
}
