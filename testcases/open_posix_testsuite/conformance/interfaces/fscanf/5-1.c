/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	A directive composed of one or more white-space characters is executed
 *	by reading input until up to the first byte which is not a white-space
 *	character, which remains unread.
 *
 * method:
 *	-open file in write mode
 *	-write 2 strings into the file with 2 white spaces in between
 *	-close the file
 *	-open the same file in read mode
 *	-read the data from the file using fscanf()
 *	with a white space in the directive
 *	-check the position of file pointer
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define TNAME "fscanf/5-1.c"
#define FNAME "in_file"
#define STR_CONST_1 "LINUX"
#define STR_CONST_2 "POSIX"

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
	int ret, read_values;
	char sample_str[sizeof(STR_CONST_1)];
	int file_ptr_pos, expected_file_ptr_pos;

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error in opening the file, errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%s  %s", STR_CONST_1, STR_CONST_2);
	if (ret < 0) {
		printf(TNAME " Error in writing into the file\n");
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	ret = fclose(in_fp);
	if (ret != 0) {
		printf(TNAME " Error in closing the file, errno = %d\n", errno);
		unlink(FNAME);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	in_fp = fopen(FNAME, "r");
	if (in_fp == NULL) {
		printf(TNAME " Error in opening the file, errno = %d\n", errno);
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	read_values = fscanf(in_fp, "%s ", sample_str);
	if (read_values != 1) {
		printf(TNAME
				" Unexpected return from fscanf. Expected 1 got %d, errno = %d\n",
				ret, errno);
		remove_file(in_fp);
		exit(PTS_FAIL);
	}

	file_ptr_pos = ftell(in_fp);

	expected_file_ptr_pos = strlen(sample_str) + 2;

	remove_file(in_fp);

	if (file_ptr_pos == expected_file_ptr_pos) {
		printf(TNAME " Test Passed\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME
				" Test Failed. Expected position for the file pointer is %d"
				" considering the first string read and 2 white-sapces but"
				" current position is %d\n",
				expected_file_ptr_pos, file_ptr_pos);
		exit(PTS_FAIL);
	}
}
