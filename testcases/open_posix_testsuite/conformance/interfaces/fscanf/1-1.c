/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *  When there are excess arguments as compared to the directives in the
 *	control string format, they will be ignored.
 *
 * method:
 *  -open file in write mode
 *  -write 3 integers into the file
 *  -close the file
 *  -open the same file in read mode
 *  -read only 2 integers from the file using fscanf() while giving 3 pointer
 *   arguments,
 *  -third integer value will be ignored
 *  -check the 2 read integer values
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "posixtest.h"

#define BUF_SIZE 1024
#define TNAME "fscanf/1-1.c"
#define FNAME "/tmp/in_file"
#define VAR1 123
#define VAR2 456
#define VAR3 789

static void remove_file(char *file_name)
{
	errno = 0;
	(void)unlink(file_name);
}


int main(void)
{
	int var1, var2, var3;
	int ret, read_values;
	FILE *in_fp;
	char file_str[BUF_SIZE];

	snprintf(file_str, BUF_SIZE, "%s-%d", FNAME, getpid());
	errno = 0;
	in_fp = fopen(file_str, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error in opening file for writing, errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%d %d %d", VAR1, VAR2, VAR3);
	if (ret < 0) {
		printf(TNAME " Error in writing to the file\n");
		remove_file(file_str);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	ret = fclose(in_fp);
	if (ret != 0) {
		printf(TNAME " Error in closing the file descriptor, errno = %d\n",
				errno);
		unlink(file_str);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	in_fp = fopen(file_str, "r");
	if (in_fp == NULL) {
		printf(TNAME " Error in opening the file for reading, errno = %d\n",
				errno);
		remove_file(file_str);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-extra-args"
	read_values = fscanf(in_fp, "%d %d", &var1, &var2, &var3);
	#pragma GCC diagnostic pop
	if (read_values != 2) {
		printf(TNAME " Unexpected return from fscanf, read %d values from "
						"fscanf, errno = %d\n", read_values, errno);
		remove_file(file_str);
		exit(PTS_FAIL);
	}

	remove_file(file_str);

	if (var1 == VAR1 && var2 == VAR2) {
		printf(TNAME " Test Passed. Excess arguments are ignored.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed\n Expected values: var 1 = %d var2 = %d\n"
						"Observed values: var1 = %d var2 = %d", VAR1, VAR2,
						var1, var2);
		exit(PTS_FAIL);
	}
}
