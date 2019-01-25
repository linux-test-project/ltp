/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	If the correct value is outside the range of representable values:
 *	{LONG_MIN}, {LONG_MAX}, will be returned (according to the sign
 *	of the value), and errno set to [ERANGE].
 *
 *	method:
 *	-Define a string which has an integer greater
 *	than LONG_MAX range.
 *	-Convert string to long using strtol() function.
 *	-Compare the return value with LONG_MAX and check errno.
 *	-Define a string which has an integer lesser
 *	than LONG_MIN range.
 *	-Convert string to long using strtol() function.
 *	-Compare the return value with LONG_MIN and check errno.
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "strtol/12-1.c"

int main(void)
{
	char nptr_1[] = "922337203685477580798";
	char nptr_2[] = "-922337203685477580798";
	long result;
	errno = 0;

	result = strtol(nptr_1, NULL, 10);
	if (result == LONG_MAX && errno == ERANGE)   {
		printf(TNAME " Test passed, LONG_MAX is returned and errno is set to indicate "
				"out of range error.\n");
	} else {
		printf(TNAME " Test Failed, Expected result: %ld, But got result: %ld\n"
				" \t\t\t   and Expected errno: %d, But obtained errno: %d\n"
				, LONG_MAX, result, ERANGE, errno);
		exit(PTS_FAIL);
	}

	errno = 0;
	result = strtol(nptr_2, NULL, 10);
	if (result == LONG_MIN && errno == ERANGE)   {
		printf(TNAME " Test passed, LONG_MIN is returned and errno is set to indicate "
				"out of range error.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed, Expected result: %ld, But got result: %ld\n"
				"\t\t\t   and Expected errno: %d, But obtained errno: %d\n",
				LONG_MIN, result, ERANGE, errno);
		exit(PTS_FAIL);
	}
}
