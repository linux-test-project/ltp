/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Steps:
 1. Set the signal mask to only having SIGABRT.
 2. Call pthread_sigmask again, this time with a randomly generated
 value of  how that is checked to make sure it does not equal any of the three defined
 values of how which are SIG_SETMASK, SIG_BLOCK, or SIG_UNBLOCK. This should
 cause pthread_sigmask() to return -1. For the second parameter in the
 pthread_sigmask() function, use a set which contains SIGABRT and SIGALRM.
 3. Now verify using the is_changed() function that the only signal that is still
 in the signal mask is SIGABRT. Neither SIGALRM nor any other signal should be
 in the signal mask of the process.

*/

#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define NUMSIGNALS (sizeof(siglist) / sizeof(siglist[0]))

int is_changed(sigset_t set, int sig)
{

	int i;
	int siglist[] = { SIGABRT, SIGALRM, SIGBUS, SIGCHLD,
		SIGCONT, SIGFPE, SIGHUP, SIGILL, SIGINT,
		SIGPIPE, SIGQUIT, SIGSEGV,
		SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU,
		SIGUSR1, SIGUSR2,
#ifdef SIGPOLL
		SIGPOLL,
#endif
#ifdef SIGPROF
		SIGPROF,
#endif
		SIGSYS,
		SIGTRAP, SIGURG, SIGVTALRM, SIGXCPU, SIGXFSZ
	};

	if (sigismember(&set, sig) != 1) {
		return 1;
	}
	for (i = 0; i < (int)NUMSIGNALS; i++) {
		if ((siglist[i] != sig)) {
			if (sigismember(&set, siglist[i]) != 0) {
				return 1;
			}
		}
	}
	return 0;
}

int get_rand()
{

	int r;
	r = rand();
	if ((r == SIG_BLOCK) || (r == SIG_SETMASK) || (r == SIG_UNBLOCK)) {
		r = get_rand();
	}
	return r;
}

void *a_thread_func()
{

	int r = get_rand();
	sigset_t actl, oactl;
/*
	printf("SIG_SETMASK=%d\n", SIG_SETMASK);
	printf("SIG_BLOCK=%d\n", SIG_BLOCK);
	printf("SIG_UNBLOCK=%d\n", SIG_UNBLOCK);
	printf("r=%d\n", r);
*/
	sigemptyset(&actl);
	sigemptyset(&oactl);
	sigaddset(&actl, SIGABRT);

	pthread_sigmask(SIG_SETMASK, &actl, NULL);

	sigaddset(&actl, SIGALRM);
	if (pthread_sigmask(r, &actl, NULL) != EINVAL) {
		perror
		    ("pthread_sigmask() did not fail even though invalid how parameter was passed to it.\n");
		pthread_exit((void *)1);
	}

	pthread_sigmask(SIG_SETMASK, NULL, &oactl);

	if (is_changed(oactl, SIGABRT)) {
		printf("FAIL: signal mask was changed. \n");
		pthread_exit((void *)-1);
	}

/*
	printf("sigismember(SIGABRT)=%d\n", sigismember(&oactl, SIGABRT));
	printf("sigismember(SIGALRM)=%d\n", sigismember(&oactl, SIGALRM));
*/

	printf("PASS: signal mask was not changed.\n");
	pthread_exit(NULL);

	/* To please some compilers */
	return NULL;
}

int main(void)
{

	int *thread_return_value;

	pthread_t new_thread;

	if (pthread_create(&new_thread, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating new thread\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_join(new_thread, (void *)&thread_return_value) != 0) {
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	if ((long)thread_return_value != 0) {
		if ((long)thread_return_value == 1) {
			printf("Test UNRESOLVED\n");
			return PTS_UNRESOLVED;
		} else if ((long)thread_return_value == -1) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		} else {
			printf("Test UNRESOLVED\n");
			return PTS_UNRESOLVED;
		}
	}
	return PTS_PASS;
}
