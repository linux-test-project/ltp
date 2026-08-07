/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	A pointer to the final string will be stored in the object pointed
 *	to by endptr, provided that endptr is not a null pointer.
 *	A final string is one or more unrecognized characters, including the
 *	terminating NUL character of the input string.
 * method:
 *	-Define a string which has half of the string valid characters
 *	and other half string as invalid characters, for base 10.
 *	-Define pos as the position of first invalid character in
 *	the string, for base 10.
 *	-Use strtol() to convert string to long int with base10.
 *	-Compare the endptr and nptr+pos.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "strtol/6-1.c"

int main(void)
{
	char *nptr = "1234F112";
	char *endptr = NULL;
	int pos = 4;
	long result;
	errno = 0;

	result = strtol(nptr, &endptr, 10);
	if (result == 0) {
		printf(TNAME " Unexpected return from strtol(), expected: %ld, But got: %ld,"
				" errno = %d\n", LONG_MAX, result, errno);
		exit(PTS_FAIL);
	}

	if (endptr == nptr + pos) {
		printf(TNAME " Test passed.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed,Expected nptr: %p"
				" ,But got nptr: %p\n", nptr + pos, endptr);
		exit(PTS_FAIL);
	}
}
