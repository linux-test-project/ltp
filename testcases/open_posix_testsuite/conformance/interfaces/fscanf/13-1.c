/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	When assignment suppression is indicated by "*", the result of the
 *	conversion introduced by "%n$" is not stored in any object.
 *
 * method:
 *	-open file in write mode
 *	-write a string and an integer into the file
 *	-close the file
 *	-open the same file in read mode
 *	-read the data from the file using fscanf(),
 *	but integer with "*" assignment suppression
 *	-check the integer value
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#define TNAME "fscanf/13-1.c"
#define FNAME "in_file"
#define INT_CONST 10
#define STR_CONST "JOHN"

static void remove_file(FILE *in_fp)
{
	errno = 0;
	if (in_fp != NULL)
		if (fclose(in_fp) != 0) {
			output(TNAME " Error in closing the file, errno = %d\n",
					errno);
			unlink(FNAME);
			exit(PTS_UNRESOLVED);
		}
	unlink(FNAME);
}

int main(void)
{
	char sample_str[sizeof(STR_CONST)];
	int sample_int = 0;
	FILE *in_fp;
	int ret;

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		output(TNAME " Error in opening the file, errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%s %d", STR_CONST, INT_CONST);
	if (ret < 0) {
		output(TNAME " Error in writing into the file\n");
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	ret = fclose(in_fp);
	if (ret != 0) {
		output(TNAME " Error in closing the file, errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	in_fp = fopen(FNAME, "r");
	if (in_fp == NULL) {
		output(TNAME " Error in opening the file, errno = %d\n", errno);
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	ret = fscanf(in_fp, "%1$s %2$*d", sample_str, &sample_int);
	if (ret != 1) {
		output(TNAME
				" Unexpected return from fscanf. Expected 1 got %d, errno = %d\n",
				ret, errno);
		remove_file(in_fp);
		exit(PTS_FAIL);
	}

	remove_file(in_fp);

	if (sample_int != INT_CONST) {
		output(TNAME " Test Passed\n");
		exit(PTS_PASS);
	} else {
		output(TNAME " Test Failed\n");
		exit(PTS_FAIL);
	}
}
