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

	SaHpiResourceIdT  id = 1;

	SaHpiSensorNumT sid = 1;
	SaHpiEventStateT state;
	SaHpiSensorReadingT reading;
			
	/* *************************************	 	 
	 * Find a resource with Sensor type rdr
	 * ************************************* */
	struct oh_handler_state handle;
	memset(&handle, 0, sizeof(struct oh_handler_state));

	/************************** 
	 * Test: snmp_bc_get_sensor_eventstate()
	 * NULL handle pointer
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	err = snmp_bc_get_sensor_eventstate(NULL, id, sid, &reading, &state);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test: NULL Reading pointer
	 * expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	 **************************/
	err = snmp_bc_get_sensor_eventstate(&handle, id, sid, NULL, &state);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test: NULL State pointer 
	 * expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	 **************************/
	err = snmp_bc_get_sensor_eventstate(&handle, id, sid, &reading, NULL);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test: Multiple NULL pointers 
	 * expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	 **************************/
	err = snmp_bc_get_sensor_eventstate(NULL , id, sid, NULL, NULL);
	checkstatus(err, expected_err, testfail);
	
	return testfail;

}

#include <tsetup.c>
