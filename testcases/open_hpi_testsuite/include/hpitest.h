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

#ifndef HPI_TEST
#define HPI_TEST

#include <SaHpi.h>

#define HPI_TEST_PASS	0
#define HPI_TEST_FAIL	-1
#define HPI_TEST_UNKNOW	1

typedef int (*callback2_t)(SaHpiSessionIdT session_id, 
		SaHpiResourceIdT resource_id, SaHpiRdrT rdr);
typedef int (*callback_t)(SaHpiSessionIdT session_id, SaHpiRptEntryT rpt_entry,	
		callback2_t func);
typedef int (*callback3_t)(SaHpiSessionIdT session_id);

static inline const char * get_error_string(SaErrorT error)
{
	switch(error) {
		case SA_OK:
			return "SA_OK";
		case SA_ERR_HPI_ERROR:
			return "SA_ERR_HPI_ERROR";
		case SA_ERR_HPI_UNSUPPORTED_API:
			return "SA_ERR_UNSUPPORTED_API";
		case SA_ERR_HPI_BUSY:
			return "SA_ERR_HPI_BUSY";
		case SA_ERR_HPI_INVALID:
			return "SA_ERR_HPI_INVALID";
		case SA_ERR_HPI_INVALID_CMD:
			return "SA_ERR_HPI_INVALID_CMD";
		case SA_ERR_HPI_TIMEOUT:
			return "SA_ERR_HPI_TIMEOUT";
		case SA_ERR_HPI_OUT_OF_SPACE:
			return "SA_ERR_HPI_OUT_OF_SPACE";
		case SA_ERR_HPI_DATA_TRUNCATED:
			return "SA_ERR_HPI_DATA_TRUNCATED";
		case SA_ERR_HPI_DATA_LEN_INVALID:
			return "SA_ERR_HPI_DATA_LEN_INVALID";
		case SA_ERR_HPI_DATA_EX_LIMITS:
			return "SA_ERR_HPI_DATA_EX_LIMITS";
		case SA_ERR_HPI_INVALID_PARAMS:
			return "SA_ERR_HPI_INVALID_PARAMS";
		case SA_ERR_HPI_INVALID_DATA:
			return "SA_ERR_HPI_INVALID_DATA";
		case SA_ERR_HPI_NOT_PRESENT:
			return "SA_ERR_HPI_NOT_PRESENT";
		case SA_ERR_HPI_INVALID_DATA_FIELD:
			return "SA_ERR_HPI_INVALID_DATA_FIELD";
		case SA_ERR_HPI_INVALID_SENSOR_CMD:
			return "SA_ERR_HPI_INVALID_SENSOR_CMD";
		case SA_ERR_HPI_NO_RESPONSE:
			return "SA_ERR_HPI_NO_RESPONSE";
		case SA_ERR_HPI_DUPLICATE:
			return "SA_ERR_HPI_DUPLICATE";
		case SA_ERR_HPI_INITIALIZING:
			return "SA_ERR_HPI_INITIALIZING";
		case SA_ERR_HPI_UNKNOWN:
			return "SA_ERR_HPI_UNKNOWN";
		case SA_ERR_HPI_INVALID_SESSION:
			return "SA_ERR_HPI_INVALID_SESSION";
		case SA_ERR_HPI_INVALID_RESOURCE:
			return "SA_ERR_HPI_INVALID_RESOURCE";
		case SA_ERR_HPI_INVALID_REQUEST:
			return "SA_ERR_HPI_INVALID_REQUEST";
		case SA_ERR_HPI_ENTITY_NOT_PRESENT:
			return "SA_ERR_HPI_ENTITY_NOT_PRESENT";
		case SA_ERR_HPI_UNINITIALIZED:
			return "SA_ERR_HPI_UNINITIALIZED";
		default:
			return "(invalid error code)";
	}
}

static inline int do_resource(SaHpiSessionIdT session_id, 
		SaHpiRptEntryT rpt_entry, callback2_t func)
{
	SaHpiEntryIdT  	current_rdr;
	SaHpiEntryIdT  	next_rdr;
	SaHpiRdrT    	rdr;
	SaErrorT       	val;
	int            	ret = HPI_TEST_UNKNOW;
	int 		r;

	if (rpt_entry.ResourceCapabilities & SAHPI_CAPABILITY_RDR) {
		next_rdr = SAHPI_FIRST_ENTRY;
		while (next_rdr != SAHPI_LAST_ENTRY) {
			current_rdr = next_rdr;
			val = saHpiRdrGet(session_id, rpt_entry.ResourceId,
					current_rdr, &next_rdr, &rdr);
			if (val != SA_OK) {
				printf("  Function \"saHpiRdrGet\" works abnormally!\n");
				printf("  Cannot get the RDR table!\n");
				printf("  Return value: %s\n", get_error_string(val));
				ret = HPI_TEST_FAIL;
				goto out;
			}
			r = (*func)(session_id, rpt_entry.ResourceId, rdr);
			if (r != HPI_TEST_UNKNOW)
				ret = r;
			if (ret == HPI_TEST_FAIL)
				goto out;
		}
	}
out:
	return ret;
}
static inline int process_domain(SaHpiDomainIdT domain_id, callback_t func, callback2_t func2, callback3_t func3)
{
	SaHpiSessionIdT	session_id;
	SaHpiEntryIdT	next_entry_id, temp_id;
	SaHpiRptEntryT	rpt_entry;
	SaErrorT       	val;
	int            	ret = HPI_TEST_UNKNOW;
	int 		r;

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

	if (func3 != NULL) {
		ret = (*func3)(session_id);
		if (ret == HPI_TEST_FAIL)
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
		
		r = (*func)(session_id, rpt_entry, func2); 
		if (r != HPI_TEST_UNKNOW)
			ret = r;
		if (ret == HPI_TEST_FAIL)
			goto out1;

		if (rpt_entry.ResourceCapabilities & SAHPI_CAPABILITY_DOMAIN) {
			r = process_domain(rpt_entry.DomainId, func, func2, 
					func3);
			if (r != HPI_TEST_UNKNOW)
				ret = r;
			if (ret == HPI_TEST_FAIL)
				goto out1;
		}
	}
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


#endif
