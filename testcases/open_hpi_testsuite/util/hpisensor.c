/*      -*- linux-c -*-
 *      $Id: hpisensor.c,v 1.1 2004/02/11 16:54:27 robbiew Exp $
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
#include <string.h>
#include <SaHpi.h>

char progver[] = "1.0";
int fdebug = 0;
int fshowthr = 0;
char *rtypes[5] = {"None    ", "Control ", "Sensor  ", "Invent  ", "Watchdog"};

#define NSU  77
char *units[NSU] = {
        "units", "degrees C", "degrees F", "degrees K", "volts", "amps", 
        "watts", "joules",    "coulombs",  "va",        "nits",  "lumen",
        "lux",   "candela",   "kpa",       "psi",     "newton",  "cfm",
        "rpm",   "Hz",        "us",        "ms",      "sec",     "min",
        "hours", "days",      "weeks",     "mil",     "in",     "ft",
        /*30*/ "cu in", "cu ft", "mm", "cm", "m", "cu cm", "cu m", 
        "liters", "fl oz", "radians", "sterad", "rev", 
        "cycles", "grav", "oz", "lbs", "ft lb", "oz in", "gauss", 
        "gilberts", "henry", "mhenry", "farad", "ufarad", "ohms",
        /*55*/  "", "", "", "", "", "", "", "", "", "", 
        "", "", "", "", "Gb", 
        /*70*/ "bytes", "KB", "MB",  "GB", 
        /*74*/ "", "", "line"
};

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

static void ShowSensor(
        SaHpiSessionIdT sessionid,
        SaHpiResourceIdT resourceid,
        SaHpiSensorRecT *sensorrec )
{
        SaHpiSensorNumT sensornum;
        SaHpiSensorReadingT reading;
        SaHpiSensorReadingT conv_reading;
        SaHpiSensorThresholdsT senstbuff; 
        SaErrorT rv;
        char *unit;
        int i;
        
        sensornum = sensorrec->Num;
        rv = saHpiSensorReadingGet(sessionid,resourceid, sensornum, &reading);
        if (rv != SA_OK)  {
                printf("ReadingGet ret=%d\n", rv);
                return;
        }
        
        if ((reading.ValuesPresent & SAHPI_SRF_INTERPRETED) == 0) { 
                if ((reading.ValuesPresent & SAHPI_SRF_RAW) == 0) {
                        /* no raw or interpreted, so just show event status */
                        /* This is a Compact Sensor */
                        if (reading.ValuesPresent & SAHPI_SRF_EVENT_STATE) 
                                printf(" = %x %x\n", reading.EventStatus.SensorStatus,
                                       reading.EventStatus.EventStatus);
                        else printf(" = no reading\n");
                        return;
                } else {
                        /* have raw, but not interpreted, so try convert. */
                        rv = saHpiSensorReadingConvert(sessionid, resourceid, sensornum,
                                                       &reading, &conv_reading);
                        if (rv != SA_OK) {
                                printf("raw=%x conv_ret=%d\n", reading.Raw, rv);
                                /* printf("conv_rv=%s\n", decode_error(rv)); */
                                return;
                        }
                        else {
                                if (fdebug) printf("conv ok: raw=%x conv=%x\n", reading.Raw,
                                                   conv_reading.Interpreted.Value.SensorUint32);
                                reading.Interpreted.Type = conv_reading.Interpreted.Type;
                                if (reading.Interpreted.Type == SAHPI_SENSOR_INTERPRETED_TYPE_BUFFER)
                                {
                                        memcpy(reading.Interpreted.Value.SensorBuffer,
                                               conv_reading.Interpreted.Value.SensorBuffer,
                                               4); /* SAHPI_SENSOR_BUFFER_LENGTH); */
                                        /* IPMI 1.5 only returns 4 bytes */
                                } else 
                                        reading.Interpreted.Value.SensorUint32 = 
                                                conv_reading.Interpreted.Value.SensorUint32;
                        }
                }
        }
        /* Also show units of interpreted reading */
        i = sensorrec->DataFormat.BaseUnits;
        if (i >= NSU) i = 0;
        unit = units[i];
        switch(reading.Interpreted.Type)
        {
        case SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32:
                printf(" = %5.2f %s\n", 
                       reading.Interpreted.Value.SensorFloat32,unit);
                break;
        case SAHPI_SENSOR_INTERPRETED_TYPE_UINT32:
                printf(" = %d %s\n", 
                       reading.Interpreted.Value.SensorUint32, unit);
                break;
        case SAHPI_SENSOR_INTERPRETED_TYPE_BUFFER:
                printf(" = %02x %02x %02x %02x\n", 
                       reading.Interpreted.Value.SensorBuffer[0],
                       reading.Interpreted.Value.SensorBuffer[1],
                       reading.Interpreted.Value.SensorBuffer[2],
                       reading.Interpreted.Value.SensorBuffer[3]);
                break;
        default: 
                printf(" = %x (itype=%x)\n", 
                       reading.Interpreted.Value.SensorUint32, 
                       reading.Interpreted.Type);
        }
        
        if (fshowthr) {
#ifdef SHOWMAX
                if ( sensorrec->DataFormat.Range.Flags & SAHPI_SRF_MAX )
                        printf( "    Max of Range: %5.2f\n",
                                sensorrec->DataFormat.Range.Max.Interpreted.Value.SensorFloat32);
                if ( sensorrec->DataFormat.Range.Flags & SAHPI_SRF_MIN )
                        printf( "    Min of Range: %5.2f\n",
                                sensorrec->DataFormat.Range.Min.Interpreted.Value.SensorFloat32);
#endif
                /* Show thresholds, if any */
                if ((!sensorrec->Ignore) && (sensorrec->ThresholdDefn.IsThreshold)) {
                        rv = saHpiSensorThresholdsGet(sessionid, resourceid, 
                                                      sensornum, &senstbuff);
                        printf( "\t\t\t");
                        if ( sensorrec->ThresholdDefn.ReadThold & SAHPI_STM_LOW_MINOR ) {
                                printf( "LoMin %5.2f ",
                                        senstbuff.LowMinor.Interpreted.Value.SensorFloat32);
                        } 
                        if ( sensorrec->ThresholdDefn.ReadThold & SAHPI_STM_LOW_MAJOR ) {
                                printf( "LoMaj %5.2f ",
                                        senstbuff.LowMajor.Interpreted.Value.SensorFloat32);
                        } 
                        if ( sensorrec->ThresholdDefn.ReadThold & SAHPI_STM_LOW_CRIT ) {
                                printf( "LoCri %5.2f ",
                                        senstbuff.LowCritical.Interpreted.Value.SensorFloat32);
                        } 
                        if ( sensorrec->ThresholdDefn.ReadThold & SAHPI_STM_UP_MINOR ) {
                                printf( "HiMin %5.2f ",
                                        senstbuff.UpMinor.Interpreted.Value.SensorFloat32);
                        } 
                        if ( sensorrec->ThresholdDefn.ReadThold & SAHPI_STM_UP_MAJOR ) {
                                printf( "HiMaj %5.2f ",
                                        senstbuff.UpMajor.Interpreted.Value.SensorFloat32);
                        } 
                        if ( sensorrec->ThresholdDefn.ReadThold & SAHPI_STM_UP_CRIT ) {
                                printf( "HiCri %5.2f ",
                                        senstbuff.UpCritical.Interpreted.Value.SensorFloat32);
                        } 
#ifdef SHOWMAX
                        if ( sensorrec->ThresholdDefn.ReadThold & SAHPI_STM_UP_HYSTERESIS ) {
                                printf( "Hi Hys %5.2f ",
                                        senstbuff.PosThdHysteresis.Interpreted.Value.SensorFloat32);
                        } 
                        if ( sensorrec->ThresholdDefn.ReadThold & SAHPI_STM_LOW_HYSTERESIS ) {
                                printf( "Lo Hys %5.2f ",
                                        senstbuff.NegThdHysteresis.Interpreted.Value.SensorFloat32);
                        } 
#endif
                        printf( "\n");
                } /* endif valid threshold */
        } /* endif showthr */
        return;
}  /*end ShowSensor*/

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
        SaHpiEntryIdT entryid;
        SaHpiEntryIdT nextentryid;
        SaHpiResourceIdT resourceid;
        SaHpiRdrT rdr;
        
        printf("%s: version %s\n",argv[0],progver); 
        
        while ( (c = getopt( argc, argv,"tx?")) != EOF )
                switch(c) {
                case 't': fshowthr = 1; break;
                case 'x': fdebug = 1; break;
                default:
                        printf("Usage %s [-t -x]\n",argv[0]);
                        printf("where -t = show Thresholds also\n");
                        printf("      -x = show eXtra debug messages\n");
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
        
#ifdef BUGGY
        /* ARC: Bug here in OpenHPI requires re-doing discovery (workaround). */
        { 
                int updcnt;
                int i = 0;
                updcnt = rptinfo.UpdateCount;
                while (rptinfo.UpdateCount == updcnt) {
                        rv = saHpiResourcesDiscover(sessionid);
                        if (fdebug) printf("saHpiResourcesDiscover %s\n",decode_error(rv));
                        rv = saHpiRptInfoGet(sessionid,&rptinfo);
                        if (fdebug) printf("saHpiRptInfoGet %s\n",decode_error(rv));
                        printf("RptInfo/%d: UpdateCount = %d, UpdateTime = %lx\n",
                               ++i,rptinfo.UpdateCount, (unsigned long)rptinfo.UpdateTimestamp);
                }
        }  /*end openhpi bug workaround*/
#endif
        
        /* walk the RPT list */
        rptentryid = SAHPI_FIRST_ENTRY;
        while ((rv == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY))
        {
                rv = saHpiRptEntryGet(sessionid,rptentryid,&nextrptentryid,&rptentry);
                if (fdebug) printf("saHpiRptEntryGet %s\n",decode_error(rv));
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
                                        char *eol;
                                        rdr.IdString.Data[rdr.IdString.DataLength] = 0;
                                        if (rdr.RdrType == SAHPI_SENSOR_RDR) eol = "    \t";
                                        else eol = "\n";
                                        printf("RDR[%02d]: %s %s %s",rdr.RecordId,
                                               rtypes[rdr.RdrType],rdr.IdString.Data,eol);
                                        if (rdr.RdrType == SAHPI_SENSOR_RDR) {
                                                ShowSensor(sessionid,resourceid,
                                                           &rdr.RdrTypeUnion.SensorRec);
                                        } 
                                        entryid = nextentryid;
                                } else {
                                        rv = SA_OK;
                                        entryid = SAHPI_LAST_ENTRY;
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
 
/* end hpisensor.c */
