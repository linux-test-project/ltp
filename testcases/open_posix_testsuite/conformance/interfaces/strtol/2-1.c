/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	If the input string is empty or consists entirely of white-space characters,
 *	or if the first non-white-space character is other than a sign or a permissible
 *	letter or digit:
 *		The subject sequence will contain no characters.
 *	A subject sequence is interpreted as an integer represented in some radix
 *	determined by the value of base.
 *
 *  method:
 *	-Take a input string with first non-white-space character
 *	other than permissible letter.
 *	-Convert string to long using strtol() function
 *	with base 10.
 *	-Compare the value stored in nptr and endptr.
 *	-Repeat above steps for empty string and string
 *	having white-space characters with base 8 and 16
 *	respectively.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define TNAME "strtol/2-1.c"

int main(void)
{
	char nptr_1[] = "Z123";
	char nptr_2[] = {};
	char nptr_3[] = "     ";
	char *endptr = NULL;

	/* Check with first non-white-space character other than permissible letter */
	strtol(nptr_1, &endptr, 10);
	if (nptr_1 == endptr) {
		printf(TNAME " Test passed.\n");
	} else {
		printf(TNAME " Test Failed. Expected: %s, But got: %s.\n", nptr_1, endptr);
		exit(PTS_FAIL);
	}

	/* Check with empty string */
	strtol(nptr_2, &endptr, 8);
	if (nptr_2 == endptr) {
		printf(TNAME " Test passed.\n");
	} else {
		printf(TNAME " Test Failed. Expected: %s, But got: %s.\n", nptr_2, endptr);
		exit(PTS_FAIL);
	}

	/* Check with white-space characters */
	strtol(nptr_3, &endptr, 16);
	if (nptr_3 == endptr) {
		printf(TNAME " Test passed.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed. Expected: %s, But got: %s.\n", nptr_3, endptr);
		exit(PTS_FAIL);
	}
}
