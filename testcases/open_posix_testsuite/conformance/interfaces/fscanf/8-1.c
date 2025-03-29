/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	Input white space characters (as specified by isspace( ))are skipped
 *	unless [, c, C, or n conversion specifiers are used.
 *
 * method:
 *	-open file in write mode
 *	-write whitespace characters ('\n', '\t', '\f', '\r', '\v') into the
 *	file
 *	-close the file
 *	-open the same file in read mode
 *	-read data from the file using fscanf() and conversion specifier
 *	-compare the values read into the arguments with input whitespace
 *	characters
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TNAME "fscanf/8-1.c"
#define FNAME "in_file"
#define SPACE ' '
#define NEW_LINE '\n'
#define HORIZONTAL_TAB '\t'
#define FORM_FEED '\f'
#define CARRIAGE_RETURN '\r'
#define VERTICAL_TAB '\v'
#define STRING_CONST "LINUX"
#define INTEGER_CONST 10
#define INPUT_COUNT 6

static const char *const char_names[] = {
	"SPACE",
	"NEW LINE",
	"HORIZONTAL TAB",
	"FORM FEED",
	"CARRIAGE RETURN",
	"VERTICAL TAB"
};

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
	// Testcase for conversion specifier c:
	// Input file contains : SPACE NEW_LINE HORIZONTAL_TAB FORM_FEED
	//			 CARRIAGE_RETURN VERTICAL_TAB
	FILE *in_fp = NULL;
	int ret = 0, count = 0, sample_int = 0, byte_count = 0;
	char sample_char;
	wchar_t sample_wide_char = 0;
	char sample_string[sizeof(STRING_CONST)];

	errno = 0;

	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%c%c%c%c%c%c", SPACE, NEW_LINE, HORIZONTAL_TAB,
		      FORM_FEED, CARRIAGE_RETURN, VERTICAL_TAB);
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
	while (fscanf(in_fp, "%c", &sample_char) == 1 && count < INPUT_COUNT) {
		count++;
		if (sample_char == SPACE && count == 1)
			continue;
		else if (sample_char == NEW_LINE && count == 2)
			continue;
		else if (sample_char == HORIZONTAL_TAB && count == 3)
			continue;
		else if (sample_char == FORM_FEED && count == 4)
			continue;
		else if (sample_char == CARRIAGE_RETURN && count == 5)
			continue;
		else if (sample_char == VERTICAL_TAB && count == 6)
			continue;
		else {
			printf(TNAME " Conversion specifier c: Test Failed\n");
			printf(TNAME " Whitespace char at failure = %s\n",
				char_names[count-1]);
			remove_file(in_fp);
			exit(PTS_FAIL);
		}
	}
	if (count == INPUT_COUNT) {
		printf(TNAME
				" Test Passed: fscanf successfully read all whitespace "
			"characters with 'c' conversion specifier\n");
	} else {
		printf(TNAME " Error in fscanf, count = %d\n ", count);
		remove_file(in_fp);
		exit(PTS_FAIL);
	}

	// Testcase for conversion specifier 'n'
	errno = 0;
	rewind(in_fp);
	count = 0;

	while (fscanf(in_fp, "%c%n", &sample_char, &byte_count) == 1) {
		count++;
		if (sample_char == SPACE && count == 1 && byte_count == 1)
			continue;
		else if (sample_char == NEW_LINE && count == 2
					&& byte_count == 1)
			continue;
		else if (sample_char == HORIZONTAL_TAB && count == 3
					&& byte_count == 1)
			continue;
		else if (sample_char == FORM_FEED && count == 4
					&& byte_count == 1)
			continue;
		else if (sample_char == CARRIAGE_RETURN && count == 5
					&& byte_count == 1)
			continue;
		else if (sample_char == VERTICAL_TAB && count == 6
					&& byte_count == 1)
			continue;
		else {
			printf(TNAME " Conversion specifier n: Test Failed\n");
			printf(TNAME " Whitespace char at failure = %s\n",
				char_names[count-1]);
			remove_file(in_fp);
			exit(PTS_FAIL);
		}
	}
	if (count == INPUT_COUNT) {
		printf(TNAME
				" Test Passed: fscanf successfully read all whitespace "
			"characters with 'n' conversion specifier\n");
	} else {
		printf(TNAME " Error in fscanf, count = %d\n ", count);
		remove_file(in_fp);
		exit(PTS_FAIL);
	}

	// Testcase for conversion specifier 'C'
	errno = 0;
	rewind(in_fp);
	count = 0;

	while (fscanf(in_fp, "%C", &sample_wide_char) == 1) {
		count++;
		if (sample_wide_char == SPACE && count == 1)
			continue;
		else if (sample_wide_char == NEW_LINE && count == 2)
			continue;
		else if (sample_wide_char == HORIZONTAL_TAB && count == 3)
			continue;
		else if (sample_wide_char == FORM_FEED && count == 4)
			continue;
		else if (sample_wide_char == CARRIAGE_RETURN && count == 5)
			continue;
		else if (sample_wide_char == VERTICAL_TAB && count == 6)
			continue;
		else {
			printf(TNAME " Conversion specifier C: Test Failed\n");
			printf(TNAME " Whitespace char at failure = %s\n",
				char_names[count-1]);
			remove_file(in_fp);
			exit(PTS_FAIL);
		}
	}
	if (count == INPUT_COUNT) {
		printf(TNAME
				" Test Passed: fscanf successfully read all whitespace "
			"characters with 'C' conversion specifier\n");
	} else {
		printf(TNAME " Error in fscanf, count = %d\n ", count);
		remove_file(in_fp);
		exit(PTS_FAIL);
	}

	remove_file(in_fp);

	// Testcase for conversion specifier [:
	// Input file contains : SPACE|NEW_LINE|HORIZONTAL_TAB|FORM_FEED|
	//			 CARRIAGE_RETURN|VERTICAL_TAB
	errno = 0;
	count = 0;
	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error in opening the file, errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%c%c%c%c%c%c%c%c%c%c%c", SPACE, '|', NEW_LINE,
		      '|', HORIZONTAL_TAB, '|', FORM_FEED, '|',
		      CARRIAGE_RETURN, '|', VERTICAL_TAB);
	if (ret < 0) {
		printf(TNAME " Error in fprintf()\n");
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
	while ((ret = fscanf(in_fp, "%[^|]%*c", &sample_char)) == 1) {
		count++;
		if (sample_char == SPACE && count == 1)
			continue;
		else if (sample_char == NEW_LINE && count == 2)
			continue;
		else if (sample_char == HORIZONTAL_TAB && count == 3)
			continue;
		else if (sample_char == FORM_FEED && count == 4)
			continue;
		else if (sample_char == CARRIAGE_RETURN && count == 5)
			continue;
		else if (sample_char == VERTICAL_TAB && count == 6)
			continue;
		else {
			printf(TNAME " Conversion specifier [: Test Failed\n");
			printf(TNAME " Whitespace char at failure = %s\n",
				char_names[count-1]);
			remove_file(in_fp);
			exit(PTS_FAIL);
		}
	}
	if (count == INPUT_COUNT) {
		printf(TNAME
				" Test Passed: fscanf successfully read all whitespace "
			"characters with '[' conversion specifier\n");
	} else {
		printf(TNAME " Error in fscanf, count =%d\n ", count);
		remove_file(in_fp);
		exit(PTS_FAIL);
	}

	remove_file(in_fp);

	// Testcase for conversion specifier other than c,C,n,[:
	errno = 0;
	in_fp = fopen(FNAME, "w");
	if (in_fp == NULL) {
		printf(TNAME " Error at fopen(), errno = %d\n", errno);
		remove_file(in_fp);
		exit(PTS_UNRESOLVED);
	}

	ret = fprintf(in_fp, "%d%c%c%c%c%c%c%s", INTEGER_CONST, SPACE, NEW_LINE,
			HORIZONTAL_TAB, FORM_FEED, CARRIAGE_RETURN,
			VERTICAL_TAB, STRING_CONST);
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
	ret = fscanf(in_fp, "%d%s", &sample_int, sample_string);
	if (ret == 2 && sample_int == INTEGER_CONST &&
	   (strcmp(sample_string, STRING_CONST) == 0)) {
		printf(TNAME " Test Passed: As expected fscanf skipped "
			"whitespace character when conversion specifer "
			"is not c, C, n or [\n");
		remove_file(in_fp);
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed when conversion specifier is "
			"not c, C, n or [:\n\t\t\tExpected values: "
			"sample_int = %d  sample_string = %s\n\t\t\t"
			"Obtained values: sample_int = %d  sample_string = %s\n",
			INTEGER_CONST, STRING_CONST, sample_int,
			sample_string);
		remove_file(in_fp);
		exit(PTS_FAIL);
	}
}

