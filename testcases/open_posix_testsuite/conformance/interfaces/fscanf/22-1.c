/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	When "%hd" specifier is specified, signed short integer(base argument
 *	10) is read from the named input stream in sequence, bytes are read in
 *	sequence and result is stored in the arguments. Function call expects
 *	a control string format and a set of pointer arguments indicating where
 *	the converted input is stored.
 *
 * method:
 *	-open file in write mode
 *	-write 3 sample integers into the file corresponding to min, max and
 *	 a midrange value of signed short decimal integer
 *	-close the file
 *	-open the same file in read mode
 *	-read data from the file using fscanf() and conversion specifier "%hd"
 *	-compare the values read into the arguments with input values
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "fscanf/22-1.c"
#define FNAME "in_file"
#define SHORT_CONST 1000

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
	signed short min_value = 0, max_value = 0, mid_value = 0;
	int ret = 0, read_values = 0;

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%d %d %d", SHRT_MAX, SHRT_MIN,
		      SHORT_CONST);
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
	read_values = fscanf(in_fp, "%hd %hd %hd ", &max_value, &min_value,
				 &mid_value);
	remove_file(in_fp);

	if (read_values == 3 && max_value == SHRT_MAX &&
		min_value == SHRT_MIN &&
		mid_value == SHORT_CONST) {
		printf(TNAME " Test Passed\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed\n");
		printf(TNAME " Expected values: ret = 3, max_value = %d, "
				"min_value = %d, mid_value = %d\n\t\t\tObtained values: "
				"ret = %d, max_value = %d, min_value = %d, mid_value = %d\n",
				SHRT_MAX, SHRT_MIN, SHORT_CONST,
				ret, max_value, min_value, mid_value);
		exit(PTS_FAIL);
	}
}
