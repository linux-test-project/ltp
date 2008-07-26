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
	int                     testfail = 0;
	SaErrorT                err, expected_err;
        SaHpiCapabilitiesT      cap_mask;
	SaHpiHsIndicatorStateT  hs_ind_state = 0;
	SaHpiHsStateT           hs_state;
	SaHpiResourceIdT        id = 0;
	SaHpiRptEntryT          rptentry;
        SaHpiSessionIdT         sessionid;
	SaHpiTimeoutT           timeout;

	/************************************************
	 * Find a resource with simple hotswap capability
	 ************************************************/
	err = tsetup(&sessionid);
	if (err != SA_OK) {
		printf("Error! Cannot open session\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		return -1;
	}
	cap_mask = SAHPI_CAPABILITY_FRU | SAHPI_CAPABILITY_MANAGED_HOTSWAP;
	err = tfind_resource(&sessionid, cap_mask, SAHPI_FIRST_ENTRY, &rptentry, SAHPI_TRUE);
	if (err != SA_OK) {
		dbg("Cannot find a managed hotswap resource\n");
		dbg("       File=%s, Line=%d\n", __FILE__, __LINE__);
		err = tcleanup(&sessionid);
		return SA_OK;
	}
	
	id = rptentry.ResourceId;

#if 0
	printf("Found resource = %s\n", rptentry.ResourceTag.Data);
#endif

	/***************************************************************
	 * Test: Capability checking
         * No BladeCenter resource currently supports hotswap indicators      
	 ***************************************************************/
	expected_err = SA_ERR_HPI_CAPABILITY;

	err = saHpiHotSwapIndicatorStateGet(sessionid, id, &hs_ind_state);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapIndicatorStateSet(sessionid, id, hs_ind_state);
	checkstatus(err, expected_err, testfail);

	/*********************************************** 
	 * Test: Immediate Auto-insert/extraction policy
         * BladeCenter only supports immediate read-only
         * auto-insert/extraction policies. 
         ***********************************************/
	expected_err = SA_ERR_HPI_INVALID_REQUEST;

	err = saHpiHotSwapPolicyCancel(sessionid, id);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceActiveSet(sessionid, id);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceInactiveSet(sessionid, id);
	checkstatus(err, expected_err, testfail);

#if 0
	/* Currently defined by domain */
	/* Timeouts are READ-ONLY */
	expected_err = SA_ERR_HPI_READ_ONLY;

	err = saHpiAutoInsertTimeoutSet(sessionid, timeout);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoExtractTimeoutSet(sessionid, id, timeout);
	checkstatus(err, expected_err, testfail);
#endif

	/* Timeouts are IMMEDIATE */
	expected_err = SA_OK;

	err = saHpiAutoInsertTimeoutGet(sessionid, &timeout);
	checkstatus(err, expected_err, testfail);
	if (timeout != SAHPI_TIMEOUT_IMMEDIATE) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
	}

	err = saHpiAutoExtractTimeoutGet(sessionid, id, &timeout);
	checkstatus(err, expected_err, testfail);
	if (timeout != SAHPI_TIMEOUT_IMMEDIATE) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
	}

	/*************************
	 * Test: Normal Operations
         *************************/
	expected_err = SA_OK;

	/* Assume resource is in ACTIVE state; else can't test saHpiHotSwapActionRequest
           Simulator needs to be setup to ensure this */
	err = saHpiHotSwapStateGet(sessionid, id, &hs_state);
	checkstatus(err, expected_err, testfail);
	if (hs_state != SAHPI_HS_STATE_ACTIVE) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Hotswap resource needs to be in ACTIVE state\n");
		printf("  Current state = %s\n", oh_lookup_hsstate(hs_state));
		return -1;
	}

	err = saHpiHotSwapActionRequest(sessionid, id, SAHPI_HS_ACTION_EXTRACTION);
	checkstatus(err, expected_err, testfail);
#if 0
	err = saHpiHotSwapStateGet(sessionid, id, &hs_state);
	checkstatus(err, expected_err, testfail);
	if (hs_state != SAHPI_HS_STATE_INACTIVE) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Current state = %s\n", oh_lookup_hsstate(hs_state));
		return -1;
	}
	
	err = saHpiHotSwapActionRequest(sessionid, id, SAHPI_HS_ACTION_INSERTION);
	checkstatus(err, expected_err, testfail);
	err = saHpiHotSwapStateGet(sessionid, id, &hs_state);
	checkstatus(err, expected_err, testfail);
	if (hs_state != SAHPI_HS_STATE_ACTIVE) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Current state = %s\n", oh_lookup_hsstate(hs_state));
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
