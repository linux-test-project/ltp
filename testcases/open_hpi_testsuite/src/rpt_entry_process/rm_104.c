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

#define TEST_STR	"Test Tag Components"
#define STR_LEN		20

int tag_cmp(SaHpiTextBufferT *tag, SaHpiTextBufferT *tag_new)
{
	if (tag->DataType == tag_new->DataType &&
	    tag->Language == tag_new->Language &&
	    tag->DataLength == tag_new->DataLength &&
	    !memcmp(tag->Data, tag_new->Data, tag->DataLength))
		return 0;
	else
		return -1;
}

int process_resource(SaHpiSessionIdT session_id, SaHpiRptEntryT rpt_entry, callback2_t funcm)
{
	SaHpiResourceIdT	resource_id;
	SaHpiTextBufferT        tag, tag_old;
	SaErrorT		val;
	int 			ret = HPI_TEST_PASS;
	
	resource_id = rpt_entry.ResourceId;
	tag_old = rpt_entry.ResourceTag;
	
	memset(&tag, 0, sizeof(tag));
	tag.DataType = SAHPI_TL_TYPE_BINARY;
	tag.Language = SAHPI_LANG_ENGLISH;
	tag.DataLength = STR_LEN;
	memcpy(tag.Data, TEST_STR, sizeof(tag.Data));
	val = saHpiResourceTagSet(session_id, resource_id, &tag);
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set new tag failed!\n");
		printf("  Return value: %s\n", get_error_string(val));
		val = HPI_TEST_FAIL;
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

	if (tag_cmp(&rpt_entry.ResourceTag, &tag)) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set tag of RPT function is invalid!\n");
		ret = HPI_TEST_FAIL;
	}

out1:
	val = saHpiResourceTagSet(session_id, resource_id, &tag_old);
	if (val != SA_OK) {
		printf("  Does not conform the expected behaviors!\n");
		printf("  Set old tag failed!\n");
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
