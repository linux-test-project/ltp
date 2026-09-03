/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	When "%hhn" specifier is specified, data is read from the named
 *	input stream in sequence, interprets the bytes in sequence according
 *	to the directive format specified before "%hhn" directive, and stores
 *	the result in its arguments. Number of bytes read by fscanf() so far
 *	are written to the argument corresponding to "%hhn" directive.
 *	Directives specified after "%hhn" are converted but they don't account
 *	for the length calculated in "%hhn" directive.
 *
 * method:
 *	-open file in write mode
 *	-write a 3 sample strings into the file each seperated by whitespace
 *	character
 *	-close the file
 *	-open the same file in read mode
 *	-read data from the file using fscanf and conversion specifier "%hhn"
 *	-compare the number of bytes read into argument corresponding to first
 *	 "%hhn" with length of first string and that corresponding to second
 *	"%hhn"  with length of first string + length of second string +1. Reason
 *	for adding  1 is that "%hhn" will consume whitespace character.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TNAME "fscanf/21-1.c"
#define FNAME "in_file"
#define STRING_CONST1 "POSIX"
#define STRING_CONST2 "CONFORMANCE"
#define STRING_CONST3 "TEST"

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
	unsigned char byte_count1 = 0, byte_count2 = 0;
	unsigned char total_byte_count = 0;
	int ret = 0;

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%s %s %s", STRING_CONST1, STRING_CONST2,
		      STRING_CONST3);
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
	ret = fscanf(in_fp, "%*s%hhn%*s%hhn%*s", &byte_count1, &byte_count2);
	remove_file(in_fp);

	total_byte_count = strlen(STRING_CONST1) + strlen(STRING_CONST2) + 1;
	if (ret == 0 && byte_count1 == (strlen(STRING_CONST1)) &&
	    byte_count2 == total_byte_count) {
		printf(TNAME " Test Passed\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed\n");
		printf(TNAME " Number of bytes read by %%hn  after reading "
			"string_const1 = %hhd and string_const2 = %hhd when the "
			"expected value of byte count after reading "
			"string_const1 = %hhd and string_const2 = %hhd\n",
			byte_count1, total_byte_count,
			strlen(STRING_CONST1), total_byte_count);
		exit(PTS_FAIL);
	}
}
