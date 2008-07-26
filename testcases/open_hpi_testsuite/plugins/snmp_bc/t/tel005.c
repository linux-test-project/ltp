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
	SaHpiEventLogEntryIdT current = 0;
	SaHpiEventLogEntryIdT previd;
	SaHpiEventLogEntryIdT nextid;
	SaHpiEventLogEntryT   entry;
	SaHpiRdrT             rdr;
	SaHpiRptEntryT        rptentry;

	struct oh_handler_state l_handle;
	memset(&l_handle, 0, sizeof(struct oh_handler_state));
	
	/************************** 
	 * Test: NULL handle, rdr, rpt
	 **************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	err = snmp_bc_get_sel_entry(NULL, id, current, &previd, &nextid, &entry, NULL, NULL);
	checkstatus(err, expected_err,testfail);

	/************************** 
	 * Test: NULL previd   
	 * expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	 **************************/
	err = snmp_bc_get_sel_entry(&l_handle, id, current,
		       	          NULL, &nextid, &entry, NULL, NULL);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test: NULL nextid   
	 * expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	 **************************/
	err = snmp_bc_get_sel_entry(&l_handle, id, current,
		       	          &previd, NULL, &entry, NULL, NULL);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test: NULL nextid   
	 * expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	 **************************/
	err = snmp_bc_get_sel_entry(&l_handle, id, current,
		       	          &previd, &nextid, NULL, NULL, NULL);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test: NULL handle   
	 * expected_err = SA_ERR_HPI_INVALID_PARAMS;                   
	 **************************/
	err = snmp_bc_get_sel_entry(NULL, id, current,
		       	          &previd, &nextid, &entry, &rdr, &rptentry);
	checkstatus(err, expected_err, testfail);

	return testfail;

}

#include <tsetup.c>
