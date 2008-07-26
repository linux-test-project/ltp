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
	SaHpiResourceIdT  id = 0;
        SaHpiSessionIdT sessionid;
	SaHpiRptEntryT rptentry;
	SaHpiRdrT	rdr;	 
	SaHpiSensorNumT sid = 0;
	SaHpiSensorThresholdsT thres;
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
		return SA_OK;
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
				(rdr.RdrTypeUnion.SensorRec.Category == SAHPI_EC_THRESHOLD) &&
				(rdr.RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold == 0))
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
		sid = rdr.RdrTypeUnion.SensorRec.Num; 
	}	


        /************************** 
	 * Test: setting to a non-writeable sensor
         **************************/
        expected_err = SA_ERR_HPI_INVALID_CMD;
	err = saHpiSensorThresholdsSet(sessionid, id, sid, &thres);
        checkstatus(err, expected_err, testfail);
											
	/***************************
	 * Cleanup after all tests
	 ***************************/
	err = tcleanup(&sessionid);
	return testfail;

}

#include <tsetup.c>
