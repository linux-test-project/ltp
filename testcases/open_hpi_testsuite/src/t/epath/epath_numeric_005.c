/*      -*- linux-c -*-
 *
 * (C) Copyright FORCE Computers 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *     Thomas Kanngieser <thomas.kanngieser@fci.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <epath_utils.h>

int main (int argc, char **argv) {
	SaHpiEntityPathT  ep;
	const int MAX_STRING_SIZE = 512;
	gchar test_string[MAX_STRING_SIZE];
	gchar returned_string[MAX_STRING_SIZE];
	int   err;

        sprintf( test_string, "{%d,13}", SAHPI_ENT_SUBBOARD_CARRIER_BLADE + 1 );

	err = string2entitypath(test_string, &ep);
	if (err)
		return 1;

        err = entitypath2string(&ep, returned_string, MAX_STRING_SIZE);
        if (err < 0)
                return 1;

        if (strcmp(returned_string, test_string))
                return 1;

        return 0;
}
