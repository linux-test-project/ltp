/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis
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
* This sample test aims to check the following assertion:
*
* The function does not return EINTR
*
* The steps are:
* -> kill a thread which calls pthread_join
* -> check that EINTR is never returned
*/


#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#define WITH_SYNCHRO

#include "../testfrmw/threads_scenarii.c"

static char do_it1 = 1;
static char do_it2 = 1;
static unsigned long count_ope;

#ifdef WITH_SYNCHRO
static sem_t semsig1;
static sem_t semsig2;
static unsigned long count_sig;
#endif

static sigset_t usersigs;
static unsigned int sc;

struct thestruct {
	int sig;
#ifdef WITH_SYNCHRO
	sem_t *sem;
#endif
};

static void *sendsig(void *arg)
{
	struct thestruct *thearg = (struct thestruct *)arg;
	int ret;
	pid_t process;

	process = getpid();

	/* We block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &usersigs, NULL);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2 "
			   "in signal thread");

	while (do_it1) {
#ifdef WITH_SYNCHRO
		ret = sem_wait(thearg->sem);
		if (ret)
			UNRESOLVED(errno, "Sem_wait in sendsig");

		count_sig++;
#endif
		ret = kill(process, thearg->sig);
		if (ret != 0)
			UNRESOLVED(errno, "Kill in sendsig");
	}

	return NULL;
}

/* Next are the signal handlers. */
/* This one is registered for signal SIGUSR1 */
static void sighdl1(int sig PTS_ATTRIBUTE_UNUSED)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig1))
		UNRESOLVED(errno, "Sem_post in signal handler 1");
#endif
}

/* This one is registered for signal SIGUSR2 */
static void sighdl2(int sig PTS_ATTRIBUTE_UNUSED)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig2))
		UNRESOLVED(errno, "Sem_post in signal handler 2");
#endif
}

static void *threaded(void *arg PTS_ATTRIBUTE_UNUSED)
{
	sched_yield();
	return NULL;
}

static void *test(void *arg PTS_ATTRIBUTE_UNUSED)
{
	int ret = 0;
	pthread_t child;

	/* We don't block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_UNBLOCK, &usersigs, NULL);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to unblock SIGUSR1 and SIGUSR2 "
			   "in worker thread");

	while (do_it2) {
		for (sc = 0; sc < NSCENAR; sc++) {
			if (scenarii[sc].detached == 1)
				continue;
			ret = pthread_create(&child, &scenarii[sc].ta,
					     threaded, NULL);

			if ((scenarii[sc].result == 0) && (ret != 0))
				UNRESOLVED(ret, "Failed to create this thread");

			if ((scenarii[sc].result == 1) && (ret == 0))

				UNRESOLVED(-1, "An error was expected but the "
					   "thread creation succeeded");

			if (ret == 0) {
				count_ope++;

				ret = pthread_join(child, NULL);
				if (ret == EINTR)
					FAILED("pthread_join returned EINTR");

				if (ret != 0)
					UNRESOLVED(ret, "Unable to join a "
						   "thread");

			}
		}
	}

	return NULL;
}

int main(void)
{
	int ret;
	pthread_t th_work, th_sig1, th_sig2;
	struct thestruct arg1, arg2;

	struct sigaction sa;

	output_init();

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighdl1;

	ret = sigaction(SIGUSR1, &sa, NULL);
	if (ret == -1)
		UNRESOLVED(ret, "Unable to register signal handler1");

	sa.sa_handler = sighdl2;
	ret = sigaction(SIGUSR2, &sa, NULL);
	if (ret == -1)
		UNRESOLVED(ret, "Unable to register signal handler2");

	/* We prepare a signal set which includes SIGUSR1 and SIGUSR2 */
	sigemptyset(&usersigs);

	ret = sigaddset(&usersigs, SIGUSR1);
	ret |= sigaddset(&usersigs, SIGUSR2);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to add SIGUSR1 or 2 to a signal set");

	/* We now block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &usersigs, NULL);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2 "
			   "in main thread");

#ifdef WITH_SYNCHRO
	if (sem_init(&semsig1, 0, 1))
		UNRESOLVED(errno, "Semsig1  init");

	if (sem_init(&semsig2, 0, 1))
		UNRESOLVED(errno, "Semsig2  init");

#endif

	/* Initialize thread attribute objects */
	scenar_init();

	ret = pthread_create(&th_work, NULL, test, NULL);
	if (ret)
		UNRESOLVED(ret, "Worker thread creation failed");

	arg1.sig = SIGUSR1;
	arg2.sig = SIGUSR2;
#ifdef WITH_SYNCHRO
	arg1.sem = &semsig1;
	arg2.sem = &semsig2;
#endif

	ret = pthread_create(&th_sig1, NULL, sendsig, (void *)&arg1);
	if (ret)
		UNRESOLVED(ret, "Signal 1 sender thread creation failed");

	ret = pthread_create(&th_sig2, NULL, sendsig, (void *)&arg2);
	if (ret)
		UNRESOLVED(ret, "Signal 2 sender thread creation failed");

	/* Now stop the threads and join them */
	do {
		do_it1 = 0;
	} while (do_it1);

	sleep(1);

	do {
		do_it2 = 0;
	} while (do_it2);

	ret = pthread_join(th_sig1, NULL);
	if (ret)
		UNRESOLVED(ret, "Signal 1 sender thread join failed");

	ret = pthread_join(th_sig2, NULL);
	if (ret)
		UNRESOLVED(ret, "Signal 2 sender thread join failed");

	ret = pthread_join(th_work, NULL);
	if (ret)
		UNRESOLVED(ret, "Worker thread join failed");

	scenar_fini();

#if VERBOSE > 0
	output("Test executed successfully.\n");

	output("  %d pthread_join calls.\n", count_ope);

#ifdef WITH_SYNCHRO
	output("  %d signals were sent meanwhile.\n", count_sig);

#endif
#endif
	PASSED;
}
