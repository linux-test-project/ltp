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
	SaHpiSeverityT		severity_old, severity_new;	
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;
	
	resource_id = rpt_entry.ResourceId;
	severity_old = rpt_entry.ResourceSeverity;
	severity_new = SAHPI_CRITICAL;
	if (severity_old == SAHPI_CRITICAL)
		severity_new = SAHPI_MAJOR;
	
	val = saHpiResourceSeveritySet(session_id, resource_id, severity_new);
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set new severity failed!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}
	
	val = saHpiRptEntryGetByResourceId(session_id, rpt_entry.ResourceId, 
			&rpt_entry);
	if (val != SA_OK) {
		printf("  Function \"saHpiRptEntryGetByResourceId\" works abnoramally!\n");
		printf("  Cannot get RPT by its resource ID!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out1;
	}

	if (rpt_entry.ResourceSeverity != severity_new) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Invalid severity level of RPT!\n");
		ret = HPI_TEST_FAIL;
	}

out1:
	val = saHpiResourceSeveritySet(session_id, resource_id, severity_old);
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set old severity failed!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
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
			NULL);
	
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
