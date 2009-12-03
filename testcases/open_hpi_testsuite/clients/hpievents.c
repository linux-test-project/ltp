/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004, 2007
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Peter D Phan <pdphan@us.ibm.com>
 *      Steve Sherman <stevees@us.ibm.com>
 *	Renier Morales <renier@openhpi.org>
 *
 *     10/13/2004 kouzmich  changed -t option for infinite wait
 *			    added -d option for call saHpiDiscover after saHpiSubscribe
 *     11/17/2004 kouzmich  linux style and checking timeout error
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_clients.h>

#define OH_SVN_REV "$Revision: 1.7 $"

#include <unistd.h>

#define HPI_NSEC_PER_SEC 1000000000LL

int fdebug = 0;

static void Usage(char **argv)
{
	printf("Usage %s [-t <value> | SAHPI_TIMEOUT_BLOCK | BLOCK] [-d] [-x]\n",argv[0]);
	printf("	where:	-t <value> - wait <value> seconds for event;\n");
	printf("		-t SAHPI_TIMEOUT_BLOCK or BLOCK - infinite wait\n");
	printf("		-d - call saHpiDiscover() after saHpiSubscribe()\n");
	printf("		-x - displays eXtra debug messages\n");
}

int main(int argc, char **argv)
{
	int c, test_fail = 0, wait = 0;
	char *timeout_str= (char *)NULL;
	int do_discover_after_subscribe = 0;
	SaErrorT rv;
	SaHpiSessionIdT sessionid;
	SaHpiDomainInfoT domainInfo;
	SaHpiRptEntryT rptentry;
	SaHpiEntryIdT rptentryid;
	SaHpiEntryIdT nextrptentryid;
	SaHpiResourceIdT resourceid;
	SaHpiEventLogInfoT info;
	SaHpiRdrT rdr;
	SaHpiTimeoutT timeout; 
	SaHpiEventT event;

	memset(&rptentry, 0, sizeof(rptentry));

	oh_prog_version(argv[0], OH_SVN_REV);

	while ( (c = getopt( argc, argv,"t:xd?")) != EOF ) {
		switch(c) {
			case 't':
				timeout_str = optarg;
				break;
			case 'd': 
				do_discover_after_subscribe = 1; 
				break;
			case 'x': 
				fdebug = 1; 
				break;
			default:
				Usage(argv);
				exit(1);
		}
	}

	if (timeout_str != (char *)NULL) {
		if ((strcmp(timeout_str, "SAHPI_TIMEOUT_BLOCK") == 0) ||
		    (strcmp(timeout_str, "BLOCK") == 0)) {
			timeout = SAHPI_TIMEOUT_BLOCK;
		} else {
			wait = atoi(timeout_str);
                	timeout = (SaHpiTimeoutT)(wait * HPI_NSEC_PER_SEC);
		}
	} else
		timeout = (SaHpiTimeoutT) SAHPI_TIMEOUT_IMMEDIATE;

	printf("************** timeout:[%lld] ****************\n", timeout);    

	rv = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sessionid, NULL);
	if (rv != SA_OK) {
		if (rv == SA_ERR_HPI_ERROR) 
			printf("saHpiSessionOpen: error %d, SpiLibd not running\n", rv);
		else
			printf("saHpiSessionOpen: %s\n", oh_lookup_error(rv));
		exit(-1);
	}
 
	if (!do_discover_after_subscribe) {
        	if (fdebug) printf("saHpiDiscover\n");
        	rv = saHpiDiscover(sessionid);
        	if (rv != SA_OK) {
        		printf("saHpiDiscover: %s\n", oh_lookup_error(rv));
        		exit(-1);
        	}
        }
	
        if (fdebug) printf( "Subscribe to events\n");
        rv = saHpiSubscribe( sessionid );
	if (rv != SA_OK) {
		printf("saHpiSubscribe: %s\n", oh_lookup_error(rv));
		exit(-1);
	}

	if (do_discover_after_subscribe) {
		if (fdebug) printf("saHpiDiscover after saHpiSubscribe\n");
		rv = saHpiDiscover(sessionid);
		if (rv != SA_OK) {
			printf("saHpiDiscover after saHpiSubscribe: %s\n", oh_lookup_error(rv));
			exit(-1);
		}
	}

	rv = saHpiDomainInfoGet(sessionid, &domainInfo);

	if (fdebug) printf("saHpiDomainInfoGet %s\n", oh_lookup_error(rv));
	printf("DomainInfo: UpdateCount = %d, UpdateTime = %lx\n",
		domainInfo.RptUpdateCount, (unsigned long)domainInfo.RptUpdateTimestamp);

	/* walk the RPT list */
	rptentryid = SAHPI_FIRST_ENTRY;
	while ((rv == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY)) {       
		printf("**********************************************\n");

		rv = saHpiRptEntryGet(sessionid, rptentryid, &nextrptentryid, &rptentry);
		if (fdebug) printf("saHpiRptEntryGet %s\n", oh_lookup_error(rv));

		if (rv == SA_OK) {
			resourceid = rptentry.ResourceId;
			if (fdebug)
				printf("RPT %x capabilities = %x\n", resourceid,
					rptentry.ResourceCapabilities);

			if ( (rptentry.ResourceCapabilities & SAHPI_CAPABILITY_EVENT_LOG)) {
				/* Using EventLogInfo to build up event queue - for now */
				rv = saHpiEventLogInfoGet(sessionid, resourceid, &info);
				if (fdebug) 
					printf("saHpiEventLogInfoGet %s\n", oh_lookup_error(rv));
				if (rv == SA_OK) 
					oh_print_eventloginfo(&info, 4);
			} else {
				if (fdebug) 
					printf("RPT doesn't have SEL\n");
			}

			rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0; 
			printf("rptentry[%d] tag: %s\n", resourceid, rptentry.ResourceTag.Data);

			rptentryid = nextrptentryid;
		}
		printf("**********************************************\n");
	}

	printf( "Go and get the event\n");
	while (1) {
		rdr.RdrType = SAHPI_NO_RECORD;
                rptentry.ResourceId = 0;

		rv = saHpiEventGet( sessionid, timeout, &event, &rdr, &rptentry, NULL);
		if (rv != SA_OK) {
			if (rv != SA_ERR_HPI_TIMEOUT) {
				printf("ERROR during EventGet : %s\n", oh_lookup_error(rv));
				test_fail = 1;
			} else {
				if (timeout == SAHPI_TIMEOUT_BLOCK) {
					printf("ERROR: Timeout while infinite wait\n");
					test_fail = 1;
				} else if (timeout != SAHPI_TIMEOUT_IMMEDIATE) {
					printf("ERROR: Time, %d seconds, expired waiting"
						" for event\n", wait);
					test_fail = 1;
				}
			}
			break;
		} else {
			if (rdr.RdrType != SAHPI_NO_RECORD)
				oh_print_event(&event, &rdr.Entity, 4);
                        else if (rptentry.ResourceId != 0)
                                oh_print_event(&event, &rptentry.ResourceEntity, 4);
                        else {
                                rptentryid = event.Source;
                                rv = saHpiRptEntryGet(sessionid, rptentryid, &nextrptentryid, &rptentry);
                                if(rv == SA_OK)
                                        oh_print_event(&event, &rptentry.ResourceEntity, 4);
                                else {
                                        printf("Wrong resource Id <%d> detected", event.Source);
                                        oh_print_event(&event, NULL, 4);
                                }
                        }
		}
	}

	if (test_fail == 0)
		printf("	Test PASS.\n");
	else
		printf("	Test FAILED.\n");

	/* Unsubscribe to future events */
	if (fdebug) printf( "Unsubscribe\n");
	rv = saHpiUnsubscribe( sessionid );

	rv = saHpiSessionClose(sessionid);
        
	return(0);
}
 
/* end hpigetevents.c */
