/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copyright IBM Corp. 2004 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *     Andy Cress <arcress@users.sourceforge.net>
 *     Renier Morales <renier@openhpi.org>
 *	
 * ChangeLog:
 *	09/08/04 pdphan@users.sf.net Add sensor number to the display.
 *      01/06/05 arcress  reduce number of display lines per sensor
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_clients.h>

#define OH_SVN_REV "$Revision: 1.6 $"

int fdebug = 0;
int fshowthr = 0;
int fshowrange = 0;
int fshowstate = 0;

static void ShowSensor(SaHpiSessionIdT sessionid,
                       SaHpiResourceIdT resourceid,
                       SaHpiSensorRecT *sensorrec )
{
        SaHpiSensorNumT sensornum;
        SaHpiSensorReadingT reading;
        SaHpiSensorThresholdsT thresh;
        SaHpiEventStateT events;
        SaHpiTextBufferT text;
        SaErrorT rv;
        
        sensornum = sensorrec->Num;
        rv = saHpiSensorReadingGet(sessionid,resourceid, sensornum, &reading, &events);
        if (rv != SA_OK)  {
                printf("ReadingGet ret=%s\n", oh_lookup_error(rv));
                return;
        }
        
        if (reading.IsSupported ) {        
        	if((rv = oh_decode_sensorreading(reading, sensorrec->DataFormat, &text)) == SA_OK) {
                	printf("= %s\n", text.Data);
        	} else {
                	printf("= FAILED, %s\n", oh_lookup_error(rv));
        	}
	} else {
                printf("\n                Sensor reading is not supported\n");
        }
	
	if (fshowstate) {
        	if ((rv = oh_decode_eventstate(events, sensorrec->Category, &text)) == SA_OK) {
                	printf("                Current Sensor State = %s\n", text.Data);
        	} else {
			printf("                Can not decode Sensor EventState value %d\n", (int) events);
		}
	}
	
	
                
        if (fshowrange) { // show ranges
                printf("\t    Ranges::\n");
                if ( sensorrec->DataFormat.Range.Flags & SAHPI_SRF_NOMINAL ) {
                        if((rv = oh_decode_sensorreading(sensorrec->DataFormat.Range.Nominal, 
							 sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tNominal of Range: %s\n", text.Data);
                        } else {
                                printf( "\t\tNominal of Range: FAILED %s\n", oh_lookup_error(rv));
                        }
                }
                if ( sensorrec->DataFormat.Range.Flags & SAHPI_SRF_MAX ) {
                        if((rv = oh_decode_sensorreading(sensorrec->DataFormat.Range.Max, 
							 sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tMax of Range: %s\n", text.Data);
                        } else {
                                printf( "\t\tMax of Range: FAILED %s\n", oh_lookup_error(rv));
                        }
                }
                if ( sensorrec->DataFormat.Range.Flags & SAHPI_SRF_MIN ) {
                        if((rv = oh_decode_sensorreading(sensorrec->DataFormat.Range.Min, 
							 sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tMin of Range: %s\n", text.Data);
                        } else {
                                printf( "\t\tMin of Range: FAILED %s\n", oh_lookup_error(rv));
                        }
                }
                if ( sensorrec->DataFormat.Range.Flags & SAHPI_SRF_NORMAL_MAX ) {
                        if((rv = oh_decode_sensorreading(sensorrec->DataFormat.Range.NormalMax, 
                                                       sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tNormal Max of Range: %s\n", text.Data);
                        } else {
                                printf( "\t\tNormal Max of Range: FAILED %s\n", oh_lookup_error(rv));
                        }
                }
                if ( sensorrec->DataFormat.Range.Flags & SAHPI_SRF_NORMAL_MIN ) {
                        if((rv = oh_decode_sensorreading(sensorrec->DataFormat.Range.NormalMin, 
							 sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tNormal Min of Range: %s\n", text.Data);
                        } else {
                                printf( "\t\tNormal Min of Range: FAILED %s\n", oh_lookup_error(rv));
                        }
                }
        }
        if(fshowthr) { // show thresholds
                rv = saHpiSensorThresholdsGet(sessionid,resourceid, sensornum, &thresh);
                 if (rv != SA_OK)  {
                         printf("\t    ThresholdsGet ret=%s\n", oh_lookup_error(rv));
                         return;
                 }
                 printf( "\t    Thresholds::\n" );

                 if (thresh.LowCritical.IsSupported) {
                         if((rv = oh_decode_sensorreading(thresh.LowCritical, 
                                                        sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tLow Critical Threshold: %s\n", text.Data);
                         } else {
                                 printf( "\t\tLow Critical Threshold: FAILED %s\n", oh_lookup_error(rv));
                         }
                 }
                 if (thresh.LowMajor.IsSupported) {
                         if((rv = oh_decode_sensorreading(thresh.LowMajor, 
                                                        sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tLow Major Threshold: %s\n", text.Data);
                         } else {
                                 printf( "\t\tLow Major Threshold: FAILED %s\n", oh_lookup_error(rv));
                         }
                 }
                 if (thresh.LowMinor.IsSupported) {
                         if((rv = oh_decode_sensorreading(thresh.LowMinor, 
                                                        sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tLow Minor Threshold: %s\n", text.Data);
                         } else {
                                 printf( "\t\tLow Minor Threshold: FAILED %s\n", oh_lookup_error(rv));
                         }
                 }
                 if (thresh.UpCritical.IsSupported) {
                         if((rv = oh_decode_sensorreading(thresh.UpCritical, 
                                                        sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tUp Critical Threshold: %s\n", text.Data);
                         } else {
                                 printf( "\t\tUp Critical Threshold: FAILED %s\n", oh_lookup_error(rv));
                         }
                 }
                 if (thresh.UpMajor.IsSupported) {
                         if((rv = oh_decode_sensorreading(thresh.UpMajor, 
                                                        sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tUp Major Threshold: %s\n", text.Data);
                         } else {
                                 printf( "\t\tUp Major Threshold: FAILED %s\n", oh_lookup_error(rv));
                         }
                 }
                 if (thresh.UpMinor.IsSupported) {
                         if((rv = oh_decode_sensorreading(thresh.UpMinor, 
                                                        sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tUp Minor Threshold: %s\n", text.Data);
                         } else {
                                 printf( "\t\tUp Minor Threshold: FAILED %s\n", oh_lookup_error(rv));
                         }
                 }
                 if (thresh.PosThdHysteresis.IsSupported) {
                         if((rv = oh_decode_sensorreading(thresh.PosThdHysteresis, 
                                                        sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tPos Threshold Hysteresis: %s\n", text.Data);
                         } else {
                                 printf( "\t\tPos Threshold Hysteresis: FAILED %s\n", oh_lookup_error(rv));
                         }
                 }
                 if (thresh.NegThdHysteresis.IsSupported) {
                         if((rv = oh_decode_sensorreading(thresh.NegThdHysteresis, 
                                                        sensorrec->DataFormat, &text)) == SA_OK) {
                                printf( "\t\tNeg Threshold Hysteresis: %s\n", text.Data);
                         } else {
                                 printf( "\t\tNeg Threshold Hysteresis: FAILED %s\n", oh_lookup_error(rv));
                         }
                 }
        }
	/* if extra lines, double-space output */
        if (fshowthr || fshowrange) printf("\n");
        return;
}

int main(int argc, char **argv)
{
        int c;
        char *ep_string = NULL;
        SaErrorT rv;
        SaHpiDomainInfoT dinfo;
        SaHpiSessionIdT sessionid;
        SaHpiRptEntryT rptentry;
        SaHpiEntryIdT rptentryid;
        SaHpiEntryIdT nextrptentryid;
        SaHpiEntryIdT entryid;
        SaHpiEntryIdT nextentryid;
        SaHpiResourceIdT resourceid;
        SaHpiRdrT rdr;
        SaHpiEntityPathT ep_target;
                
	oh_prog_version(argv[0], OH_SVN_REV);
        
        while ( (c = getopt( argc, argv,"rtse:x?")) != EOF )
                switch(c) {
                case 't': fshowthr = 1; break;
                case 'r': fshowrange = 1; break;
                case 's': fshowstate = 1; break;		
                case 'x': fdebug = 1; break;
                case 'e':
                        if (optarg) {
				ep_string = (char *)strdup(optarg);
                        }
			oh_encode_entitypath(ep_string, &ep_target);
                        break;
                default:
                        printf("Usage %s [-t -r -x -e]\n", argv[0]);
                        printf("where -t = show Thresholds also\n");
                        printf("      -r = show Range values also\n");
                        printf("      -s = show EventState also\n");			
                        printf("      -e entity path = limit to a single entity\n");
                        printf("      -x = show eXtra debug messages\n");
                        exit(1);
                }
                
        rv = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID,&sessionid,NULL);
        if (rv != SA_OK) {
                printf("saHpiSessionOpen: %s", oh_lookup_error(rv));
                exit(-1);
        }
        
        if (fdebug) printf("Starting Discovery ...\n");
        rv = saHpiDiscover(sessionid);
        if (fdebug) printf("saHpiResourcesDiscover %s\n", oh_lookup_error(rv));
        
        
        rv = saHpiDomainInfoGet(sessionid,&dinfo);

        if (fdebug) printf("saHpiDomainInfoGet %s\n", oh_lookup_error(rv));
        
        printf("RptInfo: UpdateCount = %d, UpdateTime = %lx\n",
               dinfo.RptUpdateCount, (unsigned long)dinfo.RptUpdateTimestamp);
        
        /* walk the RPT list */
        rptentryid = SAHPI_FIRST_ENTRY;
        while ((rv == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY))
        {
                rv = saHpiRptEntryGet(sessionid,rptentryid,&nextrptentryid,&rptentry);
                if (fdebug) printf("saHpiRptEntryGet %s\n", oh_lookup_error(rv));
                                                
                if (rv == SA_OK) {
                        /* Walk the RDR list for this RPT entry */

                        /* Filter by entity path if specified */
                        if (ep_string && !oh_cmp_ep(&ep_target,&(rptentry.ResourceEntity))) {
                                rptentryid = nextrptentryid;
                                continue;
                        }
                        
                        entryid = SAHPI_FIRST_ENTRY;
                        resourceid = rptentry.ResourceId;
                        rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0; 
                        printf("\nRPTEntry[%d] tag: %s\n",
                               resourceid,rptentry.ResourceTag.Data);
                        oh_print_ep(&rptentry.ResourceEntity, 0);
                        while ((rv == SA_OK) && (entryid != SAHPI_LAST_ENTRY))
                        {
                                rv = saHpiRdrGet(sessionid,resourceid,
                                                 entryid,&nextentryid, &rdr);
                                if (fdebug) printf("saHpiRdrGet[%d] rv = %d\n",entryid,rv);
                                if (rv == SA_OK) {
                                        rdr.IdString.Data[rdr.IdString.DataLength] = 0;
                                        if (rdr.RdrType == SAHPI_SENSOR_RDR) {
                                           printf("   RDR[%6d]: Sensor[%3d] %s   \t",
                                               rdr.RecordId,
					       rdr.RdrTypeUnion.SensorRec.Num,
                                               rdr.IdString.Data);	
                                                   ShowSensor(sessionid,resourceid,
                                                               &rdr.RdrTypeUnion.SensorRec);
                                        } else {
                                           printf("   RDR[%6d]: %s %s\n",
                                               rdr.RecordId,
					       oh_lookup_rdrtype(rdr.RdrType),
                                               rdr.IdString.Data);
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
        
        exit(0);
        return(0);
}
 
/* end hpisensor.c */
