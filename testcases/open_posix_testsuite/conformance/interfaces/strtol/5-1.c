/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	If the subject sequence begins with a 'hyphen-minus':
 *		The value resulting from the conversion will be negated.
 *
 * method:
 *	-Convert LONG_MIN to string using sprintf
 *	as strtol() needs input in string format.
 *	-Convert string to long using strtol() function.
 *	-Compare the return value with negative LONG_MIN.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "strtol/5-1.c"
#define MAX_ARRAY_SIZE 50

int main(void)
{
	char nptr[MAX_ARRAY_SIZE] = {};
	long result;
	int char_written;
	errno;

	/* convert integer to string */
	/* Considering LONG_MIN as it starts minus(-) */
	char_written = sprintf(nptr, "%ld", LONG_MIN);
	if (char_written != strlen(nptr)) {
		printf(TNAME "Error at sprintf(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	result = strtol(nptr, NULL, 10);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), expected: %ld, But got: %ld,"
				" errno = %d\n", LONG_MAX, result, errno);
		exit(PTS_FAIL);
	}

	if (result == LONG_MIN) {
		printf(TNAME " Test passed, String got converted into long.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed, String conversion to long got failed, Expected: %ld"
				" ,But got: %ld\n", LONG_MIN, result);
		exit(PTS_FAIL);
	}
}
