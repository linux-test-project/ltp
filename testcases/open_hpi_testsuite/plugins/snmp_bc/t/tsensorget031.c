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
	int testfail = 0;
	SaErrorT          err;
	SaErrorT expected_err;
	SaHpiRptEntryT    rptentry;
	SaHpiResourceIdT  id;
        SaHpiSessionIdT sessionid;
	 
	SaHpiSensorNumT sid = 0;
	SaHpiBoolT enable;
													    
	/* *************************************	 	 
	 * Find a resource with NO Sensor type rdr
	 * ************************************* */
	err = tsetup(&sessionid);
	if (err != SA_OK) {
		printf("Error! Can not open session for test environment\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		return -1;
	}
	err = tfind_resource(&sessionid, SAHPI_CAPABILITY_SENSOR, SAHPI_FIRST_ENTRY, &rptentry, SAHPI_TRUE);
	if (err != SA_OK) {
		printf("Error! Can not find resources for test environment\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		err = tcleanup(&sessionid);
		return SA_OK;
	}

	id = rptentry.ResourceId;
	/**************************
	 * Test:invalid handler
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;
	err = snmp_bc_get_sensor_event_enable(NULL, id, sid, &enable);
	checkstatus(err, expected_err, testfail);

	/**************************
	 * Test: invalid data pointer
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;
	err = saHpiSensorEventEnableGet(sessionid, id, sid, NULL);
	checkstatus(err, expected_err, testfail);

	/**************************
	 * Test 32
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_RESOURCE;
	err = saHpiSensorEventEnableGet(sessionid, 5000, sid, &enable);
	checkstatus(err, expected_err, testfail);
	
	/***************************
	 * Cleanup after all tests
	 ***************************/
	 err = tcleanup(&sessionid);
	 return testfail;

}

#include <tsetup.c>
