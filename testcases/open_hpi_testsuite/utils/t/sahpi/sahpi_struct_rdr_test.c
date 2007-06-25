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
	SaHpiBoolT	testFail = SAHPI_FALSE;
																																	
	SaHpiRdrT	thisRdr = {
		.RecordId = 88,
		.RdrType = SAHPI_NO_RECORD,
		.Entity =  {
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
		.IsFru = SAHPI_TRUE,
		/* .RdrTypeUnion == init later for individual rdr type */
		.IdString = {
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 32, /* Incorrectly set on purpose */
                        .Data = "This is a oh_fprint_rdr test!"
                }
        };

	memset(&thisRdr.RdrTypeUnion, 0, sizeof(SaHpiRdrTypeUnionT));

	FILE *fp;
	const char *name = "/tmp/idrareatmp";
	const char *mode = "a";

	fp = fopen(name, mode);
	if (fp == NULL) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		return -1;
	}

	/* ------------------------------------------------ */
	/* NULL Pointer tests                               */
	/* ------------------------------------------------ */
	
	err = oh_fprint_rdr(NULL , &thisRdr, 0);
	if (err != SA_ERR_HPI_INVALID_PARAMS) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}
	
	err = oh_fprint_rdr(fp, NULL, 1);
	if (err != SA_ERR_HPI_INVALID_PARAMS) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}
	
	err = oh_fprint_rdr(NULL , NULL, 2);
	if (err != SA_ERR_HPI_INVALID_PARAMS) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}

	/* ------------------------------------------------ */
	/* Normal write to file test                        */
	/* ------------------------------------------------ */
		
	err = oh_fprint_rdr(fp, &thisRdr, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}
	/* ------------------------------------------------ */
	/* Normal write to stdout test                      */
	/* ------------------------------------------------ */
	
	err = oh_print_rdr(&thisRdr, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}
	/* ------------------------------------------------ */
	/* Normal write SAHPI_CTRL_RDR stdout test          */
	/* ------------------------------------------------ */
	
	thisRdr.RdrType = SAHPI_CTRL_RDR;
	SaHpiCtrlRecT   thisctrlrec = {
		.Num = 102,
		.OutputType = SAHPI_CTRL_POWER_INTERLOCK,
		.Type = SAHPI_CTRL_TYPE_DIGITAL,
		.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_ON,
		.DefaultMode = {
			.Mode= SAHPI_CTRL_MODE_AUTO,
			.ReadOnly = SAHPI_TRUE
		},
		.WriteOnly = SAHPI_FALSE,
		.Oem = 0	
	};
	
	memcpy(&thisRdr.RdrTypeUnion.CtrlRec, &thisctrlrec, sizeof(SaHpiCtrlRecT));
	err = oh_print_rdr(&thisRdr, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}
	/* ------------------------------------------------ */
	/* Normal write SAHPI_SENSOR_RDR stdout test        */
	/* ------------------------------------------------ */
	
	thisRdr.RdrType = SAHPI_SENSOR_RDR;
        SaHpiSensorRecT  thissensorrec = {
		.Num = 1,
		.Type = SAHPI_VOLTAGE,
		.Category = SAHPI_EC_THRESHOLD,
		.Events = SAHPI_ES_LOWER_MAJOR | SAHPI_ES_LOWER_MINOR,
		.EventCtrl = SAHPI_SEC_READ_ONLY,
		.DataFormat = {
			.IsSupported = SAHPI_TRUE,
			.ReadingType = SAHPI_SENSOR_READING_TYPE_INT64,
			.BaseUnits = SAHPI_SU_VOLTS,
			.ModifierUnits = SAHPI_SU_SECOND,
			.ModifierUse = SAHPI_SMUU_BASIC_TIMES_MODIFIER,
			.Percentage = SAHPI_FALSE,
			.Range = {
				.Flags = SAHPI_SRF_MIN | SAHPI_SRF_MAX | SAHPI_SRF_NOMINAL |
			                        SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX,
				.Min = {
					.IsSupported = SAHPI_TRUE,
					.Type = SAHPI_SENSOR_READING_TYPE_INT64,
					.Value.SensorInt64 = 0
				},
				.Max = {
					.IsSupported = SAHPI_TRUE,
					.Type = SAHPI_SENSOR_READING_TYPE_INT64,
					.Value.SensorInt64 = 100
				},
				.Nominal = {
					.IsSupported = SAHPI_TRUE,
					.Type = SAHPI_SENSOR_READING_TYPE_INT64,
					.Value.SensorInt64 = 50
				},
				.NormalMax = {
					.IsSupported = SAHPI_TRUE,
					.Type = SAHPI_SENSOR_READING_TYPE_INT64,
					.Value.SensorInt64 = 75
				},
				.NormalMin = {
					.IsSupported = SAHPI_TRUE,
					.Type = SAHPI_SENSOR_READING_TYPE_INT64,
					.Value.SensorInt64 = 25
				}
			},
			.AccuracyFactor = 0.05
		},
		.Oem = 0xFF,
		.ThresholdDefn = {
			.IsAccessible = SAHPI_TRUE,
			.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_LOW_MAJOR | SAHPI_STM_LOW_CRIT | SAHPI_STM_LOW_HYSTERESIS,
			.WriteThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_MAJOR | SAHPI_STM_UP_CRIT | SAHPI_STM_UP_HYSTERESIS,
			.Nonlinear = SAHPI_TRUE
		}	
	};

	memcpy(&thisRdr.RdrTypeUnion.SensorRec, &thissensorrec, sizeof(SaHpiSensorRecT));		
	err = oh_print_rdr(&thisRdr, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}

	/* ------------------------------------------------ */
	/* Normal write SAHPI_INVENTORY_RDR stdout test     */
	/* ------------------------------------------------ */
	thisRdr.RdrType = SAHPI_INVENTORY_RDR;
        SaHpiInventoryRecT  thisinvrec = {
		.IdrId = 22,
		.Persistent = SAHPI_FALSE,
		.Oem = 32
	};
	
	memcpy(&thisRdr.RdrTypeUnion.InventoryRec, &thisinvrec, sizeof(SaHpiInventoryRecT));			
	err = oh_print_rdr(&thisRdr, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}

	/* ------------------------------------------------ */
	/* Normal write SAHPI_WATCHDOG_RDR stdout test      */
	/* ------------------------------------------------ */
	thisRdr.RdrType = SAHPI_WATCHDOG_RDR;
        SaHpiWatchdogRecT  thiswdogrec = {
		.WatchdogNum = 42,
		.Oem = 52
	};

	
	memcpy(&thisRdr.RdrTypeUnion.WatchdogRec, &thiswdogrec, sizeof(SaHpiWatchdogRecT));
	err = oh_print_rdr(&thisRdr, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}

	/* ------------------------------------------------ */
	/* Normal write SAHPI_ANNUNCIATOR_RDR stdout test   */
	/* ------------------------------------------------ */
	thisRdr.RdrType = SAHPI_ANNUNCIATOR_RDR;
        SaHpiAnnunciatorRecT thisannrec = {
		.AnnunciatorNum = 62,
		.AnnunciatorType = SAHPI_ANNUNCIATOR_TYPE_LED,
		.ModeReadOnly = SAHPI_TRUE,
		.MaxConditions= 1,
		.Oem = 72
	};
	
	memcpy(&thisRdr.RdrTypeUnion.AnnunciatorRec, &thisannrec, sizeof(SaHpiAnnunciatorRecT));	
	err = oh_print_rdr(&thisRdr, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		testFail = SAHPI_TRUE;
	}

	/* ------------------------------------------------ */
	/* Write to invalid file handle test                */
	/* ------------------------------------------------ */
	/* TBD */

	fclose(fp);
	unlink(name);

	if (testFail)
		return(-1);
	else 
		return(0);
}

