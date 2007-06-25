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

/* oh_set_ep_location: Entity type not in entity path.
 * OK but nothing changed. */
int main(int argc, char **argv)
{
        unsigned int  y = 45321;
        unsigned int  i = 0;
	SaErrorT err;
        SaHpiEntityPathT ep;
        SaHpiEntityTypeT w = SAHPI_ENT_IO_BLADE;
        SaHpiEntityLocationT x = 56873;

        for (i=0; i<SAHPI_MAX_ENTITY_PATH; i++) {
                ep.Entry[i].EntityType = w;
                ep.Entry[i].EntityLocation = y+i;
        }

	err = oh_set_ep_location(&ep, SAHPI_ENT_IO_SUBBOARD, x);
        if (err) {
 		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
       }
        for (i=0; i<SAHPI_MAX_ENTITY_PATH; i++) {
                if ((ep.Entry[i].EntityType != w) ||
		    (ep.Entry[i].EntityLocation != y+i)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
                }
        }
	
        return 0;
}
