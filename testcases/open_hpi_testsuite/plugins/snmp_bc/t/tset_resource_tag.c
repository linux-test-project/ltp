
/* -*- linux-c -*-
 * 
 * (C) Copyright IBM Corp. 2004, 2005
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

#define SAHPI_LANG_NONSENSE (SaHpiLanguageT)0xFFFFFFFF

int main(int argc, char **argv) 
{

	/* ************************
	 * Local variables
	 * ***********************/	 
	int testfail = 0;
	SaHpiResourceIdT  id;
	SaErrorT          err;
	SaErrorT expected_err;
	SaHpiTextBufferT tag;
        SaHpiRptEntryT rptentry;

//	DECLARE_HANDLE();		
        SaHpiSessionIdT sessionid;			
        SaHpiDomainIdT did;
        struct oh_handler *h = NULL;
        struct oh_domain *d = NULL;
        unsigned int *hid = NULL;
	struct oh_handler_state *handle;	
	 	 
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

	id = rptentry.ResourceId;
	INIT_HANDLE(did, d, hid, h, handle);
		
	oh_init_textbuffer(&tag);
	

	/************************** 
	 * Test 1: Invalid Tag 
	 **************************/
	tag.Language = SAHPI_LANG_NONSENSE;
	expected_err = SA_ERR_HPI_INVALID_PARAMS;
	err = snmp_bc_set_resource_tag(handle, id, &tag);
	checkstatus(err, expected_err, testfail);
	
	/************************** 
	 * Test 2: Invalid ResourceId
	 **************************/
	oh_init_textbuffer(&tag);
	expected_err = SA_ERR_HPI_INVALID_RESOURCE;

	err = snmp_bc_set_resource_tag(handle, 5000, &tag);
	checkstatus(err, expected_err, testfail);

	/************************** 
	 * Test 3: Valid case
	 **************************/
	oh_init_textbuffer(&tag);
	expected_err = SA_OK;
	
	err = saHpiResourceTagSet(sessionid, id, &tag);
	checkstatus(err, expected_err, testfail);

	/***************************
	 * Cleanup after all tests
	 ***************************/
	 err = tcleanup(&sessionid);
	 return testfail;

}

#include <tsetup.c>

