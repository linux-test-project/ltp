/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * adam.li@intel.com
 *
 * if _POSIX_THREAD_CPUTIME is defined, the new thread shall have a CPU-time
 * clock accessible, and the initial value of this clock shall be set to 0.
 */
 /* Do some work in the main thread so that it's clock value is non zero.
  * Create a new thread and get the time of the thread CUP-time clock
  * using clock_gettime().
  * Note, the tv_nsec cannot be exactly 0 at the time of calling
  * clock_gettime() since the thread has executed some time.
  */

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include "posixtest.h"

static void *a_thread_func()
{
	struct timespec ts = {.tv_sec = 1,.tv_nsec = 1 };

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);

	/* We expect the clock to be less than 0.1s */
	if (ts.tv_sec != 0 || ts.tv_nsec >= 100000000) {
		printf("FAIL: ts.tv_sec: %ld, ts.tv_nsec: %ld\n",
		       ts.tv_sec, ts.tv_nsec);
		exit(PTS_FAIL);
	}

	return NULL;
}

static volatile sig_atomic_t flag = 1;

static void alarm_handler()
{
	flag = 0;
}

int main(void)
{
	int ret;
	struct itimerval it = {.it_value = {.tv_sec = 0, .tv_usec = 100000}};

#if _POSIX_THREAD_CPUTIME == -1
	printf("_POSIX_THREAD_CPUTIME not supported\n");
	return PTS_UNSUPPORTED;
#endif
	pthread_t new_th;

	if (sysconf(_SC_THREAD_CPUTIME) == -1) {
		printf("_POSIX_THREAD_CPUTIME not supported\n");
		return PTS_UNSUPPORTED;
	}

	/* Let's do some work in the main thread so that it's cputime > 0 */
	if (signal(SIGVTALRM, alarm_handler)) {
		perror("signal()");
		return PTS_UNRESOLVED;
	}

	if (setitimer(ITIMER_VIRTUAL, &it, NULL)) {
		perror("setitimer(ITIMER_VIRTUAL, ...)");
		return PTS_UNRESOLVED;
	}

	while (flag);

	ret = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (ret) {
		fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	pthread_join(new_th, NULL);
	printf("Test PASSED\n");
	return PTS_PASS;
}
