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
#include <oh_utils.h>

int main (int argc, char **argv) {
	gchar *test_string;
	SaErrorT err, expected_err;
	SaHpiEntityPathT  ep;

        /* Invalid string */
        test_string = "{17string,11}";
	expected_err = SA_ERR_HPI_INVALID_DATA;

	err = oh_encode_entitypath(test_string, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

        return 0;
}
