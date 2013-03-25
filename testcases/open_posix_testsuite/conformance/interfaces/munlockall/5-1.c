/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the munlockall() function always return a value of zero if it is
 * supported by the implementation.
 */

#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#if !defined(_POSIX_MEMLOCK) || _POSIX_MEMLOCK == -1

int main(void)
{
	printf("Does not support ML (Memory Lock).\n");
	return PTS_UNSUPPORTED;
}

#else

#if _POSIX_MEMLOCK != 0
int main(void)
{
	int result;

	result = munlockall();

	if (result == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (errno == EPERM) {
		printf
		    ("You don't have permission to unlock your address space.\nTry to rerun this test as root.\n");
		return PTS_UNRESOLVED;
	} else {
		printf("munlockall() returns %i instead of zero.\n", result);
		return PTS_FAIL;
	}

}

#else

int main(void)
{
	int result;
	long memlock;

	memlock = sysconf(_SC_MEMLOCK);
	if (errno) {
		perror("An errno occurs when calling sysconf().\n");
		return PTS_UNRESOLVED;
	}

	result = munlockall();

	if ((result == 0 && memlock > 0) || (result == -1 && memlock <= 0)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (errno == EPERM) {
		printf
		    ("You don't have permission to unlock your address space.\nTry to rerun this test as root.\n");
		return PTS_UNRESOLVED;
	} else {
		printf("munlockall() returns %i instead of zero.\n", result);
		return PTS_FAIL;
	}

}

#endif

#endif
