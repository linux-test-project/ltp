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
	struct tm time;
	struct oh_handler_state l_handle;

	memset(&time, 0, sizeof(struct tm));
	memset(&l_handle, 0, sizeof(struct oh_handler_state));
	/************************** 
	 * Test : Invalid handle
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	err = snmp_bc_get_sp_time(NULL, &time);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test : Invalid pointer to time struct
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	err = snmp_bc_get_sp_time(&l_handle, NULL);
	checkstatus(err, expected_err, testfail);

	/**************************
	 * Cleanup after all tests
	 ***************************/
	 return testfail;

}

#include <tsetup.c>
