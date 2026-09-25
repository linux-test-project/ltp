/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	If the subject sequence has the expected form and the value of base is
 *	between 2 and 36:
 *		It will be used as the base for conversion.
 *
 * method:
 *	Demical, octal, hexadecimal:
 *	-Convert LONG_MAX to string using sprintf
 *	as strtol() needs input in string format.
 *	-Convert string to long using strtol() function with base given.
 *	-Compare the return value with LONG_MAX.
 *	-Repeat the same steps for other base values.
 * For base 9,13 and 36:
 *	-Define a string constant with value permissible for base 9.
 *	-Define expected value in decimal for the value in defined string.
 *	-Convert string to long using strtol() function with base 9.
 *	-Compare return value and expected value.
 *	-Repeat above steps for base 13 and 36.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "strtol/4-1.c"
#define MAX_ARRAY_SIZE 20

int main(void)
{
	char *nptr = malloc(MAX_ARRAY_SIZE);
	long result;
	int char_written;
	char *nptr_1 = "234";
	long expected_res_base_9 = 193;
	char *nptr_2 = "A567";
	long expected_res_base_13 = 22900;
	char *nptr_3 = "Z678";
	long expected_res_base_36 = 1640996;

	/* For decimal constant */
	errno = 0;
	char_written = sprintf(nptr, "%ld", LONG_MAX);
	if (char_written != strlen(nptr)) {
		printf(TNAME " Error at sprintf(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	result = strtol(nptr, NULL, 10);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), errno = %d\n", errno);
		exit(PTS_FAIL);
	}

	if (result == LONG_MAX) {
		printf(TNAME " Test passed, String got converted into long decimal integer.\n");
	} else {
		printf(TNAME " Test Failed, String conversion into long decimal integer got "
				"failed, Expected: %ld, But got: %ld.\n", LONG_MAX, result);
		exit(PTS_FAIL);
	}

	/* For hexadecimal constant */
	errno = 0;
	char_written = sprintf(nptr, "%lx", LONG_MAX);
	if (char_written != strlen(nptr)) {
		printf(TNAME " Error at sprintf(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	result = strtol(nptr, NULL, 16);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), errno = %d\n", errno);
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
	errno = 0;
	result = strtol(nptr, NULL, 8);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), errno = %d\n", errno);
		exit(PTS_FAIL);
	}
	if (result == LONG_MAX) {
		printf(TNAME " Test passed, String got converted into long octal integer.\n");
	} else {
		printf(TNAME " Test Failed, String conversion into octal integer got "
				"failed, Expected: %lo, But got: %lo.\n", LONG_MAX, result);
		exit(PTS_FAIL);
	}

	/* For base 9 */
	errno = 0;
	result = strtol(nptr_1, NULL, 9);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), errno = %d\n", errno);
		exit(PTS_FAIL);
	}
	if (result == expected_res_base_9) {
		printf(TNAME " Test passed, String got converted into long integer of base 9.\n");
	} else {
		printf(TNAME " Test Failed, String conversion into integer of base 9 got "
				"failed, Expected: %ld, But got: %ld.\n", expected_res_base_9, result);
		exit(PTS_FAIL);
	}

	/* For base 13 */
	errno = 0;
	result = strtol(nptr_2, NULL, 13);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), errno = %d\n", errno);
		exit(PTS_FAIL);
	}
	if (result == expected_res_base_13) {
		printf(TNAME " Test passed, String got converted into long integer of base 13.\n");
	} else {
		printf(TNAME " Test Failed, String conversion into integer of base 13 got "
				"failed, Expected: %ld, But got: %ld.\n", expected_res_base_13, result);
		exit(PTS_FAIL);
	}

	/* For base36 */
	errno = 0;
	result = strtol(nptr_3, NULL, 36);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), errno = %d\n", errno);
		exit(PTS_FAIL);
	}
	if (result == expected_res_base_36) {
		printf(TNAME " Test passed, String got converted into long integer of base 36.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed, String conversion into integer of base 36 got "
				"failed, Expected: %ld, But got: %ld.\n", expected_res_base_36, result);
		exit(PTS_FAIL);
	}
}
