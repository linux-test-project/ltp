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
	SaHpiSelEntryIdT	prev_entry_id;
	SaHpiSelEntryIdT	next_entry_id;
	SaHpiSelEntryIdT	current_entry_id;
	SaHpiSelEntryT		eventlog_entry;
	SaHpiRdrT		rdr;
	SaHpiRptEntryT 		rpt_entry1;
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;

	val = saHpiEventLogEntryGet(session_id, SAHPI_DOMAIN_CONTROLLER_ID, 
			SAHPI_OLDEST_ENTRY, &prev_entry_id, &next_entry_id, 
			&eventlog_entry, &rdr, &rpt_entry1);
	if (val == SA_ERR_HPI_INVALID) 
		goto out;
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Retrieve the oldest event log entry failed!(Domain)\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}

	while (next_entry_id != SAHPI_NO_MORE_ENTRIES) {
		current_entry_id = next_entry_id;
		val = saHpiEventLogEntryGet(session_id, 
				SAHPI_DOMAIN_CONTROLLER_ID, current_entry_id, 
				&prev_entry_id, &next_entry_id, &eventlog_entry, 				&rdr, &rpt_entry1);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Retrieve the next event log entry failed!(Domain)\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}
	}

out:
	return ret;
}

int process_resource(SaHpiSessionIdT session_id, SaHpiRptEntryT rpt_entry, callback2_t func)
{
	SaHpiResourceIdT	resource_id = rpt_entry.ResourceId;
	SaHpiSelEntryIdT	prev_entry_id;
	SaHpiSelEntryIdT	next_entry_id;
	SaHpiSelEntryIdT	current_entry_id;
	SaHpiSelEntryT		eventlog_entry;
	SaHpiRdrT		rdr;
	SaHpiRptEntryT 		rpt_entry1;
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;

	if (rpt_entry.ResourceCapabilities & SAHPI_CAPABILITY_SEL) {
		val = saHpiEventLogEntryGet(session_id, resource_id,
				SAHPI_OLDEST_ENTRY, &prev_entry_id, 
				&next_entry_id, &eventlog_entry, 
				&rdr, &rpt_entry1);
		if (val == SA_ERR_HPI_INVALID)
			goto out;
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Retrieve the oldest event log entry failed!(Domain)\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}
		
		while (next_entry_id != SAHPI_NO_MORE_ENTRIES) {
			current_entry_id = next_entry_id;
			val = saHpiEventLogEntryGet(session_id, resource_id,
					current_entry_id, &prev_entry_id, 
					&next_entry_id, &eventlog_entry, 
					&rdr, &rpt_entry1);
			if (val != SA_OK) {
				printf("  Does not conform the expected behaviors!\n");
				printf("  Retrieve the next event log entry failed!(Domain)\n");
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
