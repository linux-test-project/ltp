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

/* oh_set_ep_location: Dull entity path and victim element in the middle.
 * Only victim element's instance number changed. */
int main(int argc, char **argv)
{
        unsigned int    y = 77002;
        unsigned int    z = 3;
        unsigned int    i = 0;
	SaErrorT err;
        SaHpiEntityPathT ep;
        SaHpiEntityTypeT     w = SAHPI_ENT_SBC_BLADE;
        SaHpiEntityLocationT x = 56873;
         
        for (i=0; i<z; i++) {
                ep.Entry[i].EntityType = w;
                ep.Entry[i].EntityLocation = y;
        }
        ep.Entry[z].EntityType = SAHPI_ENT_FAN;
        ep.Entry[z].EntityLocation = z;
        for (i=z+1; i<SAHPI_MAX_ENTITY_PATH; i++) {
                ep.Entry[i].EntityType = w;
                ep.Entry[i].EntityLocation = y;
        }

	err = oh_set_ep_location(&ep, SAHPI_ENT_FAN, x);
        if (err) {
 		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}
        if (ep.Entry[z].EntityLocation != x) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        if (ep.Entry[z].EntityType != SAHPI_ENT_FAN) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
        }
        for ( i=0; i<z; i++ ) {
                if((ep.Entry[i].EntityType != w) ||
                   (ep.Entry[i].EntityLocation != y)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
                }
        }
        for ( i=z+1; i<SAHPI_MAX_ENTITY_PATH; i++ ) {
                if((ep.Entry[i].EntityType != w) ||
                   (ep.Entry[i].EntityLocation != y)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
                }
        }

        return 0;
}
