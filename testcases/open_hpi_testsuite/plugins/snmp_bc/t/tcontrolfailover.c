/* -*- linux-c -*-
 * 
 * (C) Copyright IBM Corp. 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *     Steve Sherman <stevees@us.ibm.com>
 */

#include <snmp_bc_plugin.h>
#include <sahpimacros.h>
#include <tsetup.h>

int main(int argc, char **argv) 
{
	int               testfail = 0;
        gchar             *rdr_name;
	SaErrorT          err, expected_err;
	SaHpiCtrlModeT    mode;
	SaHpiCtrlStateT   state;
	SaHpiEntityPathT  ep, vmm_ep;
	SaHpiRdrT         rdr;
        SaHpiRptEntryT    rpt;
        SaHpiSessionIdT   sessionid;

	vmm_ep.Entry[0].EntityType = SAHPI_ENT_SYS_MGMNT_MODULE;
	vmm_ep.Entry[0].EntityLocation = 0;
	vmm_ep.Entry[1].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
	vmm_ep.Entry[1].EntityLocation = 1;
	vmm_ep.Entry[2].EntityType = SAHPI_ENT_ROOT;
	vmm_ep.Entry[2].EntityLocation = 0;  

	rdr_name = "MM Failover Control";

	/***********************
	 * Find the VMM resource
	 ***********************/
	err = tsetup(&sessionid);
	if (err) {
		printf("Error! Cannot open session for test environment\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		return -1;
	}
	
	oh_init_ep(&ep);
	oh_concat_ep(&ep, &vmm_ep);

	err = tfind_resource_by_ep(&sessionid, &ep, SAHPI_FIRST_ENTRY, &rpt);
	if (err) {
		printf("Cannot find resource for test environment\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		err = tcleanup(&sessionid);
		return -1;
	}
	
	/**************************
	 * Find MM Failover control
	 **************************/
	err = tfind_rdr_by_name(&sessionid, rpt.ResourceId, rdr_name, &rdr);
	if (err) {
		printf("Cannot find resource for test environment\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		err = tcleanup(&sessionid);
		return -1;
	}

	/***************************
	 * Test: Write Invalid State    
	 ***************************/
	expected_err = SA_ERR_HPI_INVALID_REQUEST;
	state.Type = SAHPI_CTRL_TYPE_DIGITAL;
	state.StateUnion.Digital = SAHPI_CTRL_STATE_OFF;
	err = saHpiControlSet(sessionid, rpt.ResourceId, rdr.RdrTypeUnion.CtrlRec.Num,
			      SAHPI_CTRL_MODE_MANUAL, &state);
	checkstatus(err, expected_err, testfail);

	/**************************
	 * Test: Write Invalid Mode    
	 **************************/
	expected_err = SA_ERR_HPI_READ_ONLY;
	state.Type = SAHPI_CTRL_TYPE_DIGITAL;
	state.StateUnion.Digital = SAHPI_CTRL_STATE_PULSE_ON;
	err = saHpiControlSet(sessionid, rpt.ResourceId, rdr.RdrTypeUnion.CtrlRec.Num,
			      SAHPI_CTRL_MODE_AUTO, &state);
	checkstatus(err, expected_err, testfail);

	/***************************
	 * Test: Write Valid State    
	 ***************************/
	expected_err = SA_OK;
	state.Type = SAHPI_CTRL_TYPE_DIGITAL;
	state.StateUnion.Digital = SAHPI_CTRL_STATE_PULSE_ON;
	err = saHpiControlSet(sessionid, rpt.ResourceId, rdr.RdrTypeUnion.CtrlRec.Num,
			      SAHPI_CTRL_MODE_MANUAL, &state);
	checkstatus(err, expected_err, testfail);

	
	/***********************
	 * Test: Read State/Mode
	 ***********************/
	err = saHpiControlGet(sessionid, rpt.ResourceId, rdr.RdrTypeUnion.CtrlRec.Num, &mode, &state);
	if (mode != SAHPI_CTRL_MODE_MANUAL || state.StateUnion.Digital != SAHPI_CTRL_STATE_OFF) {
	  printf("  Error! Testcase failed. Line=%d\n", __LINE__);
	  printf("State=%s; Mode=%s\n", oh_lookup_ctrlmode(mode),
		 oh_lookup_ctrlstatedigital(state.StateUnion.Digital));
	  return -1;
	}

	/***************************
	 * Cleanup after all tests
	 ***************************/
	 err = tcleanup(&sessionid);
	 return testfail;
}

#include <tsetup.c>
