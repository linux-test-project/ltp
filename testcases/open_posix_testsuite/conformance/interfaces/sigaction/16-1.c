/*
 * Copyright (c) 2011, Novell Inc.  All rights reserved.
 * Author: Peter W. Morreale <pmorreale@novell.com>
 *
 * Based on a similar original program written by Sebastien Decugis
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
 * This sample test aims to check the following assertions:
 *
 * If SA_RESTART is set in sa_flags, interruptible function interrupted
 * by signal shall restart silently.
 *
 */


/* This test tests for an XSI feature */

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <posixtest.h>

/*
 * Define an array of signals we want to test against.
 * Add more if desired.
 */

struct sig_info {
	int sig;
	char *sig_name;
	char caught;
};

static struct sig_info sigs[] = {
	{SIGHUP, "SIGHUP", 0},
	{SIGINT, "SIGINT", 0},
	{SIGQUIT, "SIGQUIT", 0},
	{SIGILL, "SIGILL", 0},
	{SIGTRAP, "SIGTRAP", 0},
	{SIGABRT, "SIGABRT", 0},
	{SIGBUS, "SIGBUS", 0},
	{SIGFPE, "SIGFPE", 0},
	{SIGUSR1, "SIGUSR1", 0},
	{SIGSEGV, "SIGSEGV", 0},
	{SIGUSR2, "SIGUSR2", 0},
	{SIGPIPE, "SIGPIPE", 0},
	{SIGALRM, "SIGALRM", 0},
	{SIGTERM, "SIGTERM", 0},
#ifdef SIGSTKFLT
	{SIGSTKFLT, "SIGSTKFLT", 0},
#endif
	{SIGCHLD, "SIGCHLD", 0},
	{SIGCONT, "SIGCONT", 0},
	{SIGTSTP, "SIGTSTP", 0},
	{SIGTTIN, "SIGTTIN", 0},
	{SIGTTOU, "SIGTTOU", 0},
	{SIGURG, "SIGURG", 0},
	{SIGXCPU, "SIGXCPU", 0},
	{SIGXFSZ, "SIGXFSZ", 0},
	{SIGVTALRM, "SIGVTALRM", 0},
	{SIGPROF, "SIGPROF", 0},
	{SIGWINCH, "SIGWINCH", 0},
	{SIGPOLL, "SIGPOLL", 0},
	{-1, NULL, 0}		/* add  real time sigs? */
};

static volatile int ready;
static sem_t sem;

/* Lookup */
struct sig_info *lookup(int signo)
{
	struct sig_info *s = &sigs[0];

	while (s->sig > 0) {
		if (s->sig == signo)
			return s;
		s++;
	}
	return NULL;
}

/* Handler function */
void handler(int signo)
{
	struct sig_info *s;

	s = lookup(signo);
	if (s)
		s->caught = 1;
}

/* Thread function */
void *threaded(void *arg)
{
	int rc;
	int status = PTS_PASS;
	struct sched_param sp = { 10 };
	struct sig_info *s = arg;

	/*
	 * Move into SCHED_FIFO to help ensure we are waiting in
	 * sem_wait when the signal is delivered
	 */
	rc = pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);
	if (rc) {
		printf("Failed: pthread_setschedparam(SCHED_FIFO), root?\n");

		if (rc == EPERM)
			exit(PTS_UNTESTED);
		else
			exit(PTS_UNRESOLVED);
	}

	ready = 1;

	rc = sem_wait(&sem);
	if (rc) {
		status = PTS_UNRESOLVED;
		printf("Failed: sem_wait(): errno: %s signal: %s\n",
		       strerror(errno), s->sig_name);
		if (errno == EINTR)
			status = PTS_FAIL;
	}

	return (void *)((long)status);
}

int test_sig(struct sig_info *s)
{
	int rc;
	int status = PTS_UNRESOLVED;
	pthread_t child;
	char *label;
	void *thread_status;

	label = "sem_init()";
	rc = sem_init(&sem, 0, 0);
	if (rc)
		goto done;

	/* reset flag */
	ready = 0;

	label = "pthread_create()";
	errno = pthread_create(&child, NULL, threaded, s);
	if (errno)
		goto done;

	/*
	 * sync on the ready flag.  Since the child is running in
	 * SCHED_FIFO, it likely will continue running and wind up in
	 * sem_wait() prior to this thread sending a signal.
	 *
	 * Do one more yield for good luck.
	 */
	while (!ready)
		sched_yield();
	sched_yield();

	label = "pthread_kill()";
	errno = pthread_kill(child, s->sig);
	if (errno)
		goto done;

	while (!s->caught)
		sched_yield();

	label = "sem_post()";
	rc = sem_post(&sem);
	if (rc)
		goto done;

	label = "pthread_join()";
	errno = pthread_join(child, &thread_status);
	if (errno)
		goto done;

	sem_destroy(&sem);

	status = ((long)thread_status) & 0xFFFFFFFF;

	return status;

done:
	printf("Failed: func: %s, rc: %d errno: %s signal: %s\n",
	       label, rc, strerror(errno), s->sig_name);
	return status;
}

int main(void)
{
	int rc = 0;
	struct sig_info *s = &sigs[0];
	struct sigaction sa;
	struct sigaction sa_org;

	sa.sa_flags = SA_RESTART;
	sa.sa_handler = handler;

	while (s->sig > 0) {
		sigemptyset(&sa.sa_mask);
		rc = sigaction(s->sig, &sa, &sa_org);
		if (rc)
			goto done;

		rc = test_sig(s);
		if (rc != PTS_PASS)
			break;

		sigaction(s->sig, &sa_org, NULL);
		s++;
	}

	if (rc == PTS_PASS)
		printf("Test PASSED\n");

	return rc;

done:
	printf("Failed: sigaction(): errno: %s, signal: %s\n",
	       strerror(errno), s->sig_name);
	return PTS_FAIL;
}
