/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	If the value of base is not supported:
 *		0 will be returned and errno will be set to [EINVAL]
 *
 *  method:
 *	-Convert LONG_MAX to string using sprintf
 *	as strtol() needs input in string format.
 *	-Convert string to long using strtol() function
 *	with base which is not supported by strtol(<2 and >36)
 *	-Compare the return value and errno.
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "strtol/11-1.c"
#define MAX_ARRAY_SIZE 50

int main(void)
{
	char nptr[MAX_ARRAY_SIZE] = {};
	long result;
	int char_written;
	errno = 0;

	/* convert integer to string */
	char_written = sprintf(nptr, "%ld", LONG_MAX);
	if (char_written != strlen(nptr)) {
		printf(TNAME " Error at sprintf(), errno = %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	result = strtol(nptr, NULL, 37);
	if (result == 0 && errno == EINVAL) {
		printf(TNAME " Test passed, conversion error occured for unsupported base.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed, Expected result: %d, But obtained: %ld"
				" and Expected errno: %d, But obtained errno: %d\n"
				, 0, result, EINVAL, errno);
		exit(PTS_FAIL);
	}
}
