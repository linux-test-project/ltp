/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	If the subject sequence has the expected form and the value of base is 0:
 *		The sequence of characters starting with the first digit will be
 *		interpreted as an integer constant.
 *
 * method:
 *	Demical:
 *	-Convert LONG_MAX to string using sprintf
 *	as strtol() needs input in string format.
 *	-Convert string to long using strtol() function with base 0.
 *	-Compare the return value with LONG_MAX.
 *	Octal and Hexadecimal:
 *	-Repeat the same steps for octal and hexadecimal with one
 *	extra step to add "0" and "0x" to input string(nptr)
 *	for octal and hexadecimal respectively.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "strtol/3-1.c"
#define MAX_ARRAY_SIZE 20

int main(void)
{
	char *nptr = malloc(MAX_ARRAY_SIZE);
	long result;
	int char_written;
	char base_hex[] = "0x";
	char base_oct[] = "0";

	/* For decimal constant */
	errno = 0;
	char_written = sprintf(nptr, "%ld", LONG_MAX);
	if (char_written != strlen(nptr)) {
		printf(TNAME " Error at sprintf(), errno = %d\n", errno);
		free(nptr);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	result = strtol(nptr, NULL, 0);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), expected: %ld, But got: %ld,"
				" errno = %d\n", LONG_MAX, result, errno);
		free(nptr);
		exit(PTS_FAIL);
	}
	if (result == LONG_MAX) {
		printf(TNAME " Test passed, String got converted into long decimal integer.\n");
		free(nptr);
	} else {
		printf(TNAME " Test Failed, String conversion into long decimal integer got "
				"failed, Expected: %ld, But got: %ld.\n", LONG_MAX, result);
		free(nptr);
		exit(PTS_FAIL);
	}

	/* For hexadecimal constant */
	errno = 0;
	char_written = sprintf(nptr, "%lx", LONG_MAX);
	if (char_written != strlen(nptr)) {
		printf(TNAME " Error at sprintf(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}
	nptr = strcat(base_hex, nptr);
	errno = 0;
	result = strtol(nptr, NULL, 0);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), expected: %ld, But got: %ld,"
					" errno = %d\n", LONG_MAX, result, errno);
		exit(PTS_FAIL);
	}
	if (result == LONG_MAX) {
		printf(TNAME " Test passed, String got converted into long hexadecimal integer.\n");
	} else {
		printf(TNAME " Test Failed, String conversion into long hexadecimal integer got "
					"failed, Expected: %lx, But got: %lx.\n", LONG_MAX, result);
		exit(PTS_FAIL);
	}

	/* For octal constant */
	errno = 0;
	char_written = sprintf(nptr, "%lo", LONG_MAX);
	if (char_written != strlen(nptr)) {
		printf(TNAME " Error at sprintf(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}
	nptr = strcat(base_oct, nptr);
	errno = 0;
	result = strtol(nptr, NULL, 0);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), expected: %ld, But got: %ld,"
					" errno = %d\n", LONG_MAX, result, errno);
		exit(PTS_FAIL);
	}
	if (result == LONG_MAX) {
		printf(TNAME " Test passed, String got converted into long octal integer.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed, String conversion into octal integer got failed,"
					" Expected: %lo, But got: %lo.\n", LONG_MAX, result);
		exit(PTS_FAIL);
	}
}
