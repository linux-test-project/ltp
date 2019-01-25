/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	If no conversion could be performed:
 *		0 will be returned.
 *
 * method:
 *	-Take a string with first invalid byte, for octal integer.
 *	-Convert it to long using strtol()
 *	with base 8.
 *	-check whether the result is 0.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "strtol/10-1.c"

int main(void)
{
	char nptr[] = "H1234";
	long result;

	result = strtol(nptr, NULL, 8);
	if (result == 0) {
		printf(TNAME " Test passed. Returned zero as expected.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed. Expected: %d ,But got: %ld\n", 0, result);
		exit(PTS_FAIL);
	}
}
