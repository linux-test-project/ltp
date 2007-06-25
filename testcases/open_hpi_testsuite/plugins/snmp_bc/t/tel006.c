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
#include <oh_session.h>
#include <oh_domain.h>
#include <oh_plugin.h>
#include <tsetup.h>

int main(int argc, char **argv) 
{

	/* ************************
	 * Local variables
	 * ***********************/	 	 
	int testfail = 0;
	SaErrorT          err;
	SaErrorT expected_err;
					
	SaHpiEventLogEntryIdT current = 0;
	SaHpiEventLogEntryIdT previd;
	SaHpiEventLogEntryIdT nextid;
	SaHpiEventLogEntryT   entry;
	SaHpiRdrT             rdr;
	SaHpiRptEntryT        rptentry;

	SaHpiResourceIdT  id;
//	DECLARE_HANDLE();
		
        SaHpiSessionIdT sessionid;			
        SaHpiDomainIdT did;
        struct oh_handler *h = NULL;
        struct oh_domain *d = NULL;
        unsigned int *hid = NULL;
	struct oh_handler_state *handle;	
		
	/* *************************************	 	 
	 * Find a resource with EventLog type rdr
	 * ************************************* */
	err = tsetup(&sessionid);
	if (err != SA_OK) {
		printf("Error! Can not open session for test environment\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		return -1;
	}
		
	err = tfind_resource(&sessionid, SAHPI_CAPABILITY_EVENT_LOG, SAHPI_FIRST_ENTRY, &rptentry, SAHPI_TRUE);
	if (err != SA_OK) {
		printf("Can not find a control resource for test environment\n");
		printf("       File=%s, Line=%d\n", __FILE__, __LINE__);
		err = tcleanup(&sessionid);
		return SA_OK;
	}
	
	id = rptentry.ResourceId;
	INIT_HANDLE(did, d, hid, h, handle);
	
#if 0
	did = oh_get_session_domain(sessionid);
	d = oh_get_domain(did);	
        hid = oh_get_resource_data(&(d->rpt), id);
        h = oh_get_handler(*hid);
	handle = (struct oh_handler_state *) h->hnd;
#endif

	/************************** 
	 * Test: NULL EventLog cache 
	 **************************/
	handle->elcache = NULL;
	
	expected_err = SA_ERR_HPI_INTERNAL_ERROR;                   
	err = snmp_bc_get_sel_entry(handle, id, current, &previd, &nextid, &entry, &rdr, &rptentry);
	checkstatus(err, expected_err, testfail);

	/***************************
	 * Cleanup after all tests
	 ***************************/
	err = tcleanup(&sessionid);
	return testfail;

}

#include <tsetup.c>

