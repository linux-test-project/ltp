/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

    Test that sigtimedwait() returns -1 upon unsuccessful completion.

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
    3. Call sigtimedwait() to wait for non-pending signal SIGTOTEST for SIGTIMEDWAITSEC
       seconds.
    4. Verify that sigtimedwait() returns a -1.
 */

#define _XOPEN_REALTIME 1

#define TIMERSIGNAL SIGUSR1
#define SIGTOTEST SIGUSR2
#define TIMERSEC 2
#define SIGTIMEDWAITSEC 0

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
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
	if (sigtimedwait(&selectset, NULL, &ts) != -1) {
		printf
		    ("Test FAILED: sigtimedwait() did not return with an error\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
