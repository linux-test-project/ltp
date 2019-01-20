/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that if the signal specified by set does not become pending,
    the sigtimedwait() function shall wait for the time interval specified
    in the timespec structure referenced by timeout.

    NOTE: This program has commented out areas. The commented out functionality
    sets a timer in case sigtimedwait() never returns, to help the program
    from hanging. To make this program
    runnable on a typical system, I've commented out the timer functionality
    by default. However, if you do have a timers implementation on your
    system, then it is recommened that you uncomment the timers-related lines
    of code in this program.

    Steps:
    1. Register signal TIMERSIGNAL with the handler myhandler
   (2.)Create and set a timer that expires in TIMERSEC seconds incase sigtimedwait()
       never returns.
    3. Obtain time1.
    4. Call sigtimedwait() to wait for signal SIGTOTEST that will never be pending
    5. Obtain time2, and find the difference between time2 and time1.
    6. Verify that (time2-time1) is equal to SIGTIMEDWAITSEC within a reasonable
       error margin.
 */

#define _XOPEN_REALTIME 1

#define TIMERSIGNAL SIGUSR1
#define SIGTOTEST SIGUSR2
#define TIMERSEC 2
#define SIGTIMEDWAITSEC 1
#define ERRORMARGIN 0.1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "posixtest.h"

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf
	    ("Test FAILED: %d seconds have elapsed and sigtimedwait() has not yet returned.\n",
	     TIMERSEC);
	exit(PTS_FAIL);
}

int main(void)
{
	struct sigaction act;

	struct timeval time1, time2;
	double time_elapsed;

	sigset_t selectset;
	struct timespec ts;
/*
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;

        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
        its.it_value.tv_sec = TIMERSEC;
        its.it_value.tv_nsec = 0;

        ev.sigev_notify = SIGEV_SIGNAL;
        ev.sigev_signo = TIMERSIGNAL;
*/
	act.sa_flags = 0;
	act.sa_handler = myhandler;
	sigemptyset(&act.sa_mask);
	sigaction(TIMERSIGNAL, &act, 0);

	sigemptyset(&selectset);
	sigaddset(&selectset, SIGTOTEST);

	ts.tv_sec = SIGTIMEDWAITSEC;
	ts.tv_nsec = 0;
/*
        if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
                perror("timer_create() did not return success\n");
                return PTS_UNRESOLVED;
        }

        if (timer_settime(tid, 0, &its, NULL) != 0) {
                perror("timer_settime() did not return success\n");
                return PTS_UNRESOLVED;
        }
*/
	if (gettimeofday(&time1, NULL) == -1) {
		perror("gettimeofday()");
		return PTS_UNRESOLVED;
	}
	if (sigtimedwait(&selectset, NULL, &ts) != -1) {
		perror
		    ("sigtimedwait() did not return -1 even though signal was not pending\n");
		return PTS_UNRESOLVED;
	}
	if (gettimeofday(&time2, NULL) == -1) {
		perror("gettimeofday()");
		return PTS_UNRESOLVED;
	}

	time_elapsed = (time2.tv_sec - time1.tv_sec
		+ (time2.tv_usec - time1.tv_usec) / 1000000.0);

	if ((time_elapsed > SIGTIMEDWAITSEC + ERRORMARGIN)
	    || (time_elapsed < SIGTIMEDWAITSEC - ERRORMARGIN)) {
		printf("Test FAILED: sigtimedwait() did not return in "
			"the required time\n");
		printf("time_elapsed: %lf\n", time_elapsed);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
