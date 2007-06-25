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
	int               testfail = 0;
	SaErrorT          err, expected_err;
 	SaHpiPowerStateT  state = 0;
	SaHpiResourceIdT  id = 0;
	SaHpiRptEntryT    rptentry;
        SaHpiSessionIdT   sessionid;
	
	/***************************************	 	 
	 * Find a resource with Power capability
	 ***************************************/
	err = tsetup(&sessionid);
	if (err != SA_OK) {
		printf("Error! Cannot open session\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		return -1;
	}
	err = tfind_resource(&sessionid, SAHPI_CAPABILITY_POWER, SAHPI_FIRST_ENTRY, &rptentry, SAHPI_TRUE);
	if (err != SA_OK) {
		printf("Cannot find a Power capable resource\n");
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

	err = saHpiResourcePowerStateGet(-1, id, &state);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourcePowerStateSet(-1, id, state);
	checkstatus(err, expected_err, testfail);

	/************************
	 * Test: Invalid resource
	 ************************/
	expected_err = SA_ERR_HPI_INVALID_RESOURCE;

	err = saHpiResourcePowerStateGet(sessionid, -1, &state);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourcePowerStateSet(sessionid, -1, state);
	checkstatus(err, expected_err, testfail);

	/**************************
	 * Test: Invalid parameters
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;

	err = saHpiResourcePowerStateGet(sessionid, id, NULL);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourcePowerStateSet(sessionid, id, -1);
	checkstatus(err, expected_err, testfail);

 	/*************************
	 * Test: Normal operations
	 *************************/
	expected_err = SA_OK;

	err = saHpiResourcePowerStateSet(sessionid, id, SAHPI_POWER_ON);
	checkstatus(err, expected_err, testfail);
	err = saHpiResourcePowerStateGet(sessionid, id, &state);
	checkstatus(err, expected_err, testfail);
	if (state != SAHPI_POWER_ON) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Current state = %s\n", oh_lookup_powerstate(state));
		return -1;
	}

	err = saHpiResourcePowerStateSet(sessionid, id, SAHPI_POWER_CYCLE);
	checkstatus(err, expected_err, testfail);
	err = saHpiResourcePowerStateGet(sessionid, id, &state);
	checkstatus(err, expected_err, testfail);
	if (state != SAHPI_POWER_ON) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Current state = %s\n", oh_lookup_powerstate(state));
		return -1;
	}

	err = saHpiResourcePowerStateSet(sessionid, id, SAHPI_POWER_OFF);
	checkstatus(err, expected_err, testfail);
#if 0
	err = saHpiResourcePowerStateGet(sessionid, id, &state);
	checkstatus(err, expected_err, testfail);
	if (state != SAHPI_POWER_OFF) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Current state = %s\n", oh_lookup_powerstate(state));
		return -1;
	}
#endif

	/*************************
	 * Cleanup after all tests
	 *************************/
	err = tcleanup(&sessionid);
	return testfail;
}

#include <tsetup.c>
