/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Kevin Gao <kevin.gao@intel.com>
 */

#include <stdio.h>
#include <hpitest.h>

#define INSERT_TIMEOUT_VALUE	1000

int main()
{
	SaHpiVersionT	version;
	SaHpiSessionIdT	session_id;
	SaHpiTimeoutT	timeout_new, timeout_old;
	SaErrorT	val;
	int             ret = HPI_TEST_PASS;

	val = saHpiInitialize(&version);
	if (val != SA_OK) {
		printf("  Function \"saHpiInitialize\" works abnormally!\n");
		printf("  Cannot initialize HPI!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}
	
	val = saHpiSessionOpen(SAHPI_DEFAULT_DOMAIN_ID, &session_id, NULL);
	if (val != SA_OK) {
		printf("  Function \"saHpiSessionOpen\" works abnormally!\n");	
		printf("  Cannot open the session!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out1;
	}

	val = saHpiResourcesDiscover(session_id);
        if (val != SA_OK) {
                printf("  Function \"saHpiResourcesDiscover\" works abnormally!\n");
                printf("Can not regenerate the RPT!\n");
                printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
                goto out2;
        }

	val = saHpiAutoInsertTimeoutGet(session_id, &timeout_old);
	if (val != SA_OK) {
		printf("  Funcition \"saHpiAutoInsertTimeoutGet\" works abnormally!\n");
		printf("  Cannot get get old insert timeout info!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out2;
	}

	val = saHpiAutoInsertTimeoutSet(session_id, INSERT_TIMEOUT_VALUE);
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set new timeout value failed!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out2;
	}

	val = saHpiAutoInsertTimeoutGet(session_id, &timeout_new);
	if (val != SA_OK) {
		printf("  Funcition \"saHpiAutoInsertTimeoutGet\" works abnormally!\n");
		printf("  Cannot get old insert timeout info!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out3;
	}

	if (timeout_new != INSERT_TIMEOUT_VALUE) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set timeout value is invalid!\n");
		ret = HPI_TEST_FAIL;
	}

out3:
	val = saHpiAutoInsertTimeoutSet(session_id, timeout_old);
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set old timeout value failed!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
	}

out2:
	val = saHpiSessionClose(session_id);
	if (val != SA_OK) {
		printf("  Function \"saHpiSessionClose\" works abnormally!\n");
		printf("  Cannot close the session\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
	}

out1:
	val = saHpiFinalize();
	if (val != SA_OK) {
		printf("  Function \"saHpiFinalize\" works abnormally!\n");
		printf("  Cannot cleanup HPI");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
	}

out:	
	return ret;	
}
