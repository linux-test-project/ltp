/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Andy Cress <arcress@users.sourceforge.net>
 */

/* This tool reads and enables the watchdog timer via HPI.
 * Note that there are other methods for doing this, and the
 * standard interface is for the driver to expose a /dev/watchdog
 * device interface.
 * WARNING: If you enable the watchdog, make sure you have something
 * set up to keep resetting the timer at regular intervals, or it
 * will reset your system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "SaHpi.h"

#define  uchar  unsigned char
char *progver  = "0.8";
char fdebug = 0;

static void
show_wdt(SaHpiWatchdogNumT  wdnum, SaHpiWatchdogT *wdt)
{
  int icount, pcount;
  icount = wdt->InitialCount /1000;        /*1000 msec = 1 sec*/
  pcount = wdt->PresentCount /1000;
  printf("Watchdog: Num=%d, Log=%d, Running=%d, TimerUse=%d, TimerAction=%d\n",
	wdnum,wdt->Log,wdt->Running,wdt->TimerUse,wdt->TimerAction);
  printf("          TimerUseExpFlags=%x, Timeout=%d sec, Counter=%d sec\n",
	wdt->TimerUseExpFlags, icount,pcount);
  printf("          PreTimerInterrupt=%d, PreTimeoutInterval=%d msec\n",
	wdt->PretimerInterrupt,wdt->PreTimeoutInterval);
  return;
}

int
main(int argc, char **argv)
{
  char c;
  SaErrorT rv;
  SaHpiVersionT hpiVer;
  SaHpiSessionIdT sessionid;
  SaHpiRptInfoT rptinfo;
  SaHpiRptEntryT rptentry;
  SaHpiEntryIdT rptentryid;
  SaHpiEntryIdT nextrptentryid;
  SaHpiResourceIdT resourceid;
  SaHpiWatchdogNumT  wdnum;
  SaHpiWatchdogT     wdt;
  int t = 0;
  char freset = 0;
  char fenable = 0;
  char fdisable = 0;

  printf("%s ver %s\n", argv[0],progver);
  while ( (c = getopt( argc, argv,"dert:x?")) != EOF )
     switch(c) {
	case 'r':       /* reset wdt */
		freset = 1;
                break;
	case 'e':       /* disable wdt */
		fenable = 1;
                break;
	case 'd':       /* disable wdt */
		fdisable = 1;
                break;
	case 't':       /* timeout (enable implied) */
		t = atoi(optarg);
		fenable = 1;
                break;
	case 'x': fdebug = 1;     break;  /* debug messages */
	default:
                printf("Usage: %s [-derx -t sec]\n", argv[0]);
                printf(" where -e     enables the watchdog timer\n");
                printf("       -d     disables the watchdog timer\n");
                printf("       -r     resets the watchdog timer\n");
                printf("       -t N   sets timeout to N seconds\n");
                printf("       -x     show eXtra debug messages\n");
		exit(1);
     }
  if (t == 0) t = 120;

  rv = saHpiInitialize(&hpiVer);
  if (rv != SA_OK) {
	printf("saHpiInitialize error %d\n",rv);
	exit(-1);
	}
  rv = saHpiSessionOpen(SAHPI_DEFAULT_DOMAIN_ID,&sessionid,NULL);
  if (rv != SA_OK) {
        if (rv == SA_ERR_HPI_ERROR)
           printf("saHpiSessionOpen: error %d, SpiLibd not running\n",rv);
        else
	   printf("saHpiSessionOpen error %d\n",rv);
	exit(-1);
	}
 
  rv = saHpiResourcesDiscover(sessionid);
  if (fdebug) printf("saHpiResourcesDiscover rv = %d\n",rv);
  rv = saHpiRptInfoGet(sessionid,&rptinfo);
  if (fdebug) printf("saHpiRptInfoGet rv = %d\n",rv);
  printf("RptInfo: UpdateCount = %d, UpdateTime = %lx\n",
         rptinfo.UpdateCount, (unsigned long)rptinfo.UpdateTimestamp);
 
  /* walk the RPT list */
  rptentryid = SAHPI_FIRST_ENTRY;
  while ((rv == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY))
  {
     rv = saHpiRptEntryGet(sessionid,rptentryid,&nextrptentryid,&rptentry);
     if (rv != SA_OK) printf("RptEntryGet: rv = %d\n",rv);
     if (rv == SA_OK) {
	/* handle WDT for this RPT entry */
	resourceid = rptentry.ResourceId;
	rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0;
	printf("rptentry[%d] resourceid=%d tag: %s\n",
		rptentryid, resourceid, rptentry.ResourceTag.Data);

	/*
	 * The definition for SAHPI_DEFAULT_WATCHDOG_NUM is broken,
	 * so we are assigning wdnum to 0x00 which is what SaHpi.h
	 * attempted to set the default as
	 */
	wdnum = (SaHpiWatchdogNumT)0x00;

	rv = saHpiWatchdogTimerGet(sessionid,resourceid,wdnum,&wdt);
	if (fdebug) 
	   printf("saHpiWatchdogTimerGet rv = %d\n",rv);
	show_wdt(wdnum,&wdt);

	if (fdisable) {
	   printf("Disabling watchdog timer ...\n");
	   /* clear FRB2, timeout back to 120 sec */
	   /* TODO: add setting wdt values here */
	   wdt.TimerUse = 1;    /* 1=FRB2 2=POST 3=OSLoad 4=SMS_OS 5=OEM */
	   wdt.TimerAction = 0; /* 0=none 1=reset 2=powerdown 3=powercycle */
	   wdt.PretimerInterrupt = 0; /* 0=none 1=SMI 2=NMI 3=message */
	   wdt.PreTimeoutInterval = 60000; /*msec*/
	   wdt.InitialCount = 120000; /*msec*/
	   wdt.PresentCount = 120000; /*msec*/

	   rv = saHpiWatchdogTimerSet(sessionid,resourceid,wdnum,&wdt);
	   if (fdebug) printf("saHpiWatchdogTimerSet rv = %d\n",rv);
	   show_wdt(wdnum,&wdt);
	} else if (fenable) {
	   printf("Enabling watchdog timer ...\n");
	   /* hard reset action, no pretimeout, clear SMS/OS when done */
	   /* use t for timeout */
	   /* TODO: add setting wdt values here */
	   wdt.TimerUse = 4;    /* 1=FRB2 2=POST 3=OSLoad 4=SMS_OS 5=OEM */
	   wdt.TimerAction = 1; /* 0=none 1=reset 2=powerdown 3=powercycle */
	   wdt.PretimerInterrupt = 2; /* 0=none 1=SMI 2=NMI 3=message */
	   wdt.PreTimeoutInterval = (t / 2) * 1000; /*msec*/
	   wdt.InitialCount = t * 1000; /*msec*/
	   wdt.PresentCount = t * 1000; /*msec*/

	   rv = saHpiWatchdogTimerSet(sessionid,resourceid,wdnum,&wdt);
	   if (fdebug) printf("saHpiWatchdogTimerSet rv = %d\n",rv);
	   show_wdt(wdnum,&wdt);
	}
	if (freset && !fdisable) {
	   printf("Resetting watchdog timer ...\n");
	   rv = saHpiWatchdogTimerReset(sessionid,resourceid,wdnum);
	   if (fdebug) printf("saHpiWatchdogTimerReset rv = %d\n",rv);
	}
	rptentryid = nextrptentryid;  /* get next RPT (usu only one anyway) */
     }  /*endif RPT ok*/
  }  /*end while loop*/
 
  rv = saHpiSessionClose(sessionid);
  rv = saHpiFinalize();

  exit(0);
  return(0);
}
 
/* end hpiwdt.c */
