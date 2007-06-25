/* -*- linux-c -*-
 * 
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *     Peter D Phan <pdphan@users.sourceforge.net>
 *     Steve Sherman <stevees@us.ibm.com>
 */

#include <snmp_bc_plugin.h>
#include <sahpimacros.h>
#include <tsetup.h>

int main(int argc, char **argv) 
{
	int                testfail = 0;
	SaErrorT           err, expected_err;
	SaHpiResetActionT  act = 0;
	SaHpiResourceIdT   id = 0;
	SaHpiRptEntryT     rptentry;				
        SaHpiSessionIdT    sessionid;
	
	/***************************************	 	 
	 * Find a resource with Reset capability
	 ***************************************/
	err = tsetup(&sessionid);
	if (err != SA_OK) {
		printf("Error! Cannot open session\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		return -1;
	}
	err = tfind_resource(&sessionid, SAHPI_CAPABILITY_RESET, SAHPI_FIRST_ENTRY, &rptentry, SAHPI_TRUE);
	if (err != SA_OK) {
		printf("Cannot find a Reset capable resource\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		err = tcleanup(&sessionid);
		return SA_OK;
	}
	
	id = rptentry.ResourceId;

#if 0
	printf("Found resource = %s\n", rptentry.ResourceTag.Data);
#endif

	/***********************
	 * Test: Invalid session
	 ***********************/
	expected_err = SA_ERR_HPI_INVALID_SESSION;

	err = saHpiResourceResetStateGet(-1, id, &act);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceResetStateSet(-1, id, act);
	checkstatus(err, expected_err, testfail);

	/************************
	 * Test: Invalid resource
	 ************************/
	expected_err = SA_ERR_HPI_INVALID_RESOURCE;

	err = saHpiResourceResetStateGet(sessionid, -1, &act);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceResetStateSet(sessionid, -1, act);
	checkstatus(err, expected_err, testfail);

	/**************************
	 * Test: Invalid parameters
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;

	err = saHpiResourceResetStateGet(sessionid, id, NULL);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceResetStateSet(sessionid, id, -1);
	checkstatus(err, expected_err, testfail);

	/***********************
	 * Test: Invalid command
	 ***********************/
	expected_err = SA_ERR_HPI_INVALID_CMD;

	err = saHpiResourceResetStateSet(sessionid, id, SAHPI_RESET_ASSERT);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceResetStateSet(sessionid, id, SAHPI_RESET_DEASSERT);
	checkstatus(err, expected_err, testfail);

	/************************* 
	 * Test: Normal operations
	 *************************/
	expected_err = SA_OK;

	act = SAHPI_COLD_RESET;
	err = saHpiResourceResetStateSet(sessionid, id, act);
	checkstatus(err, expected_err, testfail);
	err = saHpiResourceResetStateGet(sessionid, id, &act);
	checkstatus(err, expected_err, testfail);
	if (act != SAHPI_RESET_DEASSERT) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Current state = %s\n", oh_lookup_resetaction(act));
		return -1;
	}
	
	act = SAHPI_WARM_RESET;
	err = saHpiResourceResetStateSet(sessionid, id, act);
	checkstatus(err, expected_err, testfail);
	err = saHpiResourceResetStateGet(sessionid, id, &act);
	checkstatus(err, expected_err, testfail);
	if (act != SAHPI_RESET_DEASSERT) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Current state = %s\n", oh_lookup_resetaction(act));
		return -1;
	}

	/*************************
	 * Cleanup after all tests
	 *************************/
	err = tcleanup(&sessionid);
	return testfail;
}

#include <tsetup.c>
