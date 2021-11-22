/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis
* Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* This sample test aims to check the following assertions:
*
* If SA_SIGINFO is set in sa_flags and Real Time Signals extension is supported,
* sa_sigaction is used as the signal handling function.
*
* The steps are:
* -> test for RTS extension
* -> register a handler for SIGHUP with SA_SIGINFO, and a known function
*   as sa_sigaction
* -> raise SIGHUP, and check the function has been called.
*
* The test fails if the function is not called
*/


#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "posixtest.h"

static volatile sig_atomic_t called = 0;

static void handler(int sig, siginfo_t *info, void *context)
{
	(void) sig;
	(void) context;

	if (info->si_signo != SIGHUP) {
		PTS_WRITE_MSG("Wrong signal generated?\n");
		_exit(PTS_FAIL);
	}

	called = 1;
}

int main(void)
{
	int ret;
	long rts;

	struct sigaction sa;

	/* Test the RTS extension */
	rts = sysconf(_SC_REALTIME_SIGNALS);

	if (rts < 0L) {
		fprintf(stderr, "This test needs the RTS extension");
		return PTS_UNTESTED;
	}

	/* Set the signal handler */
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;

	ret = sigemptyset(&sa.sa_mask);

	if (ret != 0) {
		perror("Failed to empty signal set");
		return PTS_UNRESOLVED;
	}

	/* Install the signal handler for SIGHUP */
	ret = sigaction(SIGHUP, &sa, 0);

	if (ret != 0) {
		perror("Failed to set signal handler");
		return PTS_UNTESTED;
	}

	if (called) {
		fprintf(stderr,
			"The signal handler has been called before signal was raised");
		return PTS_FAIL;
	}

	ret = raise(SIGHUP);

	if (ret != 0) {
		perror("Failed to raise SIGHUP");
		return PTS_UNRESOLVED;
	}

	if (!called) {
		fprintf(stderr, "The sa_handler was not called");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");

	return PTS_PASS;
}
