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

int process_resource(SaHpiSessionIdT session_id, SaHpiRptEntryT rpt_entry, callback2_t func)
{
	SaHpiResourceIdT	resource_id;
	SaHpiTimeoutT   	timeout_new, timeout_old;
	SaErrorT		val;
	int 			ret = HPI_TEST_UNKNOW;
	

	if (rpt_entry.ResourceCapabilities & 
				SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
		ret = HPI_TEST_PASS;
		resource_id = rpt_entry.ResourceId;

		val = saHpiAutoExtractTimeoutGet(session_id, resource_id, 
				&timeout_old);
		if (val != SA_OK) {
			printf("  Funcition \"saHpiAutoExtractTimeoutGet\" works abnormally!\n");
			printf("  Cannot get get old insert timeout info!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}

		val = saHpiAutoExtractTimeoutSet(session_id, resource_id,
				SAHPI_TIMEOUT_IMMEDIATE);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Set new timeout value failed!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}

		val = saHpiAutoExtractTimeoutGet(session_id, resource_id, 
				&timeout_new);
		if (val != SA_OK) {
			printf("  Funcition \"saHpiAutoExtractTimeoutGet\" works abnormally!\n");
			printf("  Cannot get old insert timeout info!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}

		if (timeout_new != SAHPI_TIMEOUT_IMMEDIATE) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Set timeout value is invalid!\n");
			ret = HPI_TEST_FAIL;
		}

out1:
		val = saHpiAutoExtractTimeoutSet(session_id,resource_id, 
				timeout_old);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Set old timeout value failed!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
		}
	}
out:
	return ret;
}

int main()
{
	SaHpiVersionT 	version;
	SaErrorT        val;
	int             ret = HPI_TEST_PASS;
	
	val = saHpiInitialize(&version);
	if (val != SA_OK) {
		printf("  Function \"saHpiInitialize\" works abnormally!\n");
		printf("  Cannot initialize HPI!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}
		
	ret = process_domain(SAHPI_DEFAULT_DOMAIN_ID, process_resource, NULL,
			NULL);
	if (ret == HPI_TEST_UNKNOW) {
		printf("  No resource support hotswap in SAHPI_DEFAULT_DOMAIN_ID!\n");
		ret = HPI_TEST_FAIL;
	}
	
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
