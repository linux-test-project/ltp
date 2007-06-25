/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copyright IBM Corp. 2004,2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *	Peter D. Phan <pdphan@users.sourceforge.net>
 *	Tariq Shureih <tariq.shureih@intel.com>
 *	David Judkovics <dmjudkov@us.ibm.com>
 *	Renier Morales <renier@openhpi.org>
 *
 *
 * Changes:
 *     11/03/2004  kouzmich   Fixed Bug #1057934
 *     djudkovi Copied from hpifru.c and modified for general use
 *     11/30/2006  renierm  Suppressed unneeded output from test
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <SaHpi.h> 
#include <oHpi.h> 
#include <oh_utils.h>

#define TOKEN "Hello America"
#define all_resources 255

/* 
 * Globals for this driver
 */
char progver[] = "0.2 HPI-B";
char progname[] = "hpiinjector";
int fdebug = 0;
int f_listall = 0;
int f_rpt     = 0;
int f_sensor  = 0;
int f_inv     = 0;
int f_ctrl    = 0;
int f_rdr     = 0;
int f_wdog    = 0;
int f_ann     = 0;
int f_overview = 0;
/* 
 * Main                
 */
int
main(int argc, char **argv)
{
	SaErrorT 	rv = SA_OK;
	
	SaHpiVersionT	hpiVer;
	SaHpiSessionIdT sessionid;
	SaHpiResourceIdT resourceid = all_resources;

	int c;
	    
	while ( (c = getopt( argc, argv,"adrsoiwcn:x?")) != EOF ) {
		switch(c) {
			case 'a': f_listall = 1; break;
			case 'c': f_ctrl    = 1; break;
			case 'd': f_rdr     = 1; break;
			case 'i': f_inv     = 1; break;
			case 'r': f_rpt     = 1; break;
			case 's': f_sensor  = 1; break; 
			case 'w': f_wdog    = 1; break;
			case 'o': f_overview = 1; break; 			 
			case 'n':
				if (optarg)
					resourceid = atoi(optarg);
				else 
					resourceid = all_resources;
				break;
			case 'x': fdebug = 1; break;
			default:
				printf("\n\tUsage: %s [-option]\n\n", progname);
				printf("\t      (No Option) Display all rpts and rdrs\n");
				printf("\t           -a     Display all rpts and rdrs\n");
				printf("\t           -c     Display only controls\n");
				printf("\t           -d     Display rdr records\n");
				printf("\t           -i     Display only inventories\n");
				printf("\t           -o     Display system overview: rpt & rdr headers\n");				
				printf("\t           -r     Display only rpts\n");
				printf("\t           -s     Display only sensors\n");
				printf("\t           -w     Display only watchdog\n");				
				printf("\t           -n     Select particular resource id to display\n");
				printf("\t                  (Used with [-cdirs] options)\n");
				printf("\t           -x     Display debug messages\n");
				printf("\n\n\n\n");
				exit(1);
		}
	}
 
	if (argc == 1) f_listall = 1;

	/* 
	 * House keeping:
	 * 	-- get (check?) hpi implementation version
	 *      -- open hpi session	
	 */
	if (fdebug) printf("saHpiVersionGet\n");
	hpiVer = saHpiVersionGet();

	if (fdebug) printf("saHpiSessionOpen\n");
        rv = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID,&sessionid,NULL);
	if (rv != SA_OK) {
		printf("saHpiSessionOpen returns %s\n",oh_lookup_error(rv));
		exit(-1);
	}
	if (fdebug)
	       	printf("saHpiSessionOpen returns with SessionId %d\n", sessionid);

	/*
	 * Resource discovery
	 */
	if (fdebug) printf("saHpiDiscover\n");
	rv = saHpiDiscover(sessionid);
	if (rv != SA_OK) {
		printf("saHpiDiscover returns %s\n",oh_lookup_error(rv));
		exit(-1);
	}


    SaHpiEventT event;
    SaHpiRptEntryT rpte;
//    oHpiRdrArrayT rdrs;

    memset(&event, 0, sizeof(SaHpiEventT));
    memset(&rpte, 0, sizeof(SaHpiRptEntryT));
//    memset(&rdrs, 0, sizeof(oHpiRdrArrayT));

    SaHpiRdrT  rdr;
    memset(&rdr, 0, sizeof(SaHpiRdrT));

    rdr.RecordId = 333;
    rdr.RdrType = SAHPI_WATCHDOG_RDR;

    int n = 0;
    for(n = 0; n < SAHPI_MAX_ENTITY_PATH; n++) {
            rdr.Entity.Entry[n].EntityLocation = 6;
            rdr.Entity.Entry[n].EntityType = SAHPI_ENT_ADD_IN_CARD;
    }

    rdr.IsFru = FALSE;

    rdr.RdrTypeUnion.WatchdogRec.Oem = 777;
    rdr.RdrTypeUnion.WatchdogRec.WatchdogNum = 888;


    rdr.IdString.DataType = SAHPI_TL_TYPE_TEXT;
    rdr.IdString.Language = SAHPI_LANG_ENGLISH;
    memcpy(rdr.IdString.Data, TOKEN, sizeof(TOKEN));
    rdr.IdString.Data[254] = (unsigned char)10;
    rdr.IdString.DataLength = sizeof(TOKEN);

//    int i = 0;
//    for(i = 0; i < MAX_RDR_ARRAY_LENGTH; i++) {
//            rdrs.Entry[i] = rdr;
//    }

    event.EventType=SAHPI_ET_HOTSWAP;
    event.Severity=SAHPI_CRITICAL;
    event.Source = 666;
    event.Timestamp=SAHPI_TIME_UNSPECIFIED;
    event.EventDataUnion.HotSwapEvent.HotSwapState=SAHPI_HS_STATE_EXTRACTION_PENDING;
    event.EventDataUnion.HotSwapEvent.PreviousHotSwapState=SAHPI_HS_STATE_ACTIVE;

    rv = oHpiInjectEvent(sessionid,
                         &event,
                         &rpte,
                         &rdr);


	rv = saHpiSessionClose(sessionid);
	
	exit(0);
}

 /* end hpiinjector.c */
