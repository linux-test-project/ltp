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

int domain_process(SaHpiDomainIdT domain_id)
{
	SaHpiSessionIdT	session_id;
	SaHpiEntryIdT	next_entry_id, temp_id;
	SaHpiRptEntryT	rpt_entry;
	SaHpiRptInfoT	rpt_info1, rpt_info2;
	SaErrorT       	val;
	int            	ret = HPI_TEST_PASS;

	val = saHpiSessionOpen(domain_id, &session_id, NULL);
	if (val != SA_OK) {
		printf("  Function \"saHpiSessionOpen\" works abnormally!\n");
		printf("  Cannot open the session!\n");	
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}

	val = saHpiResourcesDiscover(session_id);
	if (val != SA_OK) {
		printf("  Function \"saHpiResourcesDiscover\" works abnormally!\n");
		printf("Can not regenerate the RPT!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out1;
	}

	val = saHpiRptInfoGet(session_id, &rpt_info1);
	if (val != SA_OK) {
		printf("  Function \"saHpiRptInfoGet\" works abnormally!");
		printf("  Cannot get the information of RPT!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out1;
	}

	next_entry_id = SAHPI_FIRST_ENTRY;
	while (next_entry_id != SAHPI_LAST_ENTRY) {
		temp_id = next_entry_id;
		val = saHpiRptEntryGet(session_id, temp_id, 
				&next_entry_id, &rpt_entry);
		if (val != SA_OK) {
			printf("  Function \"saHpiRptEntryGet\" works abnormally!\n");
			printf("  Cannot retrieve the next entry of RPT!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}
		
		if (rpt_entry.ResourceCapabilities & SAHPI_CAPABILITY_DOMAIN) {
			ret = domain_process(rpt_entry.DomainId);
			if (ret == HPI_TEST_FAIL)
				goto out1;
		}
	}

	val = saHpiRptInfoGet(session_id, &rpt_info2);
	if (val != SA_OK) {
		printf("  Function \"saHpiRptInfoGet\" works abnormally!");
		printf("  Cannot get the information of RPT!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out1;
	}
	
	if (rpt_info1.UpdateCount == rpt_info2.UpdateCount) 
		printf("  RPT entry is not updated during the testing!\n");
	else
		printf("  RPT entry is updated during the testing!\n");

out1:
	val = saHpiSessionClose(session_id);
	if (val != SA_OK) {
		printf("  Function \"saHpiSessionClose\" works abnormally!\n");
		printf("  Cannot close the session\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
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
	
	ret = domain_process(SAHPI_DEFAULT_DOMAIN_ID);

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
