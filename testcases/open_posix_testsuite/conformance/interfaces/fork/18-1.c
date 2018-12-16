/*
* Copyright (c) 2004, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis

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

* This sample test aims to check the following assertion:
*
* The per-process timers are not inherited.

* The steps are:
* -> Create a per-process timer
* -> fork
* -> check if the timer exists in child.

* The test fails if the timer expires in child (timer signal is delivered).

*/


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <errno.h>

#include <signal.h>
#include <time.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

static int notified;

static void notification(union sigval sv)
{
	if (sv.sival_int != SIGUSR1) {
		output("Got signal %d, expected %d\n", sv.sival_int, SIGUSR1);
		UNRESOLVED(1, "Unexpected notification");
	}

	notified = (int)getpid();
}

int main(void)
{
	int ret, status;
	pid_t child, ctl;

	timer_t tmr;

	struct sigevent se;

	struct itimerspec it;

	output_init();

	notified = 0;

	/* Create the timer */
	se.sigev_notify = SIGEV_THREAD;
	se.sigev_signo = 0;
	se.sigev_value.sival_int = SIGUSR1;
	se.sigev_notify_function = &notification;
	se.sigev_notify_attributes = NULL;	/* default detached thread */

	ret = timer_create(CLOCK_REALTIME, &se, &tmr);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to create a timer");
	}

	/* Arm the timer */
	it.it_interval.tv_nsec = 0;

	it.it_interval.tv_sec = 0;

	it.it_value.tv_sec = 0;

	it.it_value.tv_nsec = 500000000;	/* 0.5 sec */

	ret = timer_settime(tmr, 0, &it, NULL);

	/* Create the child */
	child = fork();

	if (child == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0) {

		sleep(1);

		if (notified != 0) {
			if (notified == (int)getpid()) {
				FAILED
				    ("Per-Process Timer was inherited in child");
			} else {
				output
				    ("Notification occured before the child forked");
			}
		}

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if ((!WIFEXITED(status)) || (WEXITSTATUS(status) != PTS_PASS)) {
		FAILED("Child exited abnormally");
	}

	if (notified != (int)getpid()) {
		output("Notified value: %d\n", notified);
		UNRESOLVED(-1,
			   "No notification occured -- per process timers do not work?");
	}

#if VERBOSE > 0
	output("Test passed\n");
#endif
	PASSED;
}
