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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "SaHpi.h"

#define  uchar  unsigned char
char *progver  = "1.0";
char fdebug = 0;
char *states[3] = {"off", "ON ", "unknown" };
uchar fsetid = 0;
uchar fid = 0;
#define NLEDS  4
struct { 
   uchar fset;
   uchar val;
} leds[NLEDS] = {  /* rdr.RdrTypeUnion.CtrlRec.Oem is an index for this */
{/*pwr*/ 0, 0},
{/*crt*/ 0, 0},
{/*maj*/ 0, 0},
{/*min*/ 0, 0} };


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
  SaHpiEntryIdT entryid;
  SaHpiEntryIdT nextentryid;
  SaHpiResourceIdT resourceid;
  SaHpiRdrT rdr;
  SaHpiCtrlTypeT  ctltype;
  SaHpiCtrlNumT   ctlnum;
  SaHpiCtrlStateT ctlstate;
  int j;
  uchar b = 0;

  printf("%s ver %s\n", argv[0],progver);
  while ( (c = getopt( argc, argv,"rxc:m:n:p:i:o?")) != EOF )
     switch(c) {
          
	case 'c': b = atoi(optarg);      /* set crit alarm value */
		  leds[1].fset = 1; 
		  leds[1].val = b;
                  break;
	case 'm': b = atoi(optarg);      /* set major alarm value */
		  leds[2].fset = 1; 
		  leds[2].val = b;
                  break;
	case 'n': b = atoi(optarg);      /* set minor alarm value */
		  leds[3].fset = 1; 
		  leds[3].val = b;
                  break;
	case 'p': b = atoi(optarg);      /* set power alarm value */
		  leds[0].fset = 1; 
		  leds[0].val = b;
                  break;
	case 'i': fid = atoi(optarg);     /* set chassis id on/off */
		  fsetid = 1;
                  break;
	case 'o': fsetid=0; fid=0; 	  /* set all alarms off */
		  for (j = 0; j < NLEDS; j++) { 
			leds[j].fset = 1; 
			leds[j].val = 0; 
		  }
                  break;
	case 'x': fdebug = 1;     break;  /* debug messages */
	default:
                printf("Usage: %s [-c -i -m -n -p -o -x]\n", argv[0]);
                printf(" where -c1  sets Critical Alarm on\n");
                printf("       -c0  sets Critical Alarm off\n");
                printf("       -m1  sets Major Alarm on\n");
                printf("       -m0  sets Major Alarm off\n");
                printf("       -n1  sets Minor Alarm on\n");
                printf("       -n0  sets Minor Alarm off\n");
                printf("       -p1  sets Power Alarm on\n");
                printf("       -p0  sets Power Alarm off\n");
                printf("       -i5  sets Chassis ID on for 5 sec\n");
                printf("       -i0  sets Chassis ID off\n");
                printf("       -o   sets all Alarms off\n");
                printf("       -x   show eXtra debug messages\n");
		exit(1);
     }

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
	/* walk the RDR list for this RPT entry */
	entryid = SAHPI_FIRST_ENTRY;
	resourceid = rptentry.ResourceId;
	rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0;
	printf("rptentry[%d] resourceid=%d tag: %s\n",
		entryid,resourceid, rptentry.ResourceTag.Data);
	while ((rv == SA_OK) && (entryid != SAHPI_LAST_ENTRY))
	{
		rv = saHpiRdrGet(sessionid,resourceid,
				entryid,&nextentryid, &rdr);
  		if (fdebug) printf("saHpiRdrGet[%d] rv = %d\n",entryid,rv);
		if (rv == SA_OK) {
		   if (rdr.RdrType == SAHPI_CTRL_RDR) { 
			/*type 1 includes alarm LEDs*/
			ctlnum = rdr.RdrTypeUnion.CtrlRec.Num;
			rdr.IdString.Data[rdr.IdString.DataLength] = 0;
			if (fdebug) printf("Ctl[%d]: %d %d %s\n",
				ctlnum, rdr.RdrTypeUnion.CtrlRec.Type,
				rdr.RdrTypeUnion.CtrlRec.OutputType,
				rdr.IdString.Data);
			rv = saHpiControlTypeGet(sessionid,resourceid,
					ctlnum,&ctltype);
  			if (fdebug) printf("saHpiControlTypeGet[%d] rv = %d, type = %d\n",ctlnum,rv,ctltype);
			rv = saHpiControlStateGet(sessionid,resourceid,
					ctlnum,&ctlstate);
  			if (fdebug) 
			   printf("saHpiControlStateGet[%d] rv = %d v = %x\n",
				ctlnum,rv,ctlstate.StateUnion.Digital);
			printf("RDR[%d]: ctltype=%d:%d oem=%02x %s  \t",
				rdr.RecordId, 
				rdr.RdrTypeUnion.CtrlRec.Type,
				rdr.RdrTypeUnion.CtrlRec.OutputType,
				rdr.RdrTypeUnion.CtrlRec.Oem,
				rdr.IdString.Data);
			if (rv == SA_OK) {
			   if (ctlstate.Type == SAHPI_CTRL_TYPE_ANALOG)
			        b = 2;  /*Identify*/
			   else {
			      b = ctlstate.StateUnion.Digital;
			      if (b > 2) b = 2; 
			   }
			   printf("state = %s\n",states[b]);
			} else { printf("\n"); }
			if (rdr.RdrTypeUnion.CtrlRec.Type == SAHPI_CTRL_TYPE_ANALOG &&
			    rdr.RdrTypeUnion.CtrlRec.OutputType == SAHPI_CTRL_LED) {
			    /* This is a Chassis Identify */
			    if (fsetid) {
				printf("Setting ID led to %d sec\n", fid);
				ctlstate.Type = SAHPI_CTRL_TYPE_ANALOG;
				ctlstate.StateUnion.Analog = fid;
				rv = saHpiControlStateSet(sessionid,
					   resourceid, ctlnum,&ctlstate);
				printf("saHpiControlStateSet[%d] rv = %d\n",ctlnum,rv);
			    }
			} else 
			if (rdr.RdrTypeUnion.CtrlRec.Type == SAHPI_CTRL_TYPE_DIGITAL &&
			    (rdr.RdrTypeUnion.CtrlRec.Oem & 0xf0) == 0x10 &&
			    rdr.RdrTypeUnion.CtrlRec.OutputType == SAHPI_CTRL_LED) {
				/* this is an alarm LED */
				b = (uchar)rdr.RdrTypeUnion.CtrlRec.Oem & 0x0f;
				if ((b < NLEDS) && leds[b].fset) {
				   printf("Setting led %d to %d\n",b,leds[b].val);
				   if (leds[b].val == 0) 
					ctlstate.StateUnion.Digital = SAHPI_CTRL_STATE_OFF;
				   else 
					ctlstate.StateUnion.Digital = SAHPI_CTRL_STATE_ON;
				   rv = saHpiControlStateSet(sessionid,
						resourceid, ctlnum,&ctlstate);
  				   /* if (fdebug)  */
					printf("saHpiControlStateSet[%d] rv = %d\n",ctlnum,rv);
				}
			}
			rv = SA_OK;  /* ignore errors & continue */
		    }
		    j++;
		    entryid = nextentryid;
		}
	}
	rptentryid = nextrptentryid;
     }
  }
 
  rv = saHpiSessionClose(sessionid);
  rv = saHpiFinalize();

  exit(0);
  return(0);
}
 
/*-----------Sample output-----------------------------------
hpialarmpanel ver 0.6
RptInfo: UpdateCount = 5, UpdateTime = 8a2dc6c0
rptentry[0] resourceid=1 tag: Mullins
RDR[45]: ctltype=2:1 oem=0  Chassis Identify Control
RDR[48]: ctltype=0:1 oem=10 Front Panel Power Alarm LED         state = off
RDR[51]: ctltype=0:1 oem=13 Front Panel Minor Alarm LED         state = ON
RDR[46]: ctltype=0:0 oem=0  Cold Reset Control
RDR[49]: ctltype=0:1 oem=11 Front Panel Critical Alarm LED      state = off
RDR[50]: ctltype=0:1 oem=12 Front Panel Major Alarm LED         state = off
 *-----------------------------------------------------------*/
/* end hpialarmpanel.c */
