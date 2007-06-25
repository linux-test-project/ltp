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

/* oh_print_ep: Multi-element numeric entity path testcase. */
int main(int argc, char **argv)
{
	int offsets = 1;
	SaErrorT err;
        SaHpiEntityPathT ep = {{{210,8},
                                {211,7},
                                {212,6},
                                {213,5},
                                {214,4},
                                {215,3},
                                {216,2},
                                {255,1},
				{SAHPI_ENT_ROOT,1},
                                {0}}};

	printf("Good multi-element numeric testcase. Offset\n");
	err = oh_print_ep(&ep, offsets);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	return 0;
}
