/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 *
 * Created by:  julie.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that sigaddset() will add all defined signal numbers to a signal
 * set.
 *
 *  Test steps:
 * 1)  Initialize an empty signal set.
 * For each signal number:
 *   2)  Add the signal to the empty signal set.
 *   3)  Verify that the signal is a member of the signal set.
 */
#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

static const int sigs[] = {
	SIGABRT, SIGALRM, SIGBUS,  SIGCHLD, SIGCONT,
	SIGFPE,  SIGHUP,  SIGILL,  SIGINT,  SIGKILL,
	SIGPIPE, SIGQUIT, SIGSEGV, SIGSTOP, SIGTERM,
	SIGTSTP, SIGTTIN, SIGTTOU, SIGUSR1, SIGUSR2,
	SIGURG,
};

int main(void)
{
	sigset_t signalset;
	unsigned int i;
	int err = 0;

	if (sigemptyset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborting\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < sizeof(sigs)/sizeof(int); i++) {
		if (sigaddset(&signalset, sigs[i]) == 0) {
			if (sigismember(&signalset, sigs[i]) != 1) {
				err++;
				printf("Signal %d wasn't added \n", sigs[i]);
			}
		} else {
			err++;
			printf("Failed to add sinal %d\n", sigs[i]);
		}
	}

	if (err) {
		printf("FAILED: Some signals not added\n");
		return PTS_FAIL;
	} else {
		printf("Test PASSED: All signals added\n");
		return PTS_PASS;
	}
}
