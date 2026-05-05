/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	When the length of the input item ( i.e. the longest sequence of input
 *	bytes which is an initial subsequence of a matching sequence) is 0
 *	(when EOF,encoding error or read error has not occurred), the execution
 *	of the conversion specification fails and it is a matching failure.
 *
 * method:
 *	-open file in write mode
 *	-write 1 sample string into a file
 *	-close the file
 *	-open the same file in read mode
 *	-read data from the file using fscanf() and "%d" conversion specifier
 *	-check the return value of fscanf, errno and feof() return value
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "fscanf/10-1.c"
#define FNAME "in_file"
#define STR_CONST "POSIX CONFORMANCE TEST"

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
	int sample_value = 0;
	int ret = 0;

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error in fopen(), errno: %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%s", STR_CONST);
	if (ret < 0) {
		printf(TNAME " Error in fprintf()\n");
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
		printf(TNAME " Error in fopen(), errno = %d\n", errno);
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	ret = fscanf(in_fp, "%d", &sample_value);
	if (ret == 0 && errno == 0 && feof(in_fp) == 0) {
		printf(TNAME " Test Passed\n");
		remove_file(in_fp);
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed\n");
		printf(TNAME
			" Expected values: ret = 0, errno = 0, feof(in_fp) = 0\n\t\t\t"
			"Obtained values: ret = %d, errno = %d, feof(in_fp) = %d\n",
			ret, errno, feof(in_fp));
		remove_file(in_fp);
		exit(PTS_FAIL);
	}
}
