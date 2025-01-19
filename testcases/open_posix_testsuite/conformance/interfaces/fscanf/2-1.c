/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *  When “%n$” conversion specifier is used, conversion is applied to the
 *  nth argument after the format in the argument list (where n is a decimal
 *  integer in range [1,{NL_ARGMAX}]).
 *
 *  method:
 *   -open file in write mode
 *   -write 3 integers into the file
 *   -close the file
 *   -open the same file in read mode
 *   -read 2 integers from the file using "%n$", order for reading the arguments
 *    is: specifying "%3$d" ensures the first input is fed to the third argument
 *    in the argument list and "%1$s" ensures the next input is given to the
 *    first argument in the argument list
 *   -check the values in the arguments specified as described above
 *
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <float.h>
#include <math.h>
#include "posixtest.h"

#define TNAME "fscanf/2-1.c"
#define FNAME "in_file"
#define INT_CONST 123
#define FLOAT_CONST 456.123789F
#define max(a, b) (((a) > (b)) ? (a) : (b))

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
	int int_var = 0;
	float float_var = 0.0F;
	FILE *in_fp;
	int ret;

	errno = 0;
	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error in opening file for writing, errno = %d\n",
				errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%d %f", INT_CONST, FLOAT_CONST);
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
	ret = fscanf(in_fp, "%2$d %1$f", &float_var, &int_var);
	remove_file(in_fp);
	if (ret == 2 &&
		(fabs(float_var - FLOAT_CONST) <
		DBL_EPSILON * max(fabs(float_var), fabs(FLOAT_CONST))) &&
		int_var == INT_CONST) {
		printf(TNAME " Test Passed\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME
				" Test Failed with values as Expected:\t ret: %d got %d"
				"\n\t\tInteger value: %d got %d \n\t\t Float value: %f"
				" got %f", 2, ret, INT_CONST, int_var, FLOAT_CONST,
						float_var);
		exit(PTS_FAIL);
	}

}
