/* -*- linux-c -*-
 * 
 * (C) Copyright IBM Corp. 2004, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *     Peter D Phan <pdphan@users.sourceforge.net>
 */


#include <snmp_bc_plugin.h>
#include <sahpimacros.h>
#include <tsetup.h>

int main(int argc, char **argv) 
{

	/* ************************
	 * Local variables
	 * ***********************/	 
	int testfail = 0;
	SaHpiResourceIdT  id;
	SaErrorT          err;
	SaErrorT expected_err;
	SaHpiSeverityT sev;
        SaHpiRptEntryT rptentry;
	 
//	DECLARE_HANDLE();		
        SaHpiSessionIdT sessionid;			
        SaHpiDomainIdT did;
        struct oh_handler *h = NULL;
        struct oh_domain *d = NULL;
        unsigned int *hid = NULL;
	struct oh_handler_state *handle;	
	 
	/* ************************	 	 
	 * Find a resource with Control type rdr
	 * ***********************/		
	err = tsetup(&sessionid);
	if (err != SA_OK) {
		printf("Error! Can not open session for test environment\n");
		printf("      File=%s, Line=%d\n", __FILE__, __LINE__);
		return -1;
	}
	err = tfind_resource(&sessionid, SAHPI_CAPABILITY_CONTROL, SAHPI_FIRST_ENTRY, &rptentry, SAHPI_TRUE);
	if (err != SA_OK) {
		printf("Error! Can not find resources for test environment\n");
		printf("      File=%s, Line=%d\n", __FILE__, __LINE__);
		err = tcleanup(&sessionid);
		return -1;
	}

	id = rptentry.ResourceId;
	INIT_HANDLE(did, d, hid, h, handle);	

	/************************** 
	 * Test 1: Invalid severity 
	 **************************/
	sev = 0xFE;
	expected_err = SA_ERR_HPI_INVALID_PARAMS;
	err = snmp_bc_set_resource_severity(handle, id, sev);
	checkstatus(err, expected_err, testfail);
	
	/************************** 
	 * Test 2: Invalid ResourceId
	 **************************/
	sev = SAHPI_INFORMATIONAL;
	expected_err = SA_ERR_HPI_INVALID_RESOURCE;

	err = snmp_bc_set_resource_severity(handle, 5000, sev);
	checkstatus(err, expected_err, testfail);
	
	/************************** 
	 * Test 3: Valid case
	 **************************/
	sev = SAHPI_INFORMATIONAL;
	expected_err = SA_OK;

	err = saHpiResourceSeveritySet(sessionid, id, sev);
	checkstatus(err, expected_err, testfail);

	/***************************
	 * Cleanup after all tests
	 ***************************/

	 err = tcleanup(&sessionid);
	 return testfail;

}

#include <tsetup.c>

