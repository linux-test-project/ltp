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
	SaHpiIdrAreaHeaderT	thisIdrArea;
	SaHpiBoolT	test1Fail = SAHPI_FALSE,
			test2Fail = SAHPI_FALSE,
			test3Fail = SAHPI_FALSE,
			test4Fail = SAHPI_FALSE,
			test5Fail = SAHPI_FALSE;

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
	
	err = oh_fprint_idrareaheader(NULL , &thisIdrArea, 3);
	if (err != SA_ERR_HPI_INVALID_PARAMS) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		test1Fail = SAHPI_TRUE;
	}
	
	err = oh_fprint_idrareaheader(fp, NULL, 3);
	if (err != SA_ERR_HPI_INVALID_PARAMS) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		test2Fail = SAHPI_TRUE;
	}
	
	err = oh_fprint_idrareaheader(NULL , NULL, 3);
	if (err != SA_ERR_HPI_INVALID_PARAMS) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		test3Fail = SAHPI_TRUE;
	}

	/* ------------------------------------------------ */
	/* Normal write to file test                        */
	/* ------------------------------------------------ */
	
	thisIdrArea.AreaId = 3;
	thisIdrArea.Type = 1;
	thisIdrArea.ReadOnly = SAHPI_TRUE;
	thisIdrArea.NumFields = 1;
	
	err = oh_fprint_idrareaheader(fp, &thisIdrArea, 3);
	if (err != SA_OK) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		test4Fail = SAHPI_TRUE;
	}
	/* ------------------------------------------------ */
	/* Normal write to stdout test                      */
	/* ------------------------------------------------ */
	
	err = oh_print_idrareaheader(&thisIdrArea, 3);
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
