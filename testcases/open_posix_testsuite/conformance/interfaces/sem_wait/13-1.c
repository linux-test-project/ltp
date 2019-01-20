/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis
* Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>

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
*  If the semaphore count is 0, the call blocks until the semphore can be
* locked or the operation is interrupted by a signal.

* The steps are:
* -> Initialize a semaphore to 0 count
* -> Register a signal handler for SIGALRM which post the semaphore
* -> save current time
* -> set an alarm to 1 second
* -> sem_wait
* -> Verify either errno is EINTR or no error occured.
* -> Verify 1 sec has elapsed.

* The test fails if the call did not block.

*/

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <semaphore.h>
#include <signal.h>
#include <errno.h>

#include <timespec.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

static sem_t sem;

static void handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	int ret;
	ret = sem_post(&sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to post semaphore");
	}
}

int main(void)
{
	int ret;
	struct timespec ts_ref, ts_fin;
	struct sigaction sa;

	output_init();

	ret = sem_init(&sem, 0, 0);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to init semaphore");
	}

	sa.sa_flags = 0;
	sa.sa_handler = handler;

	ret = sigemptyset(&sa.sa_mask);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to empty signal set");
	}

	sigaction(SIGALRM, &sa, 0);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to set signal handler");
	}

	ret = clock_gettime(CLOCK_REALTIME, &ts_ref);

	if (ret != 0) {
		UNRESOLVED(errno, "Unable to read clock");
	}

	/*
	 * POSIX allows for alarm to quite unprecise, so we ask for 2 seconds
	 * and check for at least one.
	 */
	alarm(2);

	ret = sem_wait(&sem);

	if (ret != 0 && errno != EINTR) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	ret = clock_gettime(CLOCK_REALTIME, &ts_fin);

	if (ret != 0) {
		UNRESOLVED(errno, "Unable to read clock");
	}

	if (timespec_nsec_diff(&ts_fin, &ts_ref) < NSEC_IN_SEC) {
		output("Ts: %d.%9.9d  ->  %d.%9.9d\n",
		       ts_ref.tv_sec, ts_ref.tv_nsec,
		       ts_fin.tv_sec, ts_fin.tv_nsec);
		FAILED("The sem_wait call did not block");
	}

	ret = sem_destroy(&sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to sem_destroy");
	}

	output("Test passed\n");

	PASSED;
}
