/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	The function will not change the orientation of the standard error stream.
 *
 * method:
 *	-Set errno to ERANGE.
 *	-Set stderr stream orientation to 1 using fwide().
 *	-Call perror() for "strtol".
 *	-Check fwide orientation after perror().
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"
#include <features.h>
#include <wchar.h>

#define TNAME "perror/4-1.c"

int main(void)
{
	int ret;

	errno = ERANGE;
	ret = fwide(stderr, 1);
	perror("strtol");
	ret = fwide(stderr, 0);

	if (ret == 1) {
		printf(TNAME " Test Passed, orientation of the standard error stream"
					" is unchanged.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed, orientation of the standard error stream"
					" is changed.\n");
		exit(PTS_FAIL);
	}
}
