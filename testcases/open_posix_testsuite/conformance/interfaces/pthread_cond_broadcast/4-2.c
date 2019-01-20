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
 * The function does not return an error code of EINTR

 * The steps are:
 *
 * -> Create some threads which wait for a condition.
 * -> Create a worker thread which broadcasts this condition.
 * -> Another thread loops on killing the worker thread.
 *
 */


#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#define WITH_SYNCHRO
#ifndef VERBOSE
#define VERBOSE 2
#endif

static char do_it = 1;
static char woken;
static unsigned long count_cnd_sig, count_cnd_wup;
#ifdef WITH_SYNCHRO
sem_t semsig1;
sem_t semsig2;
static unsigned long count_sig;
#endif

static sigset_t usersigs;

struct thestruct {
	int sig;
#ifdef WITH_SYNCHRO
	sem_t *sem;
#endif
};

struct {
	pthread_mutex_t mtx;
	pthread_cond_t cnd;
} data;

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
		UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2 "
			   "in signal thread");

	while (do_it) {
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
static void sighdl1(int sig LTP_ATTRIBUTE_UNUSED)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig1))
		UNRESOLVED(errno, "Sem_post in signal handler 1");
#endif
}

/* This one is registered for signal SIGUSR2 */
static void sighdl2(int sig LTP_ATTRIBUTE_UNUSED)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig2))
		UNRESOLVED(errno, "Sem_post in signal handler 2");
#endif
}

/* The following function will wait on the cond
 * it does check that no error code of EINTR is returned */
static void *waiter(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret;

	/* We block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &usersigs, NULL);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2 "
			   "in signal thread");

	ret = pthread_mutex_lock(&(data.mtx));
	if (ret != 0)
		UNRESOLVED(ret, "Unable to lock mutex in waiter thread");

	do {
		ret = pthread_cond_wait(&(data.cnd), &(data.mtx));
		count_cnd_wup++;
	} while ((ret == 0) && (do_it != 0));
	if (ret != 0)
		UNRESOLVED(ret,
			   "pthread_cond_wait returned an unexpected error");
	woken++;

	ret = pthread_mutex_unlock(&(data.mtx));
	if (ret != 0)
		UNRESOLVED(ret, "Unable to unlock mutex in waiter thread");

	return NULL;
}

/* The next function will signal the condition */
static void *worker(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret = 0;

	/* We don't block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_UNBLOCK, &usersigs, NULL);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to unblock SIGUSR1 and SIGUSR2 "
			   "in worker thread");

	while (woken < 5) {
		ret = pthread_cond_broadcast(&(data.cnd));
		if (ret == EINTR)
			FAILED("pthread_cond_signal returned EINTR");
		if (ret != 0)
			UNRESOLVED(ret, "Failed to signal the condition");
		count_cnd_sig++;
	}

	return NULL;
}

int main(void)
{
	int ret, i;
	pthread_t th_waiter[5], th_worker, th_sig1, th_sig2;
	struct thestruct arg1, arg2;
	struct sigaction sa;

	output_init();

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

	for (i = 0; i < 5; i++) {
		ret = pthread_create(&th_waiter[i], NULL, waiter, NULL);
		if (ret)
			UNRESOLVED(ret, "Waiter thread creation failed");
	}

	ret = pthread_create(&th_worker, NULL, worker, NULL);
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
		do_it = 0;
	} while (do_it);

	ret = pthread_join(th_sig1, NULL);
	if (ret)
		UNRESOLVED(ret, "Signal 1 sender thread join failed");
	ret = pthread_join(th_sig2, NULL);
	if (ret)
		UNRESOLVED(ret, "Signal 2 sender thread join failed");
	for (i = 0; i < 5; i++) {
		ret = pthread_join(th_waiter[i], NULL);
		if (ret)
			UNRESOLVED(ret, "Waiter thread join failed");
	}
	ret = pthread_join(th_worker, NULL);
	if (ret)
		UNRESOLVED(ret, "Worker thread join failed");

#if VERBOSE > 0
	output("Test executed successfully.\n");
	output("  Condition was signaled %d times.\n", count_cnd_sig);
	output("  pthread_cond_wait exited %d times.\n", count_cnd_wup);
#ifdef WITH_SYNCHRO
	output("  %d signals were sent meanwhile.\n", count_sig);
#endif
#endif
	PASSED;
}
