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
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;

/* We can not insure whether the the function delete the event log entry
 * or only delete the information of event log entry.	*/	
	val = saHpiEventLogEntryDelete(session_id, SAHPI_DOMAIN_CONTROLLER_ID,
			SAHPI_NEWEST_ENTRY);
	if (val != SA_OK && val != SA_ERR_HPI_INVALID_CMD) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Delete entry the the system event log failed!(Domain)\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
	}

	return ret;
}

int process_resource(SaHpiSessionIdT session_id, SaHpiRptEntryT rpt_entry, callback2_t func)
{
	SaHpiResourceIdT	resource_id = rpt_entry.ResourceId;
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;

/* We can not insure whether the the function delete the event log entry
 * or only delete the information of event log entry.	*/
	if (rpt_entry.ResourceCapabilities & SAHPI_CAPABILITY_SEL) {
		val = saHpiEventLogEntryDelete(session_id, resource_id,
				SAHPI_NEWEST_ENTRY);
		if (val != SA_OK && val != SA_ERR_HPI_INVALID_CMD) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Delete entry the the system event log failed!(Resource)\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
		}	
	}

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
