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
	SaHpiResourceIdT	resource_id = rpt_entry.ResourceId;
	SaHpiEntryIdT   	current_rdr;
	SaHpiEntryIdT   	next_rdr;
	SaHpiRdrT       	rdr;
	SaErrorT		val;
	int 			ret = HPI_TEST_UNKNOW;

	if (rpt_entry.ResourceCapabilities & SAHPI_CAPABILITY_RDR) {
		ret = HPI_TEST_PASS;
		next_rdr = SAHPI_FIRST_ENTRY;
		while (next_rdr != SAHPI_LAST_ENTRY) {
			current_rdr = next_rdr;
			val = saHpiRdrGet(session_id, resource_id, current_rdr,
				       	&next_rdr, &rdr);
			if (val != SA_OK) {
				printf("  Does not conform the expected behaviors!\n");
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
