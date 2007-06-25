/*      -*- linux-c -*-
 *
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
 *      pdphan	<pdphan@users.sf.org>
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <SaHpi.h>
#include <oh_utils.h>

int main(int argc, char **argv) 
{
	SaErrorT   	err;
	SaHpiBoolT	test1Fail = SAHPI_FALSE,
			test2Fail = SAHPI_FALSE,
			test3Fail = SAHPI_FALSE,
			test4Fail = SAHPI_FALSE,
			test5Fail = SAHPI_FALSE;
	SaHpiRptEntryT thisrptentry = {
		.EntryId = 82,
		.ResourceId = 101,
		.ResourceInfo = {
				.ResourceRev = 2,
				.SpecificVer = 22,
				.DeviceSupport = 222,
				.ManufacturerId = 5,
				.ProductId = 2222,
				.FirmwareMajorRev = 42,
				.FirmwareMinorRev = 52,
				.AuxFirmwareRev = 62,
		},
		.ResourceEntity = {
			.Entry[0] = {
				.EntityType = SAHPI_ENT_SUBBOARD_CARRIER_BLADE,
				.EntityLocation = 14
			},
			{
				.EntityType = SAHPI_ENT_SUB_CHASSIS,
				.EntityLocation = 15
			},
			{
				.EntityType = SAHPI_ENT_SYSTEM_CHASSIS,
				.EntityLocation = 16
			},
			{
				.EntityType = SAHPI_ENT_ROOT,
				.EntityLocation = 17
			}
		},
		.ResourceCapabilities  = (
			SAHPI_CAPABILITY_AGGREGATE_STATUS |
			SAHPI_CAPABILITY_CONFIGURATION |
			SAHPI_CAPABILITY_MANAGED_HOTSWAP | 
			SAHPI_CAPABILITY_WATCHDOG |
			SAHPI_CAPABILITY_CONTROL | 
			SAHPI_CAPABILITY_FRU |
			SAHPI_CAPABILITY_ANNUNCIATOR |
			SAHPI_CAPABILITY_POWER |
			SAHPI_CAPABILITY_RESET),	
		.HotSwapCapabilities = SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED,
		.ResourceSeverity = SAHPI_INFORMATIONAL,
		.ResourceFailed = SAHPI_FALSE,
                .ResourceTag = {
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 26, /* Incorrectly set on purpose */
                        .Data = "This is a test!"
                }
        };


	FILE *fp;
	const char *name = "/tmp/rptentrytmp";
	const char *mode = "a";

	fp = fopen(name, mode);
	if (fp == NULL) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
	}

	/* ------------------------------------------------ */
	/* NULL Pointer tests                               */
	/* ------------------------------------------------ */
	
	err = oh_fprint_rptentry(NULL , &thisrptentry, 0);
	if (err != SA_ERR_HPI_INVALID_PARAMS) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		test1Fail = SAHPI_TRUE;
	}
	
	err = oh_fprint_rptentry(fp, NULL, 3);
	if (err != SA_ERR_HPI_INVALID_PARAMS) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		test2Fail = SAHPI_TRUE;
	}
	
	err = oh_fprint_rptentry(NULL , NULL, 3);
	if (err != SA_ERR_HPI_INVALID_PARAMS) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		test3Fail = SAHPI_TRUE;
	}

	/* ------------------------------------------------ */
	/* Normal write to file test                        */
	/* ------------------------------------------------ */
	strncpy((char *)thisrptentry.ResourceInfo.Guid, "GUID-123", sizeof("GUID-123"));

	err = oh_fprint_rptentry(fp, &thisrptentry, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		test4Fail = SAHPI_TRUE;
	}
	/* ------------------------------------------------ */
	/* Normal write to stdout test                      */
	/* ------------------------------------------------ */
	
	err = oh_print_rptentry(&thisrptentry, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		test5Fail = SAHPI_TRUE;
	}

	/* ------------------------------------------------ */
	/* Write to invalid file handle test                */
	/* ------------------------------------------------ */
	/* TBD */

	fclose(fp);
	unlink(name);

	if (!test1Fail && !test2Fail &&
		!test3Fail && !test4Fail && !test5Fail) 
		return(0);
	else 
		return(-1);
}

