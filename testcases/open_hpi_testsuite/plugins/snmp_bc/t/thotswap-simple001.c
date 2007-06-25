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
	SaHpiHsIndicatorStateT  hs_ind_state = 0;
	SaHpiHsStateT           hs_state = 0;
	SaHpiResetActionT       act = 0;
	SaHpiResourceIdT        id = 0;
	SaHpiEntryIdT           rptid, next_rptid;
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

	/* Can't use tfind_resource(); Need to look for SAHPI_CAPABILITY_FRU and not
           SAHPI_CAPABILITY_MANAGED_HOTSWAP */
	rptid = SAHPI_FIRST_ENTRY;
	while ((err == SA_OK) && (rptid != SAHPI_LAST_ENTRY)) {
		err = saHpiRptEntryGet(sessionid, rptid, &next_rptid, &rptentry);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}
		
		if ((rptentry.ResourceCapabilities & SAHPI_CAPABILITY_FRU) &&
		   !(rptentry.ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
			id = rptentry.ResourceId;
			break;
		}
		else {
			rptid = next_rptid;
			continue;
		}
	}
	if (id == 0) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Cannot find a simple hotswap resource\n");
		return -1;
	}

#if 0
	printf("Found resource = %s\n", rptentry.ResourceTag.Data);
#endif

	/*************************** 
	 * Test: Capability checking
	 ***************************/
	expected_err = SA_ERR_HPI_CAPABILITY;

	err = saHpiHotSwapPolicyCancel(sessionid, id);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceActiveSet(sessionid, id);
	checkstatus(err, expected_err, testfail);

	err = saHpiResourceInactiveSet(sessionid, id);
	checkstatus(err, expected_err, testfail);

	err = saHpiAutoExtractTimeoutGet(sessionid, id, &timeout);
	checkstatus(err, expected_err, testfail);

	timeout = SAHPI_TIMEOUT_IMMEDIATE;
	err = saHpiAutoExtractTimeoutSet(sessionid, id, timeout);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapStateGet(sessionid, id, &hs_state);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapActionRequest(sessionid, id, act);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapIndicatorStateGet(sessionid, id, &hs_ind_state);
	checkstatus(err, expected_err, testfail);

	err = saHpiHotSwapIndicatorStateSet(sessionid, id, hs_ind_state);
	checkstatus(err, expected_err, testfail);

	/*************************
	 * Cleanup after all tests
	 *************************/
	err = tcleanup(&sessionid);
	return testfail;
}

#include <tsetup.c>
