/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	An item is read from the input, unless the conversion specification
 *	includes an n conversion specifier
 *
 * method:
 *	-open file in write mode
 *	-write a sample integer and sample string into the file
 *	-close the file
 *	-open the same file in read mode
 *	-read data from the file using fscanf() and conversion specifier "%n"
 *	-check if the return value of fscanf is 0
 *	-rewind file pointer and read data from file again but this time without
 *	 "%n" conversion specifier
 *	-check the return value of fscanf and compare the values read into the
 *	 arguments with input values
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TNAME "fscanf/9-1.c"
#define FNAME "in_file"
#define STRING_CONST "LINUX"
#define INTEGER_CONST 32

static void remove_file(FILE *in_fp)
{
	errno = 0;
	if (in_fp != NULL) {
		if (fclose(in_fp) == EOF) {
			printf(TNAME " Error at fclose(), errno = %d\n", errno);
			unlink(FNAME);
			exit(PTS_UNRESOLVED);
		}
	}
	unlink(FNAME);
}

int main(void)
{
	FILE *in_fp = NULL;
	char sample_string[strlen(STRING_CONST)];
	int sample_integer = 0, byte_count = 0, ret = 0;

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%s %d", STRING_CONST, INTEGER_CONST);
	if (ret < 0) {
		printf(TNAME " Error at fprintf()\n");
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	if (fclose(in_fp) == EOF) {
		printf(TNAME " Error at fclose(), errno = %d\n", errno);
		unlink(FNAME);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	in_fp = fopen(FNAME, "r");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	// When conversion specifier is %n only, the return value should be 0
	errno = 0;
	ret = fscanf(in_fp, "%n", &byte_count);
	if (ret == 0) {
		printf(TNAME " When n conversion specifier is used no input is "
			"consumed\n");
	} else {
		printf(TNAME
			"Test Failled. Expected return value of fscanf = 0 "
			"but obtained value = %d\n", ret);
		exit(PTS_FAIL);
		remove_file(in_fp);
	}

	// When conversion specifier does not contain %n, ret = number of
	// inputs successfully read
	errno = 0;
	ret = fscanf(in_fp, "%s %d", sample_string, &sample_integer);
	remove_file(in_fp);
	if (ret == 2 && (strcmp(sample_string, STRING_CONST) == 0) &&
	    sample_integer == INTEGER_CONST) {
		printf(TNAME " When n conversion specifier is not used input is"
			" consumed\n");
		printf(TNAME " Test Passed\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed\n");
		printf(TNAME "  Expected values: ret = 2, sample_string = %s, "
			"sample_integer = %d\n\t\t\tObtained values: ret = %d, "
			"sample_string = %s, sample_integer = %d\n",
			STRING_CONST, INTEGER_CONST, ret, sample_string,
			sample_integer);
		exit(PTS_FAIL);
	}
}
