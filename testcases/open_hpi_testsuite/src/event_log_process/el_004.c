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
#include <string.h>
#include <hpitest.h>

#define	TEST_STR	"Event log test str"

int process_domain_eventlog(SaHpiSessionIdT session_id)
{
	SaHpiSelEntryIdT	prev_entry_id;
	SaHpiSelEntryIdT	next_entry_id;
	SaHpiSelEntryT		entry_get, entry_add;
	SaHpiRdrT		rdr;
	SaHpiRptEntryT 		rpt_entry;
	SaHpiBoolT              enable_old;
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;

	/* In order to make sure that the entry we add is the
	 * newest entry in eventlog, we should disable the
	 * event log state				      */
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
		printf("  Funcition \"saHpiEventLogStateSet\" works abnormally!\n");
		printf("  Set event log state to TRUE failed!(Domain)\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out1;
	}

	entry_add.Event.Source = SAHPI_UNSPECIFIED_RESOURCE_ID;
	entry_add.Event.EventType = SAHPI_ET_USER;
	entry_add.Event.Timestamp = SAHPI_TIME_UNSPECIFIED;
	entry_add.Event.Severity = SAHPI_OK;
	memcpy(entry_add.Event.EventDataUnion.UserEvent.UserEventData,
			TEST_STR, sizeof(TEST_STR));

	val = saHpiEventLogEntryAdd(session_id, 
			SAHPI_DOMAIN_CONTROLLER_ID, &entry_add);
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Add entry the the system event log failed!(Domain)\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out1;
	}

	val = saHpiEventLogEntryGet(session_id, 
			SAHPI_DOMAIN_CONTROLLER_ID,
			SAHPI_NEWEST_ENTRY, &prev_entry_id, 
			&next_entry_id,	&entry_get, &rdr, &rpt_entry);
	if (val != SA_OK) {
		printf("  Function \"saHpiEventLogEntryGet\" works abnormally\n");
		printf("  Cannot retrieve the specified event log entry!(Domain)\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out1;
	}
	
	if (memcmp(&entry_add.Event, &entry_get.Event, 
				sizeof(entry_add.Event))) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Add event log entry function is invalid!(Domain)\n");
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
	SaHpiSelEntryIdT	prev_entry_id;
	SaHpiSelEntryIdT	next_entry_id;
	SaHpiSelEntryT		entry_get, entry_add;
	SaHpiRdrT		rdr;
	SaHpiRptEntryT 		rpt_entry1;
	SaHpiBoolT		enable_old;
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;

	if (rpt_entry.ResourceCapabilities & SAHPI_CAPABILITY_SEL) {
		/* In order to make sure that the entry we add is the
		 * newest entry in eventlog, we should disable the
		 * event log state				      */
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
			goto out1;
		}

		
		entry_add.Timestamp = SAHPI_TIME_UNSPECIFIED;
		entry_add.Event.Source = SAHPI_UNSPECIFIED_RESOURCE_ID;
		entry_add.Event.EventType = SAHPI_ET_USER;
		entry_add.Event.Timestamp = SAHPI_TIME_UNSPECIFIED;
		entry_add.Event.Severity = SAHPI_OK;
		memcpy(entry_add.Event.EventDataUnion.UserEvent.UserEventData,
				TEST_STR, sizeof(TEST_STR));

		val = saHpiEventLogEntryAdd(session_id, resource_id, 
				&entry_add);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Add entry the the system event log failed!(Resource)\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}

		val = saHpiEventLogEntryGet(session_id, resource_id,
				SAHPI_NEWEST_ENTRY, &prev_entry_id, 
				&next_entry_id,	&entry_get, &rdr, &rpt_entry1);
		if (val != SA_OK) {
			printf("  Function \"saHpiEventLogEntryGet\" works abnormally\n");
			printf("  Cannot retrieve the specified event log entry!(Resource)\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}

		if (memcmp(&entry_add.Event, &entry_get.Event, 
					sizeof(entry_add.Event))) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Add event log entry function is invalid!(Resource)\n");
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
