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
 *

 * This file is a stress test for the pthread_mutex_lock function.

 * The steps are:
 * -> For each king of mutex, we create 10*F threads (F is a scalability factor)
 * -> we call those threads 1 to 10.
 *    -> thread 1 sends signal USR2 to the other 9 threads (which have a handler for it)
 *    -> thread 2 to 6 are loops
 *          {
 *               mutex_lock
 *               if (ctrl) exit
 *               ctrl = 1
 *               yield
 *               ctrl= 0
 *               mutex unlock
 *          }
 *     -> thread 7 & 8 have a timedlock instead of lock
 *     -> thread 9 & 10 have a trylock instead of lock
 *
 * -> the whole process stop when receiving signal SIGUSR1.
 *      This goal is achieved with a "do_it" variable.
 *
 * NOTE: With gcc/linux, the flag "-lrt" must be specified at link time.
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#if _POSIX_TIMEOUTS < 0
#error "This sample needs POSIX TIMEOUTS option support"
#endif
#if _POSIX_TIMEOUTS == 0
#warning "This sample needs POSIX TIMEOUTS option support"
#endif
#if _POSIX_TIMERS < 0
#error "This sample needs POSIX TIMERS option support"
#endif
#if _POSIX_TIMERS == 0
#warning "This sample needs POSIX TIMERS option support"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>		/* required for the pthread_mutex_timedlock() function */

/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "testfrmw.h"
#include "testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the test has failed.
  * PASSED();
  *    No parameter.
  *
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  *
  * The other file defines the functions
  * void output_init()
  * void output(char * string, ...)
  *
  * Those may be used to output information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef SCALABILITY_FACTOR
#define SCALABILITY_FACTOR 1
#endif
#ifndef VERBOSE
#define VERBOSE 2
#endif
#define N 2			/* N * 10 * 6 * SCALABILITY_FACTOR threads will be created */

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
char do_it = 1;
#ifndef WITHOUT_XOPEN
int types[] = { PTHREAD_MUTEX_NORMAL,
	PTHREAD_MUTEX_ERRORCHECK,
	PTHREAD_MUTEX_RECURSIVE,
	PTHREAD_MUTEX_DEFAULT
};
#endif

/* The following type represents the data
 * for one group of ten threads */
typedef struct {
	pthread_t threads[10];	/* The 10 threads */
	pthread_mutex_t mtx;	/* The mutex those threads work on */
	char ctrl;		/* The value used to check the behavior */
	char sigok;		/* Used to tell the threads they can return */
	sem_t semsig;		/* Semaphore for synchronizing the signal handler */
	int id;			/* An identifier for the threads group */
	int tcnt;		/* we need to make sure the threads are started before killing 'em */
	pthread_mutex_t tmtx;
	unsigned long long sigcnt, opcnt;	/* We count every iteration */
} cell_t;

pthread_key_t _c;		/* this key will always contain a pointer to the thread's cell */

/***** The next function is in charge of sending signal USR2 to
 * all the other threads in its cell, until the end of the test. */
void *sigthr(void *arg)
{
	int ret;
	int i = 0;
	cell_t *c = (cell_t *) arg;

	do {
		sched_yield();
		ret = pthread_mutex_lock(&(c->tmtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to lock the mutex");
		}
		i = c->tcnt;
		ret = pthread_mutex_unlock(&(c->tmtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to unlock the mutex");
		}
	} while (i < 9);

	/* Until we must stop, do */
	while (do_it) {
		/* Wait for the semaphore */
		ret = sem_wait(&(c->semsig));
		if (ret != 0) {
			UNRESOLVED(errno, "Sem wait failed in signal thread");
		}

		/* Kill the next thread */
		i %= 9;
		ret = pthread_kill(c->threads[++i], SIGUSR2);
		if (ret != 0) {
			UNRESOLVED(ret, "Thread kill failed in signal thread");
		}

		/* Increment the signal counter */
		c->sigcnt++;
	}

	/* Tell the other threads they can now stop */
	do {
		c->sigok = 1;
	}
	while (c->sigok == 0);

	return NULL;
}

/***** The next function is the signal handler
 * for all the others threads in the cell */
void sighdl(int sig)
{
	int ret;
	cell_t *c = (cell_t *) pthread_getspecific(_c);
	ret = sem_post(&(c->semsig));
	if (ret != 0) {
		UNRESOLVED(errno, "Unable to post semaphore in signal handler");
	}
}

/***** The next function can return only when the sigthr has terminated.
 * This avoids the signal thread try to kill a terminated thread. */
void waitsigend(cell_t * c)
{
	while (c->sigok == 0) {
		sched_yield();
	}
}

/***** The next function aims to control that no other thread
 * owns the mutex at the same time */
void control(cell_t * c, char *loc)
{
	*loc++;			/* change the local control value */
	if (c->ctrl != 0) {
		FAILED("Got a non-zero value - two threads owns the mutex");
	}
	c->ctrl = *loc;
	sched_yield();
	if (c->ctrl != *loc) {
		FAILED
		    ("Got a different value - another thread touched protected data");
	}
	c->ctrl = 0;

	/* Avoid some values for the next control */
	if (*loc == 120)
		*loc = -120;
	if (*loc == -1)
		*loc = 1;
}

/***** The next 3 functions are the worker threads
 */
void *lockthr(void *arg)
{
	int ret;
	char loc;		/* Local value for control */
	cell_t *c = (cell_t *) arg;

	/* Set the thread local data key value (used in the signal handler) */
	ret = pthread_setspecific(_c, arg);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to assign the thread-local-data key");
	}

	/* Signal we're started */
	ret = pthread_mutex_lock(&(c->tmtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to lock the mutex");
	}
	c->tcnt += 1;
	ret = pthread_mutex_unlock(&(c->tmtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to unlock the mutex");
	}

	do {
		/* Lock, control, then unlock */
		ret = pthread_mutex_lock(&(c->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex lock failed in worker thread");
		}

		control(c, &loc);

		ret = pthread_mutex_unlock(&(c->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex unlock failed in worker thread");
		}

		/* Increment the operation counter */
		c->opcnt++;
	}
	while (do_it);

	/* Wait for the signal thread to terminate before we can exit */
	waitsigend(c);
	return NULL;
}

void *timedlockthr(void *arg)
{
	int ret;
	char loc;		/* Local value for control */
	struct timespec ts;
	cell_t *c = (cell_t *) arg;

	/* Set the thread local data key value (used in the signal handler) */
	ret = pthread_setspecific(_c, arg);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to assign the thread-local-data key");
	}

	/* Signal we're started */
	ret = pthread_mutex_lock(&(c->tmtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to lock the mutex");
	}
	c->tcnt += 1;
	ret = pthread_mutex_unlock(&(c->tmtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to unlock the mutex");
	}

	do {
		/* Lock, control, then unlock */
		do {
			ret = clock_gettime(CLOCK_REALTIME, &ts);
			if (ret != 0) {
				UNRESOLVED(errno,
					   "Unable to get time for timeout");
			}
			ts.tv_sec++;	/* We will wait for 1 second */
			ret = pthread_mutex_timedlock(&(c->mtx), &ts);
		} while (ret == ETIMEDOUT);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Timed mutex lock failed in worker thread");
		}

		control(c, &loc);

		ret = pthread_mutex_unlock(&(c->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex unlock failed in worker thread");
		}

		/* Increment the operation counter */
		c->opcnt++;
	}
	while (do_it);

	/* Wait for the signal thread to terminate before we can exit */
	waitsigend(c);
	return NULL;
}

void *trylockthr(void *arg)
{
	int ret;
	char loc;		/* Local value for control */
	cell_t *c = (cell_t *) arg;

	/* Set the thread local data key value (used in the signal handler) */
	ret = pthread_setspecific(_c, arg);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to assign the thread-local-data key");
	}

	/* Signal we're started */
	ret = pthread_mutex_lock(&(c->tmtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to lock the mutex");
	}
	c->tcnt += 1;
	ret = pthread_mutex_unlock(&(c->tmtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to unlock the mutex");
	}

	do {
		/* Lock, control, then unlock */
		do {
			ret = pthread_mutex_trylock(&(c->mtx));
		} while (ret == EBUSY);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Mutex lock try failed in worker thread");
		}

		control(c, &loc);

		ret = pthread_mutex_unlock(&(c->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex unlock failed in worker thread");
		}

		/* Increment the operation counter */
		c->opcnt++;
	}
	while (do_it);

	/* Wait for the signal thread to terminate before we can exit */
	waitsigend(c);
	return NULL;
}

/***** The next function initializes a cell_t object
 * This includes running the threads */
void cell_init(int id, cell_t * c, pthread_mutexattr_t * pma)
{
	int ret, i;
	pthread_attr_t pa;	/* We will specify a minimal stack size */

	/* mark this group with its ID */
	c->id = id;
	/* Initialize some other values */
	c->sigok = 0;
	c->ctrl = 0;
	c->sigcnt = 0;
	c->opcnt = 0;
	c->tcnt = 0;

	/* Initialize the mutex */
	ret = pthread_mutex_init(&(c->tmtx), NULL);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex init failed");
	}
	ret = pthread_mutex_init(&(c->mtx), pma);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex init failed");
	}
#if VERBOSE > 1
	output("Mutex initialized in cell %i\n", id);
#endif

	/* Initialize the semaphore */
	ret = sem_init(&(c->semsig), 0, 0);
	if (ret != 0) {
		UNRESOLVED(errno, "Sem init failed");
	}
#if VERBOSE > 1
	output("Semaphore initialized in cell %i\n", id);
#endif

	/* Create the thread attribute with the minimal size */
	ret = pthread_attr_init(&pa);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to create pthread attribute object");
	}
	ret = pthread_attr_setstacksize(&pa, sysconf(_SC_THREAD_STACK_MIN));
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to specify minimal stack size");
	}

	/* Create the signal thread */
	ret = pthread_create(&(c->threads[0]), &pa, sigthr, (void *)c);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to create the signal thread");
	}

	/* Create 5 "lock" threads */
	for (i = 1; i <= 5; i++) {
		ret = pthread_create(&(c->threads[i]), &pa, lockthr, (void *)c);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to create a locker thread");
		}
	}

	/* Create 2 "timedlock" threads */
	for (i = 6; i <= 7; i++) {
		ret =
		    pthread_create(&(c->threads[i]), &pa, timedlockthr,
				   (void *)c);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to create a (timed) locker thread");
		}
	}

	/* Create 2 "trylock" threads */
	for (i = 8; i <= 9; i++) {
		ret =
		    pthread_create(&(c->threads[i]), &pa, trylockthr,
				   (void *)c);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to create a (try) locker thread");
		}
	}

#if VERBOSE > 1
	output("All threads initialized in cell %i\n", id);
#endif

	/* Destroy the thread attribute object */
	ret = pthread_attr_destroy(&pa);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to destroy thread attribute object");
	}

	/* Tell the signal thread to start working */
	ret = sem_post(&(c->semsig));
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to post signal semaphore");
	}
}

/***** The next function destroys a cell_t object
 * This includes stopping the threads */
void cell_fini(int id,
	       cell_t * c,
	       unsigned long long *globalopcount,
	       unsigned long long *globalsigcount)
{
	int ret, i;

	/* Just a basic check */
	if (id != c->id) {
		output("Something is wrong: Cell %i has id %i\n", id, c->id);
		FAILED("Some memory has been corrupted");
	}

	/* Start with joining the threads */
	for (i = 0; i < 10; i++) {
		ret = pthread_join(c->threads[i], NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to join a thread");
		}
	}

	/* Destroy the semaphore and the mutex */
	ret = sem_destroy(&(c->semsig));
	if (ret != 0) {
		UNRESOLVED(errno, "Unable to destroy the semaphore");
	}

	ret = pthread_mutex_destroy(&(c->mtx));
	if (ret != 0) {
		output("Unable to destroy the mutex in cell %i (ret = %i)\n",
		       id, ret);
		FAILED("Mutex destruction failed");
	}

	/* Report the cell counters */
	*globalopcount += c->opcnt;
	*globalsigcount += c->sigcnt;
#if VERBOSE > 1
	output
	    ("Counters for cell %i:\n\t%llu locks and unlocks\n\t%llu signals\n",
	     id, c->opcnt, c->sigcnt);
#endif

	/* We are done with this cell. */
}

/**** Next function is called when the process is killed with SIGUSR1
 * It tells every threads in every cells to stop their work.
 */
void globalsig(int sig)
{
	output("Signal received, processing. Please wait...\n");
	do {
		do_it = 0;
	}
	while (do_it);
}

/******
 * Last but not least, the main function
 */
int main(int argc, char *argv[])
{
	/* Main is responsible for :
	 * the mutex attributes initializing
	 * the creation of the cells
	 * the destruction of everything on SIGUSR1 reception
	 */

	int ret;
	int i;
	struct sigaction sa;
	unsigned long long globopcnt = 0, globsigcnt = 0;

#ifndef WITHOUT_XOPEN
	int sz = 2 + (sizeof(types) / sizeof(int));
#else
	int sz = 2;
#endif
	pthread_mutexattr_t ma[sz - 1];
	pthread_mutexattr_t *pma[sz];

	cell_t data[sz * N * SCALABILITY_FACTOR];

	pma[sz - 1] = NULL;

#if VERBOSE > 0
	output("Mutex lock / unlock stress sample is starting\n");
	output("Kill with SIGUSR1 to stop the process\n");
	output("\t kill -USR1 <pid>\n\n");
#endif

	/* Initialize the mutex attributes */
	for (i = 0; i < sz - 1; i++) {
		pma[i] = &ma[i];
		ret = pthread_mutexattr_init(pma[i]);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to init a mutex attribute object");
		}
#ifndef WITHOUT_XOPEN		/* we have the mutex attribute types */
		if (i != 0) {
			ret = pthread_mutexattr_settype(pma[i], types[i - 1]);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to set type of a mutex attribute object");
			}
		}
#endif
	}
#if VERBOSE > 1
	output("%i mutex attribute objects were initialized\n", sz - 1);
#endif

	/* Initialize the thread-local-data key */
	ret = pthread_key_create(&_c, NULL);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to initialize TLD key");
	}
#if VERBOSE > 1
	output("TLD key initialized\n");
#endif

	/* Register the signal handler for SIGUSR1  */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = globalsig;
	if ((ret = sigaction(SIGUSR1, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}

	/* Register the signal handler for SIGUSR2  */
	sa.sa_handler = sighdl;
	if ((ret = sigaction(SIGUSR2, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}

	/* Start every threads */
#if VERBOSE > 0
	output("%i cells of 10 threads are being created...\n",
	       sz * N * SCALABILITY_FACTOR);
#endif
	for (i = 0; i < sz * N * SCALABILITY_FACTOR; i++)
		cell_init(i, &data[i], pma[i % sz]);
#if VERBOSE > 0
	output("All threads created and running.\n");
#endif

	/* We stay here while not interrupted */
	do {
		sched_yield();
	}
	while (do_it);

#if VERBOSE > 0
	output("Starting to join the threads...\n");
#endif
	/* Everybody is stopping, we must join them, and destroy the cell data */
	for (i = 0; i < sz * N * SCALABILITY_FACTOR; i++)
		cell_fini(i, &data[i], &globopcnt, &globsigcnt);

	/* Destroy the mutex attributes objects */
	for (i = 0; i < sz - 1; i++) {
		ret = pthread_mutexattr_destroy(pma[i]);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to destroy a mutex attribute object");
		}
	}

	/* Destroy the thread-local-data key */
	ret = pthread_key_delete(_c);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to destroy TLD key");
	}
#if VERBOSE > 1
	output("TLD key destroyed\n");
#endif

	/* output the total counters */
#if VERBOSE > 1
	output("===============================================\n");
#endif
#if VERBOSE > 0
	output("Total counters:\n\t%llu locks and unlocks\n\t%llu signals\n",
	       globopcnt, globsigcnt);
	output("pthread_mutex_lock stress test passed.\n");
#endif

	PASSED;
}
