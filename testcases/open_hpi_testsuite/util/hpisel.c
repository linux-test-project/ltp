/* -*- linux-c -*-
 *
 * $Id: hpisel.c,v 1.1 2004/02/11 16:54:27 robbiew Exp $
 *
 * Copyright (c) 2003 Intel Corporation.
 * (C) Copyright IBM Corp 2003
 * Authors:  
 *     Andy Cress  arcress@users.sourceforge.net
 *     Sean Dague <http://dague.net/sean>
 *  
 * 04/28/03 Andy Cress - created
 * 04/30/03 Andy Cress v0.6 first good run with common use cases
 * 05/06/03 Andy Cress v0.7 added -c option to clear it
 * 05/29/03 Andy Cress v0.8 fixed pstr warnings
 * 06/13/03 Andy Cress v0.9 fixed strcpy bug, 
 *                          workaround for SensorEvent.data3
 * 06/19/03 Andy Cress 0.91 added low SEL free space warning
 * 06/25/03 Andy Cress v1.0 rework event data logic 
 * 07/23/03 Andy Cress workaround for OpenHPI BUGGY stuff
 * 11/12/03 Andy Cress v1.1 check for CAPABILITY_SEL
 * 
 * Note that HPI 1.0 does not return all event data fields, so event
 * types other than 'user' will not have all bytes filled in as they
 * would have from IPMI alone.
 */
/*M*
Copyright (c) 2003, Intel Corporation
All rights reserved.

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
 *M*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include "SaHpi.h"

char progver[] = "1.1";
int fdebug = 0;
int fclear = 0;

#define NEVTYPES  5
char *evtypes[NEVTYPES] = {"sensor","hotswap","watchdog","oem   ","user  "};

#define NECODES  27
struct { int code; char *str;
} ecodes[NECODES] = { 
 {  0,    "Success" },
 { -1001, "HPI unspecified error" },
 { -1002, "HPI unsupported function" },
 { -1003, "HPI busy" },
 { -1004, "HPI request invalid" },
 { -1005, "HPI command invalid" },
 { -1006, "HPI timeout" },
 { -1007, "HPI out of space" },
 { -1008, "HPI data truncated" },
 { -1009, "HPI data length invalid" },
 { -1010, "HPI data exceeds limits" },
 { -1011, "HPI invalid params" },
 { -1012, "HPI invalid data" },
 { -1013, "HPI not present" },
 { -1014, "HPI invalid data field" },
 { -1015, "HPI invalid sensor command" },
 { -1016, "HPI no response" },
 { -1017, "HPI duplicate request" },
 { -1018, "HPI updating" },
 { -1019, "HPI initializing" },
 { -1020, "HPI unknown error" },
 { -1021, "HPI invalid session" },
 { -1022, "HPI invalid domain" },
 { -1023, "HPI invalid resource id" },
 { -1024, "HPI invalid request" },
 { -1025, "HPI entity not present" },
 { -1026, "HPI uninitialized" }
};
char def_estr[15] = "HPI error %d   ";

#define NUM_EC  14
struct { int val; char *str;
} eventcats[NUM_EC] = {
{ /*0x00*/ SAHPI_EC_UNSPECIFIED,  "unspecified"},
{ /*0x01*/ SAHPI_EC_THRESHOLD, 	  "Threshold"},
{ /*0x02*/ SAHPI_EC_USAGE, 	  "Usage    "},
{ /*0x03*/ SAHPI_EC_STATE, 	  "State    "},
{ /*0x04*/ SAHPI_EC_PRED_FAIL, 	  "Predictive"},
{ /*0x05*/ SAHPI_EC_LIMIT, 	  "Limit    "},
{ /*0x06*/ SAHPI_EC_PERFORMANCE,  "Performnc"},
{ /*0x07*/ SAHPI_EC_SEVERITY, 	  "Severity "},
{ /*0x08*/ SAHPI_EC_PRESENCE, 	  "DevPresen"},
{ /*0x09*/ SAHPI_EC_ENABLE, 	  "DevEnable"},
{ /*0x0a*/ SAHPI_EC_AVAILABILITY, "Availabil"},
{ /*0x0b*/ SAHPI_EC_REDUNDANCY,   "Redundanc"},
{ /*0x7e*/ SAHPI_EC_USER,         "UserDefin"},
{ /*0x7f*/ SAHPI_EC_GENERIC,      "OemDefin " }};

#define NUM_ES  6
struct { int val; char *str;
} eventstates[NUM_ES] = {
{ SAHPI_ES_LOWER_MINOR , "lo-min" },
{ SAHPI_ES_LOWER_MAJOR , "lo-maj" },
{ SAHPI_ES_LOWER_CRIT  , "lo-crt" },
{ SAHPI_ES_UPPER_MINOR , "hi-min" },
{ SAHPI_ES_UPPER_MAJOR , "hi-maj" },
{ SAHPI_ES_UPPER_CRIT  , "hi-crt" } };

#define NGDESC   4
struct {
 unsigned char val;
 const char str[5];
} gen_desc[NGDESC] = {
 {0x00, "IPMB"},
 {0x03, "BIOS"},
 {0x20, "BMC "},
 {0x21, "SMI "} };

#define NSDESC   13
struct {
 unsigned char s_typ;
 unsigned char s_num;
 unsigned char data1;
 unsigned char data2;
 unsigned char data3;
 char *desc;
} sens_desc[NSDESC] = {
{0x01, 0xff, 0x07, 0xff, 0xff, "Temperature Upper Non-critical" },
{0x01, 0xff, 0x09, 0xff, 0xff, "Temperature Upper Critical"}, 
{0x07, 0xff, 0x01, 0xff, 0xff, "Processor Thermal trip"}, 
{0x09, 0x01, 0xff, 0xff, 0xff, "Power Off/Down"},
{0x0f, 0x06, 0xff, 0xff, 0xff, "POST Code"},
{0x10, 0x09, 0x02, 0xff, 0xff, "EventLog Cleared"},
{0x12, 0x83, 0xff, 0xff, 0xff, "System Boot Event"},
{0x14, 0xff, 0x02, 0xff, 0xff, "Reset Button pressed"},
{0x14, 0xff, 0x00, 0xff, 0xff, "Power Button pressed"},
{0x23, 0x03, 0x01, 0xff, 0xff, "Watchdog2 Hard Reset action"},  
{0x23, 0x03, 0x02, 0xff, 0xff, "Watchdog2 Power down action"},  
{0xf3, 0x85, 0x01, 0xff, 0xff, "State is now OK"},
{0x20, 0x00, 0xff, 0xff, 0xff, "OS Kernel Panic"} };

#define NUMST   0x2A
const char *sensor_types[NUMST] = {
/* 00h */ "reserved",
/* 01h */ "Temperature",
/* 02h */ "Voltage",
/* 03h */ "Current",
/* 04h */ "Fan",
/* 05h */ "Platform Chassis Intrusion",
/* 06h */ "Platform Security Violation",
/* 07h */ "Processor",
/* 08h */ "Power Supply",
/* 09h */ "Power Unit",
/* 0Ah */ "Cooling Device",
/* 0Bh */ "FRU Sensor",
/* 0Ch */ "Memory",
/* 0Dh */ "Drive Slot",
/* 0Eh */ "POST Memory Resize",
/* 0Fh */ "System Firmware",
/* 10h */ "EventLog Cleared",
/* 11h */ "Watchdog 1",
/* 12h */ "System Event",          /* offset 0,1,2 */
/* 13h */ "Critical Interrupt",    /* offset 0,1,2 */
/* 14h */ "Button",                /* offset 0,1,2 */
/* 15h */ "Board",
/* 16h */ "Microcontroller",
/* 17h */ "Add-in Card",
/* 18h */ "Chassis",
/* 19h */ "Chip Set",
/* 1Ah */ "Other FRU",
/* 1Bh */ "Cable / Interconnect",
/* 1Ch */ "Terminator",
/* 1Dh */ "System Boot Initiated",
/* 1Eh */ "Boot Error",
/* 1Fh */ "OS Boot",
/* 20h */ "OS Critical Stop",
/* 21h */ "Slot / Connector",
/* 22h */ "ACPI Power State",
/* 23h */ "Watchdog 2",
/* 24h */ "Platform Alert",
/* 25h */ "Entity Presence",
/* 26h */ "Monitor ASIC",
/* 27h */ "LAN",
/* 28h */ "Management Subsystem Health",
/* 29h */ "Battery",
};

static char *decode_error(SaErrorT code)
{
        int i;
        char *str = NULL;
        for (i = 0; i < NECODES; i++) {
                if (code == ecodes[i].code) { str = ecodes[i].str; break; }
        }
        if (str == NULL) { 
                sprintf(&def_estr[10],"%d",code);
                str = &def_estr[0];
        }
        return(str);
}

static void saftime2str(SaHpiTimeT time, char * str, size_t size) 
{
        struct tm t;
        time_t tt;
        tt = time / 1000000000;
        localtime_r(&tt, &t);
        strftime(str, size, "%b %d, %Y - %H:%M:%S", &t);
}

static void ShowSel( SaHpiSelEntryT  *sel, SaHpiRdrT *rdr, 
                     SaHpiRptEntryT *rptentry )
{
        unsigned char evtype;
        char timestr[40];
        time_t tt1;
        int ec, eci; 
        int es, esi; 
        char *srctag;
        char *rdrtag;
        const char *pstr;
        char estag[8];
        unsigned char *pd;
        int ix, i, styp; 
        int outlen;
        char outbuf[132];
        char mystr[26];
        unsigned char data1, data2, data3;
        
        /*format & print the EventLog entry*/
        
        if (sel->Event.Timestamp > SAHPI_TIME_MAX_RELATIVE) { /*absolute time*/
                tt1 = sel->Event.Timestamp / 1000000000;
                strftime(timestr,sizeof(timestr),"%F %T", localtime(&tt1));
        } else if (sel->Event.Timestamp > SAHPI_TIME_UNSPECIFIED) { /*invalid time*/
                strcpy(timestr,"invalid time     ");
        } else {   /*relative time*/
                tt1 = sel->Event.Timestamp / 1000000000;
                sprintf(timestr,"rel(%lx)", (unsigned long)tt1);  
        }
        if (rptentry->ResourceId == sel->Event.Source) 
                srctag = rptentry->ResourceTag.Data;
        else  
                srctag = "unspec ";  /* SAHPI_UNSPECIFIED_RESOURCE_ID */

        evtype = sel->Event.EventType;
        if (evtype > NEVTYPES) 
                evtype = NEVTYPES - 1;
        
        if (rdr->RdrType == SAHPI_NO_RECORD) {
                rdrtag = "rdr-unkn"; 
        } else {
                rdr->IdString.Data[rdr->IdString.DataLength] = 0; 
                rdrtag = &rdr->IdString.Data[0];
        }
        sprintf(outbuf,"%04x %s %s ", sel->EntryId, 
                timestr, evtypes[evtype] );
        outlen = strlen(outbuf);
        pstr = "";

        /*
          sld: there is a lot of stuff specific to IPMI and other HPI implementations
          here.  Scrubing for HPI 1.0 only data would be a good thing soon
        */
        
        switch(evtype)
        {
        case SAHPI_ET_SENSOR:   /*Sensor*/
                /* decode event category */
                ec = sel->Event.EventDataUnion.SensorEvent.EventCategory;
                for (eci = 0; eci < NUM_EC; eci++) 
                        if (eventcats[eci].val == ec) break; 
                if (eci >= NUM_EC) eci = 0;
                /* decode event state */
                es = sel->Event.EventDataUnion.SensorEvent.EventState;
                if (eci == 1) { /*SAHPI_EC_THRESHOLD*/
                        for (esi = 0; esi < NUM_ES; esi++) 
                                if (eventstates[esi].val == es) break; 
                        if (esi >= NUM_ES) esi = 0;
                        strcpy(estag,eventstates[esi].str);
                } else sprintf(estag,"%02x",es);
                
                /* decode sensor type */
                styp = sel->Event.EventDataUnion.SensorEvent.SensorType;
                /* data3 is not specifically defined in HPI 1.0, implementation hack */
                pd = (unsigned char *)&sel->Event.EventDataUnion.SensorEvent.SensorSpecific;
                data1 = pd[0];
                data2 = pd[1];
                data3 = pd[2];
                if (styp >= NUMST) { styp = 0; }
                
                if (styp == 0x20) { /*OS Critical Stop*/
                        /* Show first 3 chars of panic string */
                        mystr[0] = '(';
                        mystr[1] = sel->Event.EventDataUnion.SensorEvent.SensorNum & 0x7f;
                        mystr[2] = data2 & 0x007f;
                        mystr[3] = data3 & 0x7f;
                        mystr[4] = ')';
                        mystr[5] = 0;
                        if (sel->Event.EventDataUnion.SensorEvent.SensorNum & 0x80)
                                strcat(mystr,"Oops!");
                        if (data2 & 0x80) strcat(mystr,"Int!");
                        if (data3 & 0x80) strcat(mystr,"NullPtr!");
                        pstr = mystr;
                }
                sprintf(&outbuf[outlen], "%s, %s %s %x [%02x %02x %02x] %s", 
                        sensor_types[styp], eventcats[eci].str, estag,
                        sel->Event.EventDataUnion.SensorEvent.SensorNum,
                        data1, data2, data3, pstr);
                break;
        case SAHPI_ET_USER:   /*User, usu 16-byte IPMI SEL record */
                pd = &sel->Event.EventDataUnion.UserEvent.UserEventData[0];
                /* get gen_desc from offset 7 */
                for (ix = 0; ix < NGDESC; ix++)
                        if (gen_desc[ix].val == pd[7]) break;
                if (ix >= NGDESC) ix = 0;
                /* get sensor type description for misc cases */
                styp = pd[10];   /*sensor type*/
                data3 = pd[15];
                /* = *sel->Event.EventDataUnion.SensorEvent.SensorSpecific+1; */
                for (i = 0; i < NSDESC; i++) {
                        if (sens_desc[i].s_typ == styp) {
                                if (sens_desc[i].s_num != 0xff &&
                                    sens_desc[i].s_num != pd[11])
                                        continue;
                                if (sens_desc[i].data1 != 0xff &&
                                    (sens_desc[i].data1 & 0x07) != pd[13])
                                        continue;
                                if (sens_desc[i].data2 != 0xff &&
                                    sens_desc[i].data2 != pd[14])
                                        continue;
                                if (sens_desc[i].data3 != 0xff &&
                                    sens_desc[i].data3 != data3)
                                        continue;
                                /* have a match, use description */
                                pstr = (char *)sens_desc[i].desc;
                                break;
                        }
                } /*end for*/
                if (i >= NSDESC) {
                        if (styp >= NUMST) styp = 0;
                        pstr = sensor_types[styp];
                }
                sprintf(&outbuf[outlen], "%s, %s %02x %02x %02x [%02x %02x %02x]",
                        pstr, gen_desc[ix].str,
                        pd[10],pd[11],pd[12],pd[13],pd[14],data3);
                break;
        case SAHPI_ET_OEM:
                /* only go into this if it is IBM hardware, as others might use
                   the Oem field differently */
                if(sel->Event.EventDataUnion.OemEvent.MId == 2) {
                        /* sld: I'm going to printf directly, as the output buffer isn't
                           big enough for what I want to do */
                        printf("Oem Event:\n");
                        saftime2str(sel->Timestamp, timestr, 40);
                        printf("\tTimestamp: %s\n", timestr);
                        printf("\tSeverity: %d\n", sel->Event.Severity);
                        printf("\tMId:%d, Data: %s\n", 
                               sel->Event.EventDataUnion.OemEvent.MId,
                               sel->Event.EventDataUnion.OemEvent.OemEventData);
                }
                break;
                
        default:
                pd = &sel->Event.EventDataUnion.UserEvent.UserEventData[0];
                styp = pd[10];
                data3 = pd[15];
                /* *sel->Event.EventDataUnion.SensorEvent.SensorSpecific+1 */
                if (styp >= NUMST) { 
                        if (fdebug) printf("sensor type %d >= max %d\n",styp,NUMST);
                        styp = 0; 
                }
                pstr = sensor_types[styp];
                sprintf(&outbuf[outlen], "%s, %x %x, %02x %02x %02x [%02x %02x %02x/%02x]",
                        pstr, pd[0], pd[7], pd[10], pd[11], pd[12], 
                        pd[13], pd[14], pd[15], data3);
                break;
        }
        printf("%s\n",outbuf);
}

int main(int argc, char **argv)
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
        SaHpiSelEntryIdT entryid;
        SaHpiSelEntryIdT nextentryid;
        SaHpiSelEntryIdT preventryid;
        SaHpiSelInfoT info;
        SaHpiSelEntryT  sel;
        SaHpiRdrT rdr;
        
        printf("%s: version %s\n",argv[0],progver); 
        
        while ( (c = getopt( argc, argv,"cx?")) != EOF )
                switch(c) {
                case 'c': fclear = 1; break;
                case 'x': fdebug = 1; break;
                default:
                        printf("Usage %s [-cx]\n",argv[0]);
                        printf("where -c clears the event log\n");
                        printf("      -x displays eXtra debug messages\n");
                        exit(1);
                }
        rv = saHpiInitialize(&hpiVer);
        if (rv != SA_OK) {
                printf("saHpiInitialize: %s\n",decode_error(rv));
                exit(-1);
        }
        rv = saHpiSessionOpen(SAHPI_DEFAULT_DOMAIN_ID,&sessionid,NULL);
        if (rv != SA_OK) {
                if (rv == SA_ERR_HPI_ERROR) 
                        printf("saHpiSessionOpen: error %d, SpiLibd not running\n",rv);
                else
                        printf("saHpiSessionOpen: %s\n",decode_error(rv));
                exit(-1);
        }
 
        rv = saHpiResourcesDiscover(sessionid);
        if (fdebug) printf("saHpiResourcesDiscover %s\n",decode_error(rv));
        rv = saHpiRptInfoGet(sessionid,&rptinfo);
        if (fdebug) printf("saHpiRptInfoGet %s\n",decode_error(rv));
        printf("RptInfo: UpdateCount = %d, UpdateTime = %lx\n",
               rptinfo.UpdateCount, (unsigned long)rptinfo.UpdateTimestamp);
        
        /* walk the RPT list */
        rptentryid = SAHPI_FIRST_ENTRY;
        while ((rv == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY))
        {
                rv = saHpiRptEntryGet(sessionid,rptentryid,&nextrptentryid,&rptentry);
                if (fdebug) printf("saHpiRptEntryGet %s\n",decode_error(rv));
                if (rv == SA_OK) {
                        resourceid = rptentry.ResourceId;
                        if (fdebug) printf("RPT %x capabilities = %x\n", resourceid,
                                           rptentry.ResourceCapabilities);
                        if (!(rptentry.ResourceCapabilities & SAHPI_CAPABILITY_SEL)) {
                                if (fdebug) printf("RPT doesn't have SEL\n");
                                rptentryid = nextrptentryid;
                                continue;  /* no SEL here, try next RPT */
                        }
                        if (fclear) {
                                rv = saHpiEventLogClear(sessionid,resourceid);
                                if (rv == SA_OK) printf("EventLog successfully cleared\n");
                                else printf("EventLog clear, error = %d\n",rv);
                                break;
                        }
                        rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0; 
                        printf("rptentry[%d] tag: %s\n", resourceid,rptentry.ResourceTag.Data);
                        
                        rv = saHpiEventLogInfoGet(sessionid,resourceid,&info);
                        if (fdebug) printf("saHpiEventLogInfoGet %s\n",decode_error(rv));
                        if (rv == SA_OK) {
                                char date[30];
                                printf("EventLog entries=%d, size=%d, enabled=%d\n",
                                       info.Entries,info.Size,info.Enabled);
                                saftime2str(info.UpdateTimestamp,date,30);
                                printf("UpdateTime = %s,", date);
                                saftime2str(info.CurrentTime,date,30);
                                printf("CurrentTime = %s\n", date);
				printf("Overflow = %d\n", info.OverflowFlag);
				printf("DeleteEntrySupported = %d\n", info.DeleteEntrySupported);
                        }
                        
                        entryid = SAHPI_OLDEST_ENTRY;
                        while ((rv == SA_OK) && (entryid != SAHPI_NO_MORE_ENTRIES))
                        {
                                rv = saHpiEventLogEntryGet(sessionid,resourceid,entryid,
                                                           &preventryid,&nextentryid,&sel,&rdr,NULL);
                                if (fdebug) printf("saHpiEventLogEntryGet %s\n",
                                                   decode_error(rv));
                                if (rv == SA_OK) {
                                        ShowSel(&sel, &rdr, &rptentry);
                                        preventryid = entryid;
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
 
/* end hpisel.c */
