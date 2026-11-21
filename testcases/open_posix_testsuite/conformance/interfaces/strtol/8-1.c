/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	This function will not change the setting of errno if successful.
 *
 * method:
 *	-Convert LONG_MAX to string using sprintf
 *	as strtol() needs input in string format.
 *	-Convert string to long using strtol() function.
 *	-Compare the return value with LONG_MAX.
 *	-If comparison is successfull, then check errno value.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "strtol/8-1.c"
#define MAX_ARRAY_SIZE 50

int main(void)
{
	char nptr[MAX_ARRAY_SIZE] = { };
	long result;
	int char_written;
	errno = 0;

	/* convert integer to string */
	char_written = sprintf(nptr, "%ld", LONG_MAX);
	if (char_written != strlen(nptr)) {
		printf(TNAME "Error at sprintf(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	result = strtol(nptr, NULL, 10);

	if (result == LONG_MAX) {
		if (errno == 0) {
			printf(TNAME " Test passed, errno is not changed.\n");
			exit(PTS_PASS);
		} else {
			printf(TNAME " Test Failed, errno is changed. Expected errno: %d,"
					" Obtained: %d.\n", 0, errno);
			exit(PTS_FAIL);
		}
	} else {
		printf(TNAME " Unexpected return from strtol. Expected: %ld,"
				" But got: %ld\n", LONG_MAX, result);
		exit(PTS_FAIL);
	}
}
