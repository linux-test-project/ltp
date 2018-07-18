/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	If the stream is valid, feof function shall not change the setting of
 *	`errno.
 *
 * method:
 *	-open file in write mode
 *	-write a sample string into the file
 *	-close the file
 *	-open the same file in read mode so that a valid stream is obtained
 *	-check feof return value and errno
 *	-read the  string from the file using fscanf
 *	-check feof return value and errno
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define BUF_SIZE 1024
#define TNAME "feof/2-1.c"
#define STR_CONST "LINUX"

int main(void)
{
	FILE *in_fp;
	char sample_string[sizeof(STR_CONST)];
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

	errno = 0;
	in_fp = fopen(filename_p, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		error_val = PTS_UNRESOLVED;
		goto remove_and_exit;
	}

	ret = fprintf(in_fp, "%s", STR_CONST);
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

	if (!feof(in_fp)) {
		ret = fscanf(in_fp, "%s", sample_string);
		if (ret != 1) {
			printf(TNAME " Unexpected error at fscanf, errno = %d\n", errno);
			error_val = PTS_UNRESOLVED;
			goto remove_and_exit;
		}
		/* set errno to an invalid value which feof cannot assume */
		errno = EDOM;
		/* If stream is valid, errno should not be set by feof */
		if (feof(in_fp) && errno == EDOM) {
			printf(TNAME " Test Passed.\n");
			error_val = PTS_PASS;
			goto remove_and_exit;
		} else {
			printf(TNAME " Test Failed.\n");
			error_val = PTS_FAIL;
			goto remove_and_exit;
		}
	} else {
		printf(TNAME " Test Failed.\n");
		error_val = PTS_FAIL;
		goto remove_and_exit;
	}

remove_and_exit:
	(void) unlink(filename_p);
exit_test:
	exit(error_val);
}
