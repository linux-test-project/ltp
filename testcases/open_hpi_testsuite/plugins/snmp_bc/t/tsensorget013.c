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

	SaHpiResourceIdT  id;
        SaHpiSessionIdT sessionid;
        SaHpiRptEntryT    rptentry;
	SaHpiRdrT         rdr;
	SaHpiSensorNumT   dd_sid = 0;
	SaHpiEventStateT state;
	SaHpiSensorReadingT reading;
	SaHpiEntryIdT entryid;
	SaHpiEntryIdT nextentryid;
	SaHpiBoolT foundSensor;			
													    
	/* *************************************	 	 
	 * Find a resource with Sensor type rdr
	 * ************************************* */
	err = tsetup(&sessionid);
	if (err != SA_OK) {
		printf("Error! Can not open session for test environment\n");
		printf("      File=%s, Line=%d\n", __FILE__, __LINE__);
		return -1;

	}
	err = tfind_resource(&sessionid,SAHPI_CAPABILITY_SENSOR,SAHPI_FIRST_ENTRY, &rptentry, SAHPI_TRUE);
	if (err != SA_OK) {
		printf("Error! Can not find resources for test environment\n");
		printf("      File=%s, Line=%d\n", __FILE__, __LINE__);
		err = tcleanup(&sessionid);
		return -1;
	}

	id = rptentry.ResourceId;
	/************************** 
	 * Test: find a sensor with desired property
	 **************************/
	entryid = SAHPI_FIRST_ENTRY;
	foundSensor = SAHPI_FALSE;			
	do {
		err = saHpiRdrGet(sessionid,id,entryid,&nextentryid, &rdr);
		if (err == SA_OK)
		{
			if ((rdr.RdrType == SAHPI_SENSOR_RDR) &&
				(rdr.RdrTypeUnion.SensorRec.DataFormat.IsSupported == SAHPI_TRUE))
			{
				foundSensor = SAHPI_TRUE;
				break;
														
			}
			entryid = nextentryid;
		}
	} while ((err == SA_OK) && (entryid != SAHPI_LAST_ENTRY)) ;

	if (!foundSensor) {
		err("Did not find desired resource for test\n");
		return(SA_OK);
	} else {
		dd_sid = rdr.RdrTypeUnion.SensorRec.Num; 
	}	

	/************************** 
	 * Test : Read sensor with NULL Reading area. State only 
	 **************************/
	expected_err = SA_OK;                   
	err = saHpiSensorReadingGet(sessionid, id, dd_sid, NULL, &state);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test:Read with NULL State area, Read Value only 
	 **************************/
	err = saHpiSensorReadingGet(sessionid, id, dd_sid, &reading, NULL);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test: Both Reading and State are NULL. ie checking for sensor existance
	 **************************/
	err = saHpiSensorReadingGet(sessionid, id, dd_sid, NULL, NULL);
	checkstatus(err, expected_err, testfail);
	
	/***************************
	 * Cleanup after all tests
	 ***************************/
	 err = tcleanup(&sessionid);
	 return testfail;

}

#include <tsetup.c>
