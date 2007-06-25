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

/* oh_print_ep: NULL pointer testcase. */
int main(int argc, char **argv)
{
	int offsets = 0;
	SaErrorT err;
	SaErrorT expected_err = SA_ERR_HPI_INVALID_PARAMS;

	err = oh_print_ep(NULL, offsets);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

        return 0;
}
