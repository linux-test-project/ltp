/*      -*- linux-c -*-
 *
 * Copyright (c) 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Filename: hpiwdt.c
 * Authors:  Andy Cress <arcress@users.sourceforge.net>
 * 
 * Changes:
 * 03/15/04 Andy Cress - v1.0 added strings for use & actions in show_wdt
 * 10/13/04  kouzmich  - porting to HPI B
 * 12/02/04 Andy Cress - v1.1 fixed domain/RPT loop, added some decoding
 */
/* 
 * This tool reads and enables the watchdog timer via HPI.
 * Note that there are other methods for doing this, and the
 * standard interface is for the driver to expose a /dev/watchdog
 * device interface.
 * WARNING: If you enable the watchdog, make sure you have something
 * set up to keep resetting the timer at regular intervals, or it
 * will reset your system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <SaHpi.h>
#include <oh_clients.h>

#define  uchar  unsigned char
#define OH_SVN_REV "$Revision: 1.6 $"

#define NUSE  6
char *usedesc[NUSE] = {"reserved", "BIOS FRB2", "BIOS/POST",
                    "OS Load", "SMS/OS", "OEM" };
#define NACT  5
char *actions[NACT] = {"No action", "Hard Reset", "Power down",
		    "Power cycle", "Reserved" };
char fdebug = 0;

static void
show_wdt(SaHpiWatchdogNumT  wdnum, SaHpiWatchdogT *wdt)
{
  int icount, pcount;
  char ustr[12]; 
  char astr[16]; 
  char estr[30]; 
  char *pstr;
  icount = wdt->InitialCount /1000;        /*1000 msec = 1 sec*/
  pcount = wdt->PresentCount /1000;

  if (wdt->TimerUse > NUSE) sprintf(ustr,"%d", wdt->TimerUse );
  else strcpy(ustr, usedesc[wdt->TimerUse]);
  if (wdt->TimerAction > NACT) sprintf(astr,"%d", wdt->TimerAction );
  else strcpy(astr, actions[wdt->TimerAction]);
  printf("Watchdog: Num=%d, Log=%d, Running=%d, TimerUse=%s, TimerAction=%s\n",
	wdnum,wdt->Log,wdt->Running,ustr, astr);
  if (wdt->TimerUseExpFlags == 0) strcpy(estr,"none");
  else {
	estr[0] = 0;
	if (wdt->TimerUseExpFlags & 0x01) strcat(estr,"FRB2 ");
	if (wdt->TimerUseExpFlags & 0x02) strcat(estr,"POST ");
	if (wdt->TimerUseExpFlags & 0x04) strcat(estr,"OS_Load ");
	if (wdt->TimerUseExpFlags & 0x08) strcat(estr,"SMS_OS ");
	if (wdt->TimerUseExpFlags & 0x10) strcat(estr,"OEM ");
  }
  printf("          ExpiredUse=%s, Timeout=%d sec, Counter=%d sec\n",
	estr, icount,pcount);
  switch(wdt->PretimerInterrupt) {
  case 1:  pstr = "SMI";    break;
  case 2:  pstr = "NMI";    break;
  case 3:  pstr = "MsgInt"; break;
  default: pstr = "none";   break;
  }
  printf("          PreTimerInterrupt=%s, PreTimeoutInterval=%d msec\n",
	 pstr,wdt->PreTimeoutInterval);
  return;
}

int
main(int argc, char **argv)
{
  int c;
  SaErrorT rv;
  SaHpiSessionIdT sessionid;
  SaHpiDomainInfoT domainInfo;
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

  oh_prog_version(argv[0], OH_SVN_REV);
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

  rv = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sessionid, NULL);
  if (rv != SA_OK) {
        printf("saHpiSessionOpen error %d\n",rv);
	exit(-1);
	}
  rv = saHpiDiscover(sessionid);
  if (fdebug) printf("saHpiDiscover rv = %d\n",rv);
  rv = saHpiDomainInfoGet(sessionid, &domainInfo);
  if (fdebug) printf("saHpiDomainInfoGet rv = %d\n",rv);
  printf("DomainInfo: UpdateCount = %x, UpdateTime = %lx\n",
       domainInfo.RptUpdateCount, (unsigned long)domainInfo.RptUpdateTimestamp);

  /* walk the RPT list */
  rptentryid = SAHPI_FIRST_ENTRY;
  while ((rv == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY))
  {
     rv = saHpiRptEntryGet(sessionid,rptentryid,&nextrptentryid,&rptentry);
     if (rv != SA_OK) printf("RptEntryGet: rv = %d\n",rv);
     if (rv == SA_OK) {
	/* handle WDT for this RPT entry */
	resourceid = rptentry.ResourceId;
	// rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0;
	if (fdebug)
	   printf("rptentry[%d] resourceid=%d capab=%x tag: %s\n",
		rptentryid, resourceid, rptentry.ResourceCapabilities, 
		rptentry.ResourceTag.Data);

	if (rptentry.ResourceCapabilities & SAHPI_CAPABILITY_WATCHDOG) {
	   printf("%s has watchdog capability\n",rptentry.ResourceTag.Data);

	   wdnum = SAHPI_DEFAULT_WATCHDOG_NUM;
	   rv = saHpiWatchdogTimerGet(sessionid,resourceid,wdnum,&wdt);
	   if (fdebug) printf("saHpiWatchdogTimerGet rv = %d\n",rv);
	   if (rv != 0) {
		printf("saHpiWatchdogTimerGet error = %d\n",rv);
		rv = 0;
		rptentryid = nextrptentryid; 
		continue;
	   }
	   show_wdt(wdnum,&wdt);

	   if (fdisable) {
	      printf("Disabling watchdog timer ...\n");
	      /* clear FRB2, timeout back to 120 sec */
	      /* TODO: add setting wdt values here */
	      wdt.TimerUse = SAHPI_WTU_NONE;    /* 1=FRB2 2=POST 3=OSLoad 4=SMS_OS 5=OEM */
	      wdt.TimerAction = SAHPI_WAE_NO_ACTION; /* 0=none 1=reset 2=powerdown 3=powercycle */
	      wdt.PretimerInterrupt = SAHPI_WPI_NONE; /* 0=none 1=SMI 2=NMI 3=message */
	      wdt.PreTimeoutInterval = 60000; /*msec*/
	      wdt.InitialCount = 120000; /*msec*/
	      wdt.PresentCount = 120000; /*msec*/

	      rv = saHpiWatchdogTimerSet(sessionid,resourceid,wdnum,&wdt);
	      if (fdebug) printf("saHpiWatchdogTimerSet rv = %d\n",rv);
	      if (rv == 0) show_wdt(wdnum,&wdt);
	   } else if (fenable) {
	      printf("Enabling watchdog timer ...\n");
	      /* hard reset action, no pretimeout, clear SMS/OS when done */
	      /* use t for timeout */
	      wdt.TimerUse = SAHPI_WTU_SMS_OS;    /* 1=FRB2 2=POST 3=OSLoad 4=SMS_OS 5=OEM */
	      wdt.TimerAction = SAHPI_WAE_RESET; /* 0=none 1=reset 2=powerdown 3=powercycle */
	      wdt.PretimerInterrupt = SAHPI_WPI_NMI; /* 0=none 1=SMI 2=NMI 3=message */
	      wdt.PreTimeoutInterval = (t / 2) * 1000; /*msec*/
	      wdt.InitialCount = t * 1000; /*msec*/
	      wdt.PresentCount = t * 1000; /*msec*/

	      rv = saHpiWatchdogTimerSet(sessionid,resourceid,wdnum,&wdt);
	      if (fdebug) printf("saHpiWatchdogTimerSet rv = %d\n",rv);
	      if (rv == 0) show_wdt(wdnum,&wdt);
	   }
	   if (freset && !fdisable) {
	      printf("Resetting watchdog timer ...\n");
	      rv = saHpiWatchdogTimerReset(sessionid,resourceid,wdnum);
	      if (fdebug) printf("saHpiWatchdogTimerReset rv = %d\n",rv);
	   }
	} /*watchdog capability*/
	rptentryid = nextrptentryid;  /* get next RPT (usu only one anyway) */
     }  /*endif RPT ok*/
  }  /*end while loop*/
 
  rv = saHpiSessionClose(sessionid);

  exit(0);
}
 
/* end hpiwdt.c */
