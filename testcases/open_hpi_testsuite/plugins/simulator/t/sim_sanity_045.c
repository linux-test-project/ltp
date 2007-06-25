/*      -*- linux-c -*-
*
*(C) Copyright IBM Corp. 2005-2006
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
* file and program are licensed under a BSD style license.  See
* the Copying file included with the OpenHPI distribution for
* full licensing terms.
*
* Authors:
*     W. David Ashley <dashley@us.ibm.com>
*/

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>

/**
 * Run a series of sanity tests on the simulator
 * Return 0 on success, otherwise return -1
 **/

int main(int argc, char **argv)
{
	SaHpiSessionIdT sid = 0;
	SaErrorT rc = SA_OK;
	int event_ctr = 0;
	SaHpiEventLogEntryT ele;
	SaHpiRdrT rdr;
	SaHpiRptEntryT rpte;
	SaHpiEventLogEntryIdT eleid, neleid, peleid;

	rc = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sid, NULL);
	if(rc != SA_OK)
		return -1;

	rc = saHpiSubscribe(sid);
	if(rc != SA_OK)
		return -1;

	rc = saHpiDiscover(sid);
	if (rc != SA_OK)
		return -1;

	/* count discovery events */
	eleid = neleid = SAHPI_OLDEST_ENTRY;
	while (rc == SA_OK && neleid != SAHPI_NO_MORE_ENTRIES) {
		rc = saHpiEventLogEntryGet(sid,
					   SAHPI_UNSPECIFIED_RESOURCE_ID,
					   eleid,
					   &peleid,
					   &neleid,
					   &ele,
					   &rdr,
					   &rpte);
		if (ele.Event.EventType == SAHPI_ET_RESOURCE)
			event_ctr++;

		eleid = neleid;
	}
	
	if (rc != SA_OK) {
		printf("SaHpiEventLogEntryGet returned %d.\n", rc);
		return -1;
	}
	/* A FRU device does NOT generate an ADD event. So our Hot Swap Disk
	 * Drive Bay resource will not generate an ADD event since it is marked
	 * as a FRU. If you change this, this test will see an additional event.
	 */
	if (event_ctr != 4) {
		printf("Incorrect number of events returned. Found %d events.\n",
		       event_ctr);
		return -1;
	}

	return 0;
}
