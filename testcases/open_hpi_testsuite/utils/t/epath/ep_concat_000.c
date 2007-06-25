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
 *     Renier Morales <renier@openhpi.org>
 *     Steve Sherman <stevees@us.ibm.com>
 */

#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>

/* oh_concat_ep: NULL parameter testcase. */
int main(int argc, char **argv)
{
        char *test_string = "{CHASSIS_SPECIFIC,89}{OPERATING_SYSTEM,46}";
	SaErrorT err, expected_err;
        SaHpiEntityPathT ep;

	err = oh_encode_entitypath(test_string, &ep);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	expected_err = SA_ERR_HPI_INVALID_PARAMS;
	err = oh_concat_ep(NULL, &ep);

	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

        return 0;
}
