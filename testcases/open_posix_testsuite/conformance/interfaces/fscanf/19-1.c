/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	When "%hhx" specifier is specified, unsigned char(base argument is 16)
 *	is read from the named input stream in sequence, bytes are read in
 *	sequence and result is stored in the arguments. Function call expects
 *	a control string format and a set of pointer arguments indicating where
 *	the converted input is stored.
 *
 * method:
 *	-open file in write mode
 *	-write 3 ineger values corresponding to max value, min value and
 *	 mid range value of unsigned short short hexadecimal integer.
 *	-close the file
 *	-open the same file in read mode
 *	-read data from the file using fscanf() and conversion specifier "%hhx"
 *	-compare the values read into the arguments with input values
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "fscanf/19-1.c"
#define FNAME "in_file"
#define UCHAR_MIN_VALUE 0
#define UCHAR_CONST_VALUE 16

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
	unsigned char min_value = 0, max_value = 0, mid_value = 0;
	int ret = 0;

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%hhx %hhx %hhx", UCHAR_MIN_VALUE, UCHAR_MAX,
		      UCHAR_CONST_VALUE);
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

	errno = 0;
	ret = fscanf(in_fp, "%hhx %hhx %hhx",
				&min_value, &max_value, &mid_value);
	remove_file(in_fp);
	if (ret == 3 && min_value == UCHAR_MIN_VALUE &&
		max_value == UCHAR_MAX &&
	    mid_value == UCHAR_CONST_VALUE) {
		printf(TNAME " Test Passed\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed\n");
		printf(TNAME " Expected values: ret = 3, max_value = %hhx, "
			"min_value = %hhx, mid_value = %hhx\n\t\t\tObtained values: "
			"ret = %d, max_value = %hhx, min_value = %hhx, mid_value = %hhx\n",
			UCHAR_MAX, UCHAR_MIN_VALUE, UCHAR_CONST_VALUE, ret,
			max_value, min_value, mid_value);
		exit(PTS_FAIL);
	}
}
