/*      -*- linux-c -*-
 *
 * Copyright (c) 2003, 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * ipmi_thres_test.c
 *
 * Authors:
 *     Bill Barkely 
 *     Andy Cress <arcress@users.sourceforge.net>
 * Changes:
 * 02/26/04 ARCress - added general search for any Fan sensor.
 * 03/17/04 ARCress - changed to Temp sensor (always has Lower Major)
 * 11/03/2004  kouzmich   porting to HPI B
 *
 *     Copied from clients/hpithres.c and modified
 *
 * 11/18/2004  kouzmich   modified as ipmi test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <SaHpi.h>
#include <oh_utils.h>

typedef enum {
	PASS = 0,	// test passed
	FAIL,		// test failed
	UNTESTED	// error before testing
} TestStatus_T;

int fdebug = 0;
char progver[] = "1.1";
SaErrorT rv;
TestStatus_T status = FAIL;

char *rdrtypes[5] = {
	"None    ",
	"Control ",
	"Sensor  ",
	"Invent  ",
	"Watchdog"};

#define HPI_NSEC_PER_SEC 1000000000LL
#define NSU   32

char *units[NSU] = {
	"units", "deg C",     "deg F",     "deg K",     "volts", "amps",
	"watts", "joules",    "coulombs",  "va",        "nits",  "lumen",
	"lux",   "candela",   "kpa",       "psi",     "newton",  "cfm",
	"rpm",   "Hz",        "us",        "ms",      "sec",     "min",
	"hours", "days",      "weeks",     "mil",     "in",     "ft",
	"mm",    "cm"
};

static void print_value(SaHpiSensorReadingT *item, char *mes)
{
	char *val;

	if (item->IsSupported != SAHPI_TRUE)
		return;
	switch (item->Type) {
		case SAHPI_SENSOR_READING_TYPE_INT64:
			printf("%s %lld\n", mes, item->Value.SensorInt64);
			return;
		case SAHPI_SENSOR_READING_TYPE_UINT64:
			printf("%s %llu\n", mes, item->Value.SensorUint64);
			return;
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
			printf("%s %10.3f\n", mes, item->Value.SensorFloat64);
			return;
		case SAHPI_SENSOR_READING_TYPE_BUFFER:
			val = (char *)(item->Value.SensorBuffer);
			if (val != NULL)
				printf("%s %s\n", mes, val);
			return;
	}
}

static void Set_value(SaHpiSensorReadingT *item, SaHpiSensorReadingUnionT value)
{
	switch (item->Type) {
		case SAHPI_SENSOR_READING_TYPE_INT64:
			item->Value.SensorInt64 = value.SensorInt64;
			break;
		case SAHPI_SENSOR_READING_TYPE_UINT64:
			item->Value.SensorUint64 = value.SensorUint64;
			break;
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
			item->Value.SensorFloat64 = value.SensorFloat64;
			break;
		case SAHPI_SENSOR_READING_TYPE_BUFFER:
			printf("Set_value: Buffer type is not supported\n");
			break;
	}
}

static void Mul_value(SaHpiSensorReadingT *item, SaHpiFloat64T value)
{
	switch (item->Type) {
		case SAHPI_SENSOR_READING_TYPE_INT64:
			item->Value.SensorInt64 =
				(SaHpiInt64T)((SaHpiFloat64T)(item->Value.SensorInt64) * value);
			break;
		case SAHPI_SENSOR_READING_TYPE_UINT64:
			item->Value.SensorUint64 =
				(SaHpiUint64T)((SaHpiFloat64T)(item->Value.SensorUint64) * value);
			break;
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
			item->Value.SensorFloat64 *= value;
			break;
		case SAHPI_SENSOR_READING_TYPE_BUFFER:
			printf("Mul_value: Buffer type is not supported\n");
			break;
	}
}

static void Sum_value(SaHpiSensorReadingT *item, SaHpiFloat64T value)
{
	switch (item->Type) {
		case SAHPI_SENSOR_READING_TYPE_INT64:
			item->Value.SensorInt64 =
				(SaHpiInt64T)((SaHpiFloat64T)(item->Value.SensorInt64) + value);
			break;
		case SAHPI_SENSOR_READING_TYPE_UINT64:
			item->Value.SensorUint64 =
				(SaHpiUint64T)((SaHpiFloat64T)(item->Value.SensorUint64) + value);
			break;
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
			item->Value.SensorFloat64 += value;
			break;
		case SAHPI_SENSOR_READING_TYPE_BUFFER:
			printf("Sum_value: Buffer type is not supported\n");
			break;
	}
}

static void ShowThresh(SaHpiSensorThresholdsT *sensbuff)
{
	printf("    Supported Thresholds:\n");
	print_value(&(sensbuff->LowCritical), "      Lower Critical Threshold:");
	print_value(&(sensbuff->LowMajor), "      Lower Major Threshold:");
	print_value(&(sensbuff->LowMinor), "      Lower Minor Threshold:");
	print_value(&(sensbuff->UpCritical), "      Upper Critical Threshold:");
	print_value(&(sensbuff->UpMajor), "      Upper Major Threshold:");
	print_value(&(sensbuff->UpMinor), "      Upper Minor Threshold:");
	print_value(&(sensbuff->PosThdHysteresis), "      Positive Threshold Hysteresis:");
	print_value(&(sensbuff->NegThdHysteresis), "      Negative Threshold Hysteresis:");
	printf("\n");
}

static TestStatus_T DoEvent(
   SaHpiSessionIdT sessionid,
   SaHpiResourceIdT resourceid,
   SaHpiSensorRecT *sensorrec )
{
	SaHpiSensorNumT sensornum;
	SaHpiSensorReadingT reading;
	SaHpiSensorReadingT item;
	SaHpiSensorThresholdsT senstbuff1;
	SaHpiSensorThresholdsT senstbuff2;
	SaHpiSensorThresholdsT senstbuff3;
	SaHpiTimeoutT timeout = (SaHpiInt64T)(10 * HPI_NSEC_PER_SEC); /* 10 seconds */
	SaHpiEventT event;
	SaHpiRptEntryT rptentry;
	SaHpiRdrT rdr;
	char *unit;
	TestStatus_T stat = PASS;
	int i;

	sensornum = sensorrec->Num;

	/* Get current sensor reading */

	rv = saHpiSensorReadingGet(sessionid, resourceid, sensornum, &reading, NULL);
	if (rv != SA_OK)  {
		printf("saHpiSensorReadingGet: %s\n", oh_lookup_error(rv));
		return(FAIL);
	}

	/* Determine units of interpreted reading */

	i = sensorrec->DataFormat.BaseUnits;

	if (i > NSU)
		i = 0;
	unit = units[i];

	print_value(&reading, unit);

	/* Retrieve current threshold setings, twice */
	/* once for backup and once for modification */

	/* Get backup copy */
	rv = saHpiSensorThresholdsGet(sessionid, resourceid, sensornum, &senstbuff1);
	if (rv != SA_OK) {
		printf("saHpiSensorThresholdsGet: %s\n", oh_lookup_error(rv));
		return(FAIL);
	}

	/* Get modification copy */
	rv = saHpiSensorThresholdsGet(sessionid, resourceid, sensornum, &senstbuff2);
	if (rv != SA_OK) {
		printf("saHpiSensorThresholdsGet: %s\n", oh_lookup_error(rv));
		return(FAIL);
	}

	/* Display current thresholds */ 
	printf("   Current thresholds:\n");
	ShowThresh(&senstbuff2);

	senstbuff2.LowMajor.IsSupported = 1;

	/* Set new threshold to current reading + 10% */
	switch (senstbuff2.LowMajor.Type) {
		case SAHPI_SENSOR_READING_TYPE_INT64:
			senstbuff2.LowMajor.Value.SensorInt64 =
        			reading.Value.SensorInt64 * 1.10;
			break;
		case SAHPI_SENSOR_READING_TYPE_UINT64:
			senstbuff2.LowMajor.Value.SensorFloat64 =
        			reading.Value.SensorUint64 * 1.10;
			break;
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
			senstbuff2.LowMajor.Value.SensorFloat64 =
        			reading.Value.SensorFloat64 * 1.10;
			break;
		case SAHPI_SENSOR_READING_TYPE_BUFFER:
			printf("Buffer type is not supported\n");
			break;
	};
	item = reading;
	Mul_value(&item, (SaHpiFloat64T)1.10);
	Set_value(&(senstbuff2.LowMajor), item.Value);

	/* In this case, LowMinor > LowMajor */

	senstbuff2.LowMinor.IsSupported = 1;
	switch (senstbuff2.LowMinor.Type) {
		case SAHPI_SENSOR_READING_TYPE_INT64:
			senstbuff2.LowMinor.Value.SensorInt64 =
        			reading.Value.SensorInt64 * (SaHpiInt64T)1.10 + 10;
			break;
		case SAHPI_SENSOR_READING_TYPE_UINT64:
			senstbuff2.LowMinor.Value.SensorFloat64 =
        			reading.Value.SensorUint64 * (SaHpiUint64T)1.10 + 10;
			break;
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
			senstbuff2.LowMinor.Value.SensorFloat64 =
        			reading.Value.SensorFloat64 * (SaHpiFloat64T)1.10 + 10;
			break;
		case SAHPI_SENSOR_READING_TYPE_BUFFER:
			printf("Buffer type is not supported\n");
			break;
	};

	item = reading;
	Mul_value(&item, (SaHpiFloat64T)1.10);
	Sum_value(&item, (SaHpiFloat64T)10);
	Set_value(&(senstbuff2.LowMinor), item.Value);
   
	printf("   New\n");
	ShowThresh(&senstbuff2);

	/* Subscribe to New Events, only */
	printf("Subscribe to events\n");
	rv = saHpiSubscribe(sessionid);
	if (rv != SA_OK) {
		printf("saHpiSubscribe: %s\n", oh_lookup_error(rv));
		return(UNTESTED);
	}

	/* Set new thresholds */
	printf("Set new thresholds\n");

	rv = saHpiSensorThresholdsSet(sessionid, resourceid, sensornum, &senstbuff2);
	if (rv != SA_OK) {
		printf("saHpiSensorThresholdsSet: %s\n", oh_lookup_error(rv));
		printf("Unsubscribe\n");
		saHpiUnsubscribe(sessionid);
		return(FAIL);
	}

	/* Go wait on event to occur */
	printf("Go and get the event\n");
	for (;;) {
		rv = saHpiEventGet(sessionid, timeout, &event, &rdr, &rptentry, NULL);
		if (rv != SA_OK) { 
			if (rv != SA_ERR_HPI_TIMEOUT) {
				printf("Error during EventGet\n");
				stat = FAIL;
			};
			break;
		}

		if (event.EventType == SAHPI_ET_SENSOR) {
			printf("Sensor # = %2d  Severity = %2x\n", 
				event.EventDataUnion.SensorEvent.SensorNum, event.Severity );
			if (event.EventDataUnion.SensorEvent.SensorNum == sensornum) {
				printf("Got it\n");
				break;
			}
		}
	}

	if (stat != FAIL) {
		saHpiSensorThresholdsGet(sessionid, resourceid, sensornum, &senstbuff3);
		printf("   Current thresholds:\n");
		ShowThresh(&senstbuff3);
		if (memcmp(&senstbuff3, &senstbuff2, sizeof(SaHpiSensorThresholdsT)) != 0) {
			printf("Invalid current threshold!\n");
			stat = FAIL;
		}
	}
	
	/* Reset to the original thresholds */
	printf("Reset thresholds\n");
	rv = saHpiSensorThresholdsSet(sessionid, resourceid, sensornum, &senstbuff1);
	if (rv != SA_OK) {
		printf("saHpiSensorThresholdsSet: %s\n", oh_lookup_error(rv));
	} else {
		/* Re-read threshold values */
		rv = saHpiSensorThresholdsGet(sessionid, resourceid, sensornum, &senstbuff2);
		if (rv != SA_OK) {
			printf("saHpiSensorThresholdsGet: %s\n", oh_lookup_error(rv));
		} else {
			/* Display reset thresholds */ 
			printf("   Reset thresholds:\n");
			ShowThresh(&senstbuff2);
		}
	}
	
	printf("Unsubscribe\n");
	saHpiUnsubscribe(sessionid);
	return (stat);
}  /*end DoEvent*/

static TestStatus_T test(int argc, char **argv)
{
	int			c;
	SaHpiSessionIdT		sessionid;
	SaHpiDomainInfoT	domainInfo;
	SaHpiRptEntryT		rptentry;
	SaHpiEntryIdT		rptentryid;
	SaHpiEntryIdT		nextrptentryid;
	SaHpiEntryIdT		entryid;
	SaHpiEntryIdT		nextentryid;
	SaHpiResourceIdT	resourceid;
	SaHpiRdrT		rdr;
	TestStatus_T		result = PASS;

	printf("%s  ver %s\n", argv[0], progver);
	while ( (c = getopt( argc, argv,"x?")) != EOF )
		switch(c)  {
			case 'x':
				fdebug = 1;
				break;
			default:
				printf("Usage: %s [-x]\n", argv[0]);
				printf("   -x  Display debug messages\n");
				return (UNTESTED);
		}

	rv = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID ,&sessionid, NULL);

	if (rv != SA_OK) {
		printf("saHpiSessionOpen: %s\n", oh_lookup_error(rv));
		return (UNTESTED);
	}
 
	rv = saHpiDiscover(sessionid);

	if (fdebug) printf("saHpiDiscover: %s\n", oh_lookup_error(rv));

	rv = saHpiDomainInfoGet(sessionid,&domainInfo);

	if (fdebug) printf("saHpiDomainInfoGet: %s\n", oh_lookup_error(rv));
	if (fdebug) printf("DomainInfo: UpdateCount = %x, UpdateTime = %lx\n",
		domainInfo.RptUpdateCount, (unsigned long)domainInfo.RptUpdateTimestamp);
 
	/* walk the RPT list */
	rptentryid = SAHPI_FIRST_ENTRY;
	while ((result == PASS) && (rptentryid != SAHPI_LAST_ENTRY)) {
		rv = saHpiRptEntryGet(sessionid, rptentryid, &nextrptentryid, &rptentry);
		if (rv != SA_OK)
			break;
		/* walk the RDR list for this RPT entry */
		entryid = SAHPI_FIRST_ENTRY;
		rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0;
		resourceid = rptentry.ResourceId;
      
		if (fdebug) printf("rptentry[%d] resourceid=%d\n", rptentryid,resourceid);
		printf("Resource Tag: %s\n", rptentry.ResourceTag.Data);

		while ((result == PASS) && (entryid != SAHPI_LAST_ENTRY)) {
			rv = saHpiRdrGet(sessionid, resourceid, entryid, &nextentryid, &rdr);

			if (fdebug) printf("saHpiRdrGet[%d]: %s\n",
				entryid, oh_lookup_error(rv));

			if (rv != SA_OK) {
				printf( "saHpiRdrGet: Returned HPI Error: %s\n", oh_lookup_error(rv));
				break;
			};
			if (rdr.RdrType == SAHPI_SENSOR_RDR) { 
				/*type 2 includes sensor and control records*/
				rdr.IdString.Data[rdr.IdString.DataLength] = 0;
				printf("%02d %s\t", rdr.RecordId, rdr.IdString.Data);
				result = DoEvent(sessionid, resourceid,
						&rdr.RdrTypeUnion.SensorRec);
				if (result != PASS) {
					printf("Returned Error from DoEvent\n");
					break;
				}
			} 
			entryid = nextentryid;
		}  /*while rdr loop */

		rptentryid = nextrptentryid;
	}  /*while rpt loop*/
	saHpiSessionClose(sessionid);
	return(result);
}

int main(int argc, char **argv)
{
	status = test(argc, argv);
	switch (status) {
		case PASS:
			printf("	Test PASSED\n");
			break;
		case FAIL:
			printf("	Test FAILED\n");
			break;
		case UNTESTED:
			printf("	Test UNTESTED (error before testing)\n");
			break;
		default:
			printf("	UNKNOWN ERROR!\n");
	}
	return (status == PASS ? 0 : 1);
}
/* end ipmi_thres_test.c */
