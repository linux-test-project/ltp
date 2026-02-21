/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *  A directive composed of one or more white-space characters is executed
 *  by reading input until no more valid input can be read.
 *
 * method:
 *	-open file in write mode
 *	-write 2 strings and a character with one white-space between the
 *   strings and two white-spaces between string and character
 *	-close the file
 *	-open the same file in read mode
 *	-read the data from the file using fscanf()
 *   with wrong conversion specifier for third argument.
 *	-check the position of the file pointer
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define TNAME "fscanf/4-1.c"
#define FNAME "in_file"
#define STR_CONST_1 "JOHN"
#define STR_CONST_2 "LINUX"
#define CHAR_CONST 'A'

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
	char sample_str_1[sizeof(STR_CONST_1)];
	char sample_str_2[sizeof(STR_CONST_2)];
	int sample_int;
	FILE *in_fp;
	int ret, read_values;
	int file_ptr_pos, expected_file_ptr_pos;

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error in opening the file, errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%s %s  %c", STR_CONST_1, STR_CONST_2, CHAR_CONST);
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
	read_values = fscanf(in_fp, "%s %s %d",
						sample_str_1, sample_str_2,
						&sample_int);
	if (read_values != 2) {
		printf(TNAME
				" Unexpected return from fscanf. Expected 2 got %d, errno = %d\n",
			   ret, errno);
		remove_file(in_fp);
		exit(PTS_FAIL);
	}

	file_ptr_pos = ftell(in_fp);

	expected_file_ptr_pos = strlen(sample_str_1) + 1 + strlen(sample_str_2)
							+ 2;

	remove_file(in_fp);

	if (file_ptr_pos == expected_file_ptr_pos) {
		printf(TNAME " Test Passed\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME
				" Test Failed. Expected position for the file pointer is %d"
			   " considering the first string, a white space, second string and 2"
			   " whitespaces but current position is %d\n",
				expected_file_ptr_pos, file_ptr_pos);
		exit(PTS_FAIL);
	}
}
