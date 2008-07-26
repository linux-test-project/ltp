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
        SaHpiRptEntryT rptentry;
	SaHpiRdrT      rdr;
	SaHpiResourceIdT  id;
        SaHpiSessionIdT sessionid;
	SaHpiSensorNumT sid = 0;
	SaHpiSensorEventMaskActionT act   = SAHPI_SENS_ADD_EVENTS_TO_MASKS;
	SaHpiEventStateT assertMask       = SAHPI_ES_UPPER_MINOR;
	SaHpiEventStateT deassertMask      = SAHPI_ES_UPPER_CRIT;
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
				(rdr.RdrTypeUnion.SensorRec.EventCtrl != SAHPI_SEC_PER_EVENT))
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
	 * Test: Sensor with !SAHPI_SEC_PER_EVENT
	 **************************/						
	expected_err = SA_ERR_HPI_READ_ONLY;                   
	err = saHpiSensorEventMasksSet(sessionid, id, sid, act, assertMask, deassertMask);
	checkstatus(err, expected_err, testfail);

	/***************************
	 * Cleanup after all tests
	 ***************************/
	 err = tcleanup(&sessionid);
	 return testfail;

}

#include <tsetup.c>
