/*
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license. For the full content of this
 * license, see the COPYING file at the top level of this source tree.
 *
 * After sigprocmask() is called on an invalid how it should return -1 and set
 * errno to EINVAL.
 */


#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include "posixtest.h"

int main(void)
{
	sigset_t set;
	int r, i, fails = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGABRT);

	for (i = 0; i < 100000; i++) {
		r = rand() % (i + 1);

		switch (r) {
		case SIG_BLOCK:
		case SIG_UNBLOCK:
		case SIG_SETMASK:
			continue;
		default:
		break;
		}

		if (sigprocmask(r, &set, NULL) != -1 || errno != EINVAL) {
			printf("sigprocmask(%i, ...) failed to fail\n", r);
			fails++;
		}
	}

	if (fails) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
