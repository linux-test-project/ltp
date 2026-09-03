/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	When "%hi" specifier is specified, signed short integer(base argumnet
 *	is 0) is read from the named input stream in sequence,bytes are read
 *	in sequence and result is stored in the arguments. Function call
 *	expects a control string format and a set of pointer arguments
 *	indicating where the converted input is stored.
 *
 * method:
 *	-open a file in write mode
 *	-write 4 sample integers into the file: min and max values of
 *	 signed short integer, an octal value and a hexadecimal value.
 *	-close the file
 *	-open the same file in read mode
 *	-read data from the file using fscanf() and conversion specifier "%hi"
 *	-compare the values read into the arguments with input values
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "fscanf/23-1.c"
#define FNAME "in_file"
#define HEX_CONST 0xAF
#define OCT_CONST 012

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
	signed short min_value = 0, max_value = 0, hex_value = 0, oct_value = 0;
	int ret = 0, read_values = 0;

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%d %d 0x%x 0%o ", SHRT_MIN, SHRT_MAX,
				  HEX_CONST, OCT_CONST);
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
	read_values = fscanf(in_fp, "%hi %hi %hi %hi", &min_value, &max_value,
						 &hex_value, &oct_value);
	remove_file(in_fp);

	if (read_values == 4 && min_value == SHRT_MIN &&
		max_value == SHRT_MAX &&
		hex_value == HEX_CONST && oct_value == OCT_CONST) {
		printf(TNAME " Test Passed\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed\n");
		printf(TNAME "  Expected values: ret = 4, max_value = %d, "
			"min_value = %d, hex_value = %x, oct_value = %o\n\t\t\t "
			"Obtained values: ret = %d, max_value = %d, min_value = %d, "
			"hex_value = %x, oct_value = %o\n",
			SHRT_MAX, SHRT_MIN, HEX_CONST,
			OCT_CONST, ret, max_value, min_value,
			hex_value, oct_value);
		exit(PTS_FAIL);
	}
}
