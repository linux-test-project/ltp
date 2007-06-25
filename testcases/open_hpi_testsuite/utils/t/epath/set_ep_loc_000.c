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

/* oh_set_ep_location test: NULL first parameter testcase. */
int main(int argc, char **argv)
{
	SaErrorT err, expected_err;

	expected_err = SA_ERR_HPI_INVALID_PARAMS;
	err = oh_set_ep_location(NULL, SAHPI_ENT_ROOT, 5);

	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

        return 0;
}
