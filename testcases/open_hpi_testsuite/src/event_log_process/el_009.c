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

int process_domain_eventlog(SaHpiSessionIdT session_id)
{
	SaHpiBoolT      	enable_new, enable_old;
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;

	val = saHpiEventLogStateGet(session_id, SAHPI_DOMAIN_CONTROLLER_ID,
			&enable_old);
	if (val != SA_OK) {
		printf("  Funcition \"saHpiEventLogStateGet\" works abnormally!\n");
		printf("  Cannot get the event log state info!(Domain)\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}

	val = saHpiEventLogStateSet(session_id, SAHPI_DOMAIN_CONTROLLER_ID,
			SAHPI_FALSE);
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set event log state to TRUE failed!(Domain)\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}

	val = saHpiEventLogStateGet(session_id, SAHPI_DOMAIN_CONTROLLER_ID,
			&enable_new);
	if (val != SA_OK) {
		printf("  Funcition \"saHpiEventLogStateGet\" works abnormally!\n");
		printf("  Cannot get the event log state info!(Domain)\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out1;
	}

	if (enable_new != SAHPI_FALSE) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set event log state function is invalid!(Domain)\n");
		ret = HPI_TEST_FAIL;
	}

out1:
	val = saHpiEventLogStateSet(session_id, SAHPI_DOMAIN_CONTROLLER_ID,
			enable_old);
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set old event log state value failed!(Domain)\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
	}

out:
	return ret;
}

int process_resource(SaHpiSessionIdT session_id, SaHpiRptEntryT rpt_entry, callback2_t func)
{
	SaHpiResourceIdT	resource_id = rpt_entry.ResourceId;
	SaHpiBoolT      	enable_new, enable_old;
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;

	if (rpt_entry.ResourceCapabilities & SAHPI_CAPABILITY_SEL) {
		val = saHpiEventLogStateGet(session_id, resource_id, 
				&enable_old);
		if (val != SA_OK) {
			printf("  Funcition \"saHpiEventLogStateGet\" works abnormally!\n");
			printf("  Cannot get the event log state info!(Resource)\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}

		val = saHpiEventLogStateSet(session_id, resource_id, 
				SAHPI_FALSE);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Set event log state to TRUE failed!(Resource)\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}
	
		val = saHpiEventLogStateGet(session_id, resource_id,
				&enable_new);
		if (val != SA_OK) {
			printf("  Funcition \"saHpiEventLogStateGet\" works abnormally!\n");
			printf("  Cannot get the event log state info!(Resource)\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}

		if (enable_new != SAHPI_FALSE) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Set event log state function is invalid!(Resource)\n");
			ret = HPI_TEST_FAIL;
		}

out1:
		val = saHpiEventLogStateSet(session_id, resource_id,
				enable_old);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Set old event log state value failed!(Resource)\n");
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
	SaErrorT	val;
	int 		ret = HPI_TEST_PASS;

	val = saHpiInitialize(&version);
	if (val != SA_OK) {
		printf("  Function \"saHpiInitialize\" works abnormally!\n");
		printf("  Cannot initialize HPI!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}

	ret = process_domain(SAHPI_DEFAULT_DOMAIN_ID, process_resource, NULL,
			process_domain_eventlog);
	
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
