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
	int                    testfail = 0;
	SaErrorT               err, expected_err;
	SaHpiHsIndicatorStateT hs_ind_state = SAHPI_HS_INDICATOR_OFF;
	SaHpiHsStateT          hs_state = SAHPI_HS_STATE_INACTIVE;
	SaHpiHsActionT         act = 0;
	SaHpiResourceIdT       id = 0;
	SaHpiRptEntryT         rptentry;			
        SaHpiSessionIdT        sessionid;
	SaHpiTimeoutT          timeout = SAHPI_TIMEOUT_IMMEDIATE;

	/*****************************************
	 * Find a resource with hotswap capability
	 *****************************************/
	err = tsetup(&sessionid);
	if (err != SA_OK) {
		printf("Error! Cannot open session\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		return -1;
	}
	err = tfind_resource(&sessionid, SAHPI_CAPABILITY_FRU, SAHPI_FIRST_ENTRY, &rptentry, SAHPI_TRUE);
	if (err != SA_OK) {
		dbg("Cannot find a hotswap resource\n");
		dbg("       File=%s, Line=%d\n", __FILE__, __LINE__);
		err = tcleanup(&sessionid);
		return SA_OK;
	}
	
	id = rptentry.ResourceId;

	/****************************** 
	 * Test: Bad parameter checking
	 ******************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;

	err = saHpiAutoInsertTimeoutGet(sessionid, NULL);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoInsertTimeoutSet(sessionid, -5);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoExtractTimeoutGet(sessionid, id, NULL);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoExtractTimeoutSet(sessionid, id, -5);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapStateGet(sessionid, id, NULL);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapActionRequest(sessionid, id, -1);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapIndicatorStateGet(sessionid, id, NULL);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapIndicatorStateSet(sessionid, id, -1);
	checkstatus(err, expected_err, testfail);

	/******************************* 
	 * Test: Invalid session checking
	 *******************************/
	expected_err = SA_ERR_HPI_INVALID_SESSION;

	err = saHpiHotSwapPolicyCancel(-1, id);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceActiveSet(-1, id);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceInactiveSet(-1, id);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoInsertTimeoutGet(-1, &timeout);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoInsertTimeoutSet(-1, timeout);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoExtractTimeoutGet(-1, id, &timeout);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoExtractTimeoutSet(-1, id, timeout);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapStateGet(-1, id, &hs_state);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapActionRequest(-1, id, act);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapIndicatorStateGet(-1, id, &hs_ind_state);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapIndicatorStateSet(-1, id, hs_ind_state);
	checkstatus(err, expected_err, testfail);

	/**************************** 
	 * Test: Invalid RID checking
	 ****************************/
	expected_err = SA_ERR_HPI_INVALID_RESOURCE;

	err = saHpiHotSwapPolicyCancel(sessionid, -1);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceActiveSet(sessionid, -1);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceInactiveSet(sessionid, -1);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoExtractTimeoutGet(sessionid, -1, &timeout);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoExtractTimeoutSet(sessionid, -1, timeout);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapStateGet(sessionid, -1, &hs_state);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapActionRequest(sessionid, -1, act);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapIndicatorStateGet(sessionid, -1, &hs_ind_state);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapIndicatorStateSet(sessionid, -1, hs_ind_state);
	checkstatus(err, expected_err, testfail);

	/*************************
	 * Cleanup after all tests
	 *************************/
	err = tcleanup(&sessionid);
	return testfail;
}

#include <tsetup.c>
