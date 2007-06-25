/* -*- linux-c -*-
 * 
 * (C) Copyright IBM Corp. 2004
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
//	gchar *BUSY_OID = ".1.3.6.1.4.1.2.3.51.2.4.4.1.7777";

	int testfail = 0;
	SaErrorT          err;
	SaErrorT expected_err;
        SaHpiRptEntryT rptentry;

        SaHpiSessionIdT sessionid;
	struct snmp_value value;
	struct snmp_bc_hnd custom_handle;
		
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

	memset (&custom_handle, 0, sizeof(struct snmp_bc_hnd));
	
	/************************** 
	 * Test 1: Test Busy Status
	 * Under simulation it is ok for custom_handle == NULL
	 **************************/
//	expected_err = SA_ERR_HPI_BUSY;
//	err = snmp_bc_snmp_get(&custom_handle, BUSY_OID, &value, SAHPI_FALSE); 
//	checkstatus(err, expected_err, testfail);
	
	/************************** 
	 * Test 2: Test Timeout Status
	 **************************/
//	custom_handle.handler_retries = SNMP_BC_MAX_SNMP_RETRY_ATTEMPTED;
//	expected_err = SA_ERR_HPI_NO_RESPONSE;

//	err = snmp_bc_snmp_get(&custom_handle, BUSY_OID, &value, SAHPI_TRUE); 
//	checkstatus(err, expected_err, testfail);
	
	/************************** 
	 * Test 3: Valid case
	 **************************/
	expected_err = SA_OK;

	err = snmp_bc_snmp_get(&custom_handle, SNMP_BC_DATETIME_OID, &value, SAHPI_TRUE); 
	checkstatus(err, expected_err, testfail);
	
	/************************** 
	 * Test 4: validate field
	 **************************/
	if (custom_handle.handler_retries != 0) {
		printf("Error! handler_retries does not get cleared after a successful snmp_get, Line=%d\n",  __LINE__);
		testfail = -1;
	}
		
	/************************** 
	 * Test 5: Test Busy Status, snmp_set
	 **************************/
//	expected_err = SA_ERR_HPI_BUSY;
//	err = snmp_bc_snmp_set(&custom_handle, BUSY_OID, value); 
//	checkstatus(err, expected_err, testfail);
	
	/************************** 
	 * Test 6: Test Timeout Status, snmp_set
	 **************************/
//	custom_handle.handler_retries = SNMP_BC_MAX_SNMP_RETRY_ATTEMPTED;
//	expected_err = SA_ERR_HPI_NO_RESPONSE;

//	err = snmp_bc_snmp_set(&custom_handle, BUSY_OID, value); 
//	checkstatus(err, expected_err, testfail);
	
	/************************** 
	 * Test 7: Valid case
	 **************************/
	expected_err = SA_OK;

	err = snmp_bc_snmp_set(&custom_handle, SNMP_BC_DATETIME_OID, value); 
	checkstatus(err, expected_err, testfail);
	
	/************************** 
	 * Test 8: validate field
	 **************************/
	if (custom_handle.handler_retries != 0) {
		printf("Error! handler_retries does not get cleared after a successful snmp_set, Line=%d\n", __LINE__);
		testfail = -1;
	}

	/***************************
	 * Cleanup after all tests
	 ***************************/
	 err = tcleanup(&sessionid);
	 return testfail;

}

#include <tsetup.c>
