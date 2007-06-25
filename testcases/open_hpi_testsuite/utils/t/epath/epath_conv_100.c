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
 * Authors:
 *     Sean Dague <http://dague.net/sean>
 */

#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>

/**
 * main: 
 * epathstr -> epath test
 * 
 * Test if an entity path string is converted properly into an entity path.
 **/
int main(int argc, char **argv) 
{
        char *test_string = "{CHASSIS_SPECIFIC,97}{BOARD_SET_SPECIFIC,-5}";
	SaErrorT err;
        SaHpiEntityPathT ep; 
        
	err = oh_encode_entitypath(test_string, &ep);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}
         
        if (ep.Entry[0].EntityType != SAHPI_ENT_BOARD_SET_SPECIFIC) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received=%d; Expected=%d\n", ep.Entry[0].EntityType, SAHPI_ENT_BOARD_SET_SPECIFIC);
		return -1;
	}
        
        if (ep.Entry[0].EntityLocation != -5) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received=%d; Expected=%d\n", ep.Entry[0].EntityLocation, -5);
                return -1;
	}
        
        if (ep.Entry[1].EntityType != SAHPI_ENT_CHASSIS_SPECIFIC) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received=%d; Expected=%d\n", ep.Entry[0].EntityType, SAHPI_ENT_CHASSIS_SPECIFIC);
                return -1;
	}

	if (ep.Entry[1].EntityLocation != 97) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received=%d; Expected=%d\n", ep.Entry[0].EntityLocation, 97);
                return -1;
	}
        
        return 0;
}
