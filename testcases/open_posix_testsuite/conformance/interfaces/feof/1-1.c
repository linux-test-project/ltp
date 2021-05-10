/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	The feof function shall test the end-of-file indicator for
 *	the stream pointed to by argument and the function shall return
 *	non-zero if and only if the end-of-file indicator is set for stream
 *
 * method:
 *	-open file in write mode
 *	-write 2 sample strings into the file
 *	-close the file
 *	-open the same file in read mode
 *	-read first string from file and check feof return value
 *	-read second string from file and check feof return value
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define BUF_SIZE 1024
#define TNAME "feof/1-1.c"
#define STR_CONST1 "LINUX"
#define STR_CONST2 "POSIX"

int main(void)
{
	FILE *in_fp;
	char sample_string1[sizeof(STR_CONST1)];
	char sample_string2[sizeof(STR_CONST2)];
	char file_str[BUF_SIZE];
	char *filename_p;
	int ret;
	int error_val;

	filename_p = tmpnam(file_str);
	if (filename_p == NULL) {
		printf(TNAME " Error in generating tmp file name\n");
		error_val = PTS_UNRESOLVED;
		goto exit_test;
	}

	in_fp = fopen(filename_p, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		error_val = PTS_UNRESOLVED;
		goto exit_test;
	}

	ret = fprintf(in_fp, "%s %s", STR_CONST1, STR_CONST2);
	if (ret < 0) {
		printf(TNAME " Error at fprintf()\n");
		error_val = PTS_UNRESOLVED;
		goto remove_and_exit;
	}

	errno = 0;
	if (fclose(in_fp) == EOF) {
		printf(TNAME " Error at fclose(), errno = %d\n", errno);
		error_val = PTS_UNRESOLVED;
		goto remove_and_exit;
	}

	errno = 0;
	in_fp = fopen(filename_p, "r");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		error_val = PTS_UNRESOLVED;
		goto remove_and_exit;
	}

	errno = 0;
	ret = fscanf(in_fp, "%s", sample_string1);
	if (ret != 1) {
		printf(TNAME " Unexpected error at fscanf, errno = %d\n",
				errno);
		error_val = PTS_UNRESOLVED;
		goto remove_and_exit;
	}

	if (feof(in_fp)) {
		printf(TNAME " Test Failed. Expected feof() return "
				"value as zero as EOF has not reached, but "
				"obtained a non-zero value.\n");
		error_val = PTS_FAIL;
		goto remove_and_exit;
	}

	ret = fscanf(in_fp, "%s", sample_string2);
	if (ret != 1) {
		printf(TNAME " Unexpected error at fscanf, "
			"errno = %d\n", errno);
		error_val = PTS_UNRESOLVED;
		goto remove_and_exit;
	}

	if (feof(in_fp)) {
		printf(TNAME " Test Passed.\n");
		error_val = PTS_PASS;
		goto remove_and_exit;
	} else {
		printf(TNAME " Test Failed. Expected feof()"
			" return value as non-zero as EOF "
			"has reached, but obtained a zero.\n");
		error_val = PTS_FAIL;
		goto remove_and_exit;
	}

remove_and_exit:
	(void) unlink(filename_p);
exit_test:
	exit(error_val);
}
