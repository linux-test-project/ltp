/* -*- linux-c -*-
 *
 *
 * (C) Copyright IBM Corp 2003,2004
 * Authors:  
 *     Peter D.Phan pdphan@users.sourceforge.net
 *  
 *     01/13/2004 pdphan  module created
 *			  Change clock for event log on IBM Blade Center E.
 *     03/10/2004 pdphan  Remove reference to IBM Blade Center.
 *     10/12/2004 kouzmich  porting to HPI B.
 *			    check month, day, year, hours, minutes and seconds
 *			    for correctness
 */
/*
Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

  a.. Redistributions of source code must retain the above copyright notice, 
      this list of conditions and the following disclaimer. 
  b.. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation 
      and/or other materials provided with the distribution. 
  c.. Neither the name of Intel Corporation nor the names of its contributors 
      may be used to endorse or promote products derived from this software 
      without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_clients.h>

#define OH_SVN_REV "$Revision: 1.6 $"

int fdebug = 0;
int findate = 0;
int fintime = 0;

static void usage(char **argv)
{
	printf("Usage\n\t%s -d mm/dd/yyyy -t HH:MM:SS [-x]\n",argv[0]);
	printf("\twhere -d date in mm/dd/yyyy format\n");
	printf("\t      -t time of day in 24-hr format\n");
	printf("\t      -x displays eXtra debug messages\n");

	return;
}

int main(int argc, char **argv)
{
	int c, month, day, year;
	char i_newdate[20];
	char i_newtime[20];
	int day_array[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	struct tm  new_tm_time;
	SaErrorT rv;
	SaHpiSessionIdT sessionid;
	SaHpiDomainInfoT domainInfo;
	SaHpiRptEntryT rptentry;
	SaHpiEntryIdT rptentryid;
	SaHpiEntryIdT nextrptentryid;
	SaHpiResourceIdT resourceid;
	SaHpiEventLogEntryIdT entryid;
	SaHpiTimeT oldtime;
	SaHpiTimeT newtime;
	SaHpiTimeT readbacktime;
	SaHpiTextBufferT buffer;

	oh_prog_version(argv[0], OH_SVN_REV);
        
	while ( (c = getopt( argc, argv,"d:t:x")) != EOF )
	{
		switch(c) {
			case 'd': 
				findate = 1;
				strcpy(i_newdate, optarg);
				break;
			case 't': 
				fintime = 1;
				strcpy(i_newtime, optarg);
				break;
			case 'x':
				fdebug = 1;
				break;
			default:
				usage(argv);
				exit(1);
		}
	}
	
	if ( !findate || !fintime) {
		usage(argv);
		exit(1);
	}

	if (findate) {
		if (fdebug) printf("New date to be set: %s\n",i_newdate);
	        if (sscanf(i_newdate,"%2d/%2d/%4d", &month, &day, &year) < 8) {
			printf("%s: Invalid date\n", argv[0]);
		}
		/* check month, day and year for correctness */
		if ((month < 1) || (month > 12)) {
			printf("%s: Month out of range: (%d)\n", argv[0], month);
			usage(argv);
			exit(1);
		};
		if (year < 1900) {
			printf("%s: Year out of range: (%d)\n", argv[0], year);
			usage(argv);
			exit(1);
		};
		month--;
		if (month == 1) {
		/* if the given year is a leap year */
			if ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0))
				day_array[1] = 29;
		};
		if ((day < 1) || (day > day_array[month])) {
			printf("%s: Day out of range: (%d)\n", argv[0], day);
			usage(argv);
			exit(1);
		};

		new_tm_time.tm_mon = month;
		new_tm_time.tm_mday = day;
		new_tm_time.tm_year = year - 1900;
	}

	if (fintime) {
		if (fdebug)  printf("New time to be set:  %s\n",i_newtime);
	        if (sscanf(i_newtime,"%2d:%2d:%2d",
                  &new_tm_time.tm_hour, &new_tm_time.tm_min, &new_tm_time.tm_sec) < 6) {
			printf("%s: Invalid time\n", argv[0]);
		}
		/* check hours, minutes and seconds for correctness */
		if ((new_tm_time.tm_hour < 0) || (new_tm_time.tm_hour > 24)) {
			printf("%s: Hours out of range: (%d)\n", argv[0], new_tm_time.tm_hour);
			usage(argv);
			exit(1);
		};
		if ((new_tm_time.tm_min < 0) || (new_tm_time.tm_min > 60)) {
			printf("%s: Minutes out of range: (%d)\n", argv[0], new_tm_time.tm_min);
			usage(argv);
			exit(1);
		};
		if ((new_tm_time.tm_sec < 0) || (new_tm_time.tm_sec > 60)) {
			printf("%s: Seconds out of range: (%d)\n", argv[0], new_tm_time.tm_sec);
			usage(argv);
			exit(1);
		}
	}

	if (fdebug) printf("Values passed to mktime():\n\tmon %d\n\tday %d\n\tyear %d\n\tHH %d\n\tMM %d\n\tSS %d\n",
			new_tm_time.tm_mon, new_tm_time.tm_mday, new_tm_time.tm_year,
			new_tm_time.tm_hour, new_tm_time.tm_min, new_tm_time.tm_sec);

	newtime = (SaHpiTimeT) mktime(&new_tm_time) * 1000000000;
	if (fdebug) printf("New date and time in SaHpiTimeT %lli\n", (long long int)newtime);

	rv = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sessionid,NULL);
	if (rv != SA_OK) {
		if (rv == SA_ERR_HPI_ERROR) 
			printf("saHpiSessionOpen: error %d, SpiLibd not running\n",rv);
		else
			printf("saHpiSessionOpen: %s\n", oh_lookup_error(rv));
		exit(-1);
	}
 
	rv = saHpiDiscover(sessionid);
	if (fdebug) printf("saHpiDiscover %s\n", oh_lookup_error(rv));
	rv = saHpiDomainInfoGet(sessionid, &domainInfo);
	if (fdebug) printf("saHpiDomainInfoGet %s\n", oh_lookup_error(rv));
	printf("DomainInfo: RptUpdateCount = %d, RptUpdateTimestamp = %lx\n",
		domainInfo.RptUpdateCount, (unsigned long)domainInfo.RptUpdateTimestamp);
        
	/* walk the RPT list */
	rptentryid = SAHPI_FIRST_ENTRY;
	while ((rv == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY))
	{
                rv = saHpiRptEntryGet(sessionid,rptentryid,&nextrptentryid,&rptentry);
                if (fdebug) printf("saHpiRptEntryGet %s\n", oh_lookup_error(rv));
                if ((rv == SA_OK) && (rptentry.ResourceCapabilities & SAHPI_CAPABILITY_EVENT_LOG)) {
                        resourceid = rptentry.ResourceId;
                        if (fdebug) printf("RPT %x capabilities = %x\n", resourceid,
                                           rptentry.ResourceCapabilities);
			rv = saHpiEventLogTimeGet(sessionid, resourceid, &oldtime);
			oh_decode_time(oldtime, &buffer);
			printf ("\nCurrent event log time on HPI target: %s\n", buffer.Data);
			printf ("Setting new event log time on HPI target ...\n");
		 	rv = saHpiEventLogTimeSet(sessionid, resourceid, newtime);
			if (rv != SA_OK) 
			{
                		printf("saHpiEventLogTimeSet %s\n", oh_lookup_error(rv));
			}
			rv = saHpiEventLogTimeGet(sessionid, resourceid, &readbacktime);
			oh_decode_time(readbacktime, &buffer);
			printf ("Read-Back-Check event log time: %s\n", buffer.Data);

                }
                entryid = SAHPI_OLDEST_ENTRY;
                rptentryid = nextrptentryid;
	} 
        
        rv = saHpiSessionClose(sessionid);
        
        return(0);
}
 
/* end hpisettime.c */
