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
#include <unistd.h>
#include <hpitest.h>

int process_resource(SaHpiSessionIdT session_id, SaHpiRptEntryT rpt_entry, callback2_t func)
{
	SaHpiResourceIdT	resource_id;
	SaHpiHsStateT   	state, state_new;
	SaErrorT		val;
	int 			ret = HPI_TEST_UNKNOW;
	

	if (rpt_entry.ResourceCapabilities & 
				SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
		resource_id = rpt_entry.ResourceId;

		val = saHpiHotSwapStateGet(session_id, resource_id, &state);
		if (val != SA_OK) {
			printf("  Function \"saHpiHotSwapStateGet\" works abnormally!\n");
			printf("  Cannot get the hotswap state!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}

		if (state == SAHPI_HS_STATE_INACTIVE) {		
			val = saHpiHotSwapActionRequest(session_id, resource_id,
					SAHPI_HS_ACTION_INSERTION);
			if (val != SA_OK) {
				printf("  Function \"saHpiHotSwapActionRequest\" works abnormally!\n");
				printf("  Cannot change hotswap state!\n");
				printf("  Return value: %s\n", get_error_string(val));
				ret = HPI_TEST_FAIL;
				goto out;
			}
			sleep(1);

			val = saHpiHotSwapStateGet(session_id, resource_id, 
					&state_new);
			if (val != SA_OK) {
				printf("  Function \"saHpiHotSwapStateGet\" works abnormally!\n");
				printf("  Cannot get the hotswap state!\n");
				printf("  Return value: %s\n", get_error_string(val));
				ret = HPI_TEST_FAIL;
				goto out;
			}
			
			if (state_new != SAHPI_HS_STATE_ACTIVE_HEALTHY 
			&&  state_new != SAHPI_HS_STATE_ACTIVE_UNHEALTHY) {
                                printf("The hotswap state is not changed!\n");
                                goto out;
                        }
		}
		else if (state != SAHPI_HS_STATE_ACTIVE_HEALTHY &&
			 state != SAHPI_HS_STATE_ACTIVE_UNHEALTHY)
			goto out;	/*We don't care the pending state */

		ret = HPI_TEST_PASS;
	
		val = saHpiHotSwapControlRequest(session_id, resource_id);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Function \"saHpiHotSwapControlRequest\" failed!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}

		val = saHpiHotSwapActionRequest(session_id, resource_id,
				SAHPI_HS_ACTION_EXTRACTION);
		if (val != SA_OK) {
			printf("  Function \"saHpiHotSwapActionRequest\" works abnormally!\n");
			printf("  Cannot change hotswap state!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}

		val = saHpiResourceInactiveSet(session_id, resource_id);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Function \"saHpiResourceInactiveSet\" failed\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
		}

		val = saHpiHotSwapActionRequest(session_id, resource_id,
				SAHPI_HS_ACTION_INSERTION);
		if (val != SA_OK) {
			printf("  Function \"saHpiHotSwapActionRequest\" works abnormally!\n");
			printf("  Cannot change hotswap state!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
		}

out1:
		if (state == SAHPI_HS_STATE_INACTIVE) {		
			val = saHpiHotSwapActionRequest(session_id, resource_id,
					SAHPI_HS_ACTION_EXTRACTION);
			if (val != SA_OK) {
				printf("  Function \"saHpiHotSwapActionRequest\" works abnormally!\n");
				printf("  Cannot change hotswap state!\n");
				printf("  Return value: %s\n", get_error_string(val));
				ret = HPI_TEST_FAIL;
				goto out;
			}
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
