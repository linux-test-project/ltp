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
	SaHpiEventLogInfoT info;
		
	struct oh_handler_state l_handle;
	memset(&l_handle, 0, sizeof(struct oh_handler_state));

	/************************** 
	 * Test: Invalid handle
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	err = snmp_bc_get_sel_info(NULL, id, &info);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test: Invalid info space
	 * expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	 **************************/
	err = snmp_bc_get_sel_info(&l_handle, id, NULL);
	checkstatus(err, expected_err, testfail);

	return testfail;

}

#include <tsetup.c>
