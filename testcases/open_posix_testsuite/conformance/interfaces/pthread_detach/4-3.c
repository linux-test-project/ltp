/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * This sample test aims to check the following assertion:
 *
 * The function does not return EINTR
 *
 * The steps are:
 * -> kill a thread which calls pthread_detach()
 * -> check that EINTR is never returned
 *
 */

#define _POSIX_C_SOURCE 200112L

/* Some routines are part of the XSI Extensions */
#ifndef WITHOUT_XOPEN
#define _XOPEN_SOURCE	600
#endif

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
#include <time.h>
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

struct thestruct {
	int	sig;
#ifdef WITH_SYNCHRO
	sem_t	*sem;
#endif
};

/* the following function keeps on sending the signal to the process */
static void *sendsig(void *arg)
{
	struct thestruct *thearg = (struct thestruct *)arg;
	int ret;
	pid_t process;

	process = getpid();

	/* We block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &usersigs, NULL);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2"
			   " in signal thread");

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

static void sighdl1(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig1))
		UNRESOLVED(errno, "Sem_post in signal handler 1");
#endif
}

static void sighdl2(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig2))
		UNRESOLVED(errno, "Sem_post in signal handler 2");
#endif
}

static void *threaded(void *arg)
{
	int ret;

	/* We don't block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_UNBLOCK, &usersigs, NULL);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to unblock SIGUSR1 and SIGUSR2"
			   " in worker thread");

	ret = pthread_detach(pthread_self());
	if (ret == EINTR)
		FAILED("pthread_detach() returned EINTR");

	/* Signal we're done */
	do {
		ret = sem_post(&scenarii[sc].sem);
	} while ((ret == -1) && (errno == EINTR));
	if (ret == -1)
		UNRESOLVED(errno, "Failed to wait for the semaphore");

	return arg;
}

static void *test(void *arg)
{
	int ret = 0;
	pthread_t child;

	/* We block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &usersigs, NULL);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2"
			   " in signal thread");

	sc = 0;

	while (do_it2) {
#if VERBOSE > 5
		output("-----\n");
		output("Starting test with scenario (%i): %s\n",
		       sc, scenarii[sc].descr);
#endif

		count_ope++;

		ret = pthread_create(&child, &scenarii[sc].ta, threaded, NULL);
		switch (scenarii[sc].result) {
		case 0: /* Operation was expected to succeed */
			if (ret != 0)
				UNRESOLVED(ret, "Failed to create this thread");
			break;

		case 1: /* Operation was expected to fail */
			if (ret == 0)
				UNRESOLVED(-1, "An error was expected but the"
					   " thread creation succeeded");
			break;

		case 2: /* We did not know the expected result */
		default:
#if VERBOSE > 5
			if (ret == 0)
				output("Thread has been created successfully"
				       " for this scenario\n");
			else
				output("Thread creation failed with the error:"
				       " %s\n", strerror(ret));
#endif
			;
		}
		if (ret == 0) {
			/* Just wait for the thread to terminate */
			do {
				ret = sem_wait(&scenarii[sc].sem);
			} while ((ret == -1) && (errno == EINTR));
			if (ret == -1)
				UNRESOLVED(errno, "Failed to wait for the"
					   " semaphore");
		}

		/* Change thread attribute for the next loop */
		sc++;
		sc %= NSCENAR;
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int ret;
	pthread_t th_work, th_sig1, th_sig2;
	struct thestruct arg1, arg2;
	struct sigaction sa;

	output_init();
	scenar_init();

	/* We prepare a signal set which includes SIGUSR1 and SIGUSR2 */
	sigemptyset(&usersigs);
	ret = sigaddset(&usersigs, SIGUSR1);
	ret |= sigaddset(&usersigs, SIGUSR2);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to add SIGUSR1 or 2 to a signal set");

	/* We now block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &usersigs, NULL);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2"
			   " in main thread");

#ifdef WITH_SYNCHRO
	if (sem_init(&semsig1, 0, 1))
		UNRESOLVED(errno, "Semsig1  init");
	if (sem_init(&semsig2, 0, 1))
		UNRESOLVED(errno, "Semsig2  init");
#endif

	/* We need to register the signal handlers for the PROCESS */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighdl1;

	ret = sigaction(SIGUSR1, &sa, NULL);
	if (ret)
		UNRESOLVED(ret, "Unable to register signal handler1");
	sa.sa_handler = sighdl2;
	ret = sigaction(SIGUSR2, &sa, NULL);
	if (ret)
		UNRESOLVED(ret, "Unable to register signal handler2");

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

	/* Let's wait for a while now */
	sleep(1);

	/* Now stop the threads and join them */
	do {
		do_it1 = 0;
	} while (do_it1);

	sleep(1);

	do {
		do_it2 = 0;
	} while (do_it2 = 0);

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
	output("  %d thread detached.\n", count_ope);
#ifdef WITH_SYNCHRO
	output("  %d signals were sent meanwhile.\n", count_sig);
#endif
#endif
	PASSED;
}
