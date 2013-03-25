/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Test that sched_setparam() sets errno == EINVAL when the sched_ss_max_repl
 * is not within the inclusive range [1,SS_REPL_MAX]
 *
 * @pt:SS
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#if defined(_POSIX_SPORADIC_SERVER)&&(_POSIX_SPORADIC_SERVER != -1)

int main(void)
{
	int policy, result;
	int result_code = PTS_PASS;
	struct sched_param param;

	if (sched_getparam(0, &param) == -1) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	/* test when sched_ss_max_repl < 1 */
	param.sched_ss_max_repl = 0;
	result = sched_setparam(0, &param);

	if (result != -1) {
		printf
		    ("The returned code is not -1 when sched_ss_max_repl < 1.\n");
		result_code = PTS_FAIL;
	} else if (errno == EPERM) {
		printf
		    ("This process does not have the permission to set its own scheduling parameter.\nTry to launch this test as root\n");
		result_code = PTS_UNRESOLVED;
	} else if (errno != EINVAL) {
		perror("Unknow error");
		result_code = PTS_FAIL;
	}

	/* test when sched_ss_max_repl > SS_REPL_MAX */
	param.sched_ss_max_repl = SS_REPL_MAX + 1;
	result = sched_setparam(0, &param);

	if (result == -1 && errno == EINVAL) {
		if (result_code == PTS_PASS) {
			printf("Test PASSED\n");
		}
		return result_code;
	} else if (result != -1) {
		printf
		    ("The returned code is not -1 when sched_ss_max_repl > SS_REPL_MAX.\n");
		return PTS_FAIL;
	} else if (errno == EPERM) {
		if (result_code == PTS_FAIL) {
			printf
			    ("This process does not have the permission to set its own scheduling parameter.\nTry to launch this test as root\n");
			return PTS_FAIL;
		}
		return PTS_UNRESOLVED;
	} else {
		perror("Unknow error");
		return PTS_FAIL;
	}

}

#elif _POSIX_SPORADIC_SERVER == -1
int main(void)
{
	printf("_POSIX_SPORADIC_SERVER support not available\n");
	return PTS_UNSUPPORTED;
}
#else
#error "_POSIX_SPORADIC_SERVER not defined properly"
#endif
