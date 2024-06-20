/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *  When “%n$” conversion specifier is used, “%%” and “%*”
 *  specifiers are permissible.
 *
 * method:
 *  -open file in write mode
 *  -write 3 long long integers into the file
 *  -close the file
 *  -open the same file in read mode
 *  -read the data from the file using %n$ and %* as directives in fscanf
 *  -rewind the file
 *  -read the data from the file using %n$ and %% as directives in fscanf
 *  -compare the values
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TNAME "fscanf/3-1.c"
#define FNAME "in_file"
#define INT_CONST 123
#define FLOAT_CONST 456.123789F
#define CONST_NUM 789

static void remove_file(FILE *in_fp)
{
	errno = 0;
	if (in_fp != NULL) {
		if (fclose(in_fp) != 0) {
			printf(TNAME " Error in closing the file, errno = %d\n",
					errno);
			unlink(FNAME);
			exit(PTS_UNRESOLVED);
		}
	}
	unlink(FNAME);
}

int main(void)
{
	FILE *in_fp;
	int ret;
	int int_var = 0, var1 = 0;

	errno = 0;
	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error in opening file for writing, errno = %d\n",
				errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%d %f %d %%", INT_CONST, FLOAT_CONST, CONST_NUM);
	if (ret < 0) {
		printf(TNAME " Error in writing to the file\n");
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	ret = fclose(in_fp);
	if (ret != 0) {
		printf(TNAME
				" Error in closing the file descriptor errno = %d\n",
				errno);
		unlink(FNAME);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	in_fp = fopen(FNAME, "r");
	if (in_fp == NULL) {
		printf(TNAME
				" Error in opening the file for reading, errno = %d\n",
				errno);
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	ret = fscanf(in_fp, "%1$d %*f", &int_var);
	if (ret != 1 && feof(in_fp) == 0 && errno == 0) {
		printf(TNAME " Error: %s\n", strerror(errno));
		remove_file(in_fp);
		exit(PTS_FAIL);
	}

	if (int_var == INT_CONST) {
		printf(TNAME " Test passed for %%*\n");
	} else {
		printf(TNAME
				" Test Failed for %%*. Expected integer value: %d got %d",
				INT_CONST, int_var);
		exit(PTS_FAIL);
	}

	errno = 0;
	ret = fscanf(in_fp, "%1$d %%", &var1);
	remove_file(in_fp);
	if (ret != 1 && feof(in_fp) == 0 && errno == 0) {
		printf(TNAME " Error: %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	if (var1 == CONST_NUM) {
		printf(TNAME " Test Passed for %%%%\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed for \%\%. Expected var1: %d got %d",
						CONST_NUM, var1);
		exit(PTS_FAIL);
	}
}
