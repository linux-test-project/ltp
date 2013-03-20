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
* The CPU-time clock of the new process/ new process's thread is initialized to 0.

* The steps are:
* -> compute until the parent process CPU-time clock is greater than 1 sec.
* -> fork
* -> check the child process process CPU time and thread CPU time clocks.

* The test fails if any of these clocks are > 1sec.

*/

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <errno.h>

#include <time.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

int main(void)
{
	int ret, status;
	pid_t child, ctl;

	long ctp, ctt;
	clockid_t clp, clt;

	struct timespec tp;

	output_init();

	ctp = sysconf(_SC_CPUTIME);
	ctt = sysconf(_SC_THREAD_CPUTIME);

	if ((ctp == -1) && (ctt == -1)) {
		UNTESTED
		    ("The testcase needs CPUTIME or THREAD_CPUTIME support");
	}
#if VERBOSE > 0
	output("System abilities:\n");

	output("  _POSIX_CPUTIME        : %ld\n", ctp);

	output("  _POSIX_THREAD_CPUTIME : %ld\n", ctt);

#endif
	if (ctp > 0) {
		ret = clock_getcpuclockid(0, &clp);

		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to get cpu-time clock id of the process");
		}

		do {
			ret = clock_gettime(clp, &tp);

			if (ret != 0) {
				UNRESOLVED(errno,
					   "Failed to read CPU time clock");
			}
		}
		while (tp.tv_sec < 1);
	}

	if (ctt > 0) {
		ret = pthread_getcpuclockid(pthread_self(), &clt);

		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to get cpu-time clock id of the thread");
		}

		do {
			ret = clock_gettime(clt, &tp);

			if (ret != 0) {
				UNRESOLVED(errno,
					   "Failed to read thread CPU time clock");
			}
		}
		while (tp.tv_sec < 1);
	}

	/* Create the child */
	child = fork();

	if (child == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0) {
		if (ctp > 0) {
			ret = clock_getcpuclockid(0, &clp);

			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to get cpu-time clock id of the process");
			}

			ret = clock_gettime(clp, &tp);

			if (ret != 0) {
				UNRESOLVED(errno,
					   "Failed to read CPU time clock");
			}

			if (tp.tv_sec > 0) {
				FAILED
				    ("The process CPU-time clock was not reset in child\n");
			}
		}

		if (ctt > 0) {
			ret = pthread_getcpuclockid(pthread_self(), &clt);

			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to get cpu-time clock id of the thread");
			}

			ret = clock_gettime(clt, &tp);

			if (ret != 0) {
				UNRESOLVED(errno,
					   "Failed to read thread CPU time clock");
			}

			if (tp.tv_sec > 0) {
				FAILED
				    ("The thread CPU-time clock was not reset in child\n");
			}
		}

		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != PTS_PASS)) {
		FAILED("Child exited abnormally");
	}

#if VERBOSE > 0
	output("Test passed\n");
#endif
	PASSED;
}
