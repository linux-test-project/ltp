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

 * This file is a scalability test for the pthread_mutex_lock function.
 * The goal is to test if there is a limit on the number
 *  of concurrent mutex having threads pending.

 * The steps are:
 * -> create some mutex attributes objects
 * -> As long as nothing fails, do
 *    - create a thread.
 *       - this thread initializes a mutex with one of the mutex attributes
 *       - lock this mutex
 *       - create another thread which waits on the mutex (and hangs) then returns
 *       - wait for a condition
 *       - unlock the mutex.
 *       - join the thread
 * -> When a create operation fails, broadcast the condition then join every threads.
 *
 * Additional note:
 *    This test will test only N/2 parallel mutex, where N is the max number of threads.
 *    It would be possible to create N parallel mutex with a slightly different algorithme:
 *     the main thread owns each mutex, then creates a thread which will block.
 *    This test could be written too. The current algorithm will give more stress to
 *     the mutex threads queues mechanism, as the threads are always different.
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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
#define SCALABILITY_FACTOR 1	/* This is not used in this testcase */
#endif
#ifndef VERBOSE
#define VERBOSE 2
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

#ifndef WITHOUT_XOPEN
int types[] = {
	PTHREAD_MUTEX_NORMAL,
	PTHREAD_MUTEX_ERRORCHECK,
	PTHREAD_MUTEX_RECURSIVE,
	PTHREAD_MUTEX_DEFAULT
};
#endif

/* The condition used to signal the main thread to go to the next step */
pthread_cond_t cnd;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
char do_it;
unsigned long counter;

/* Mutex attribute objects and pointers */
pthread_mutexattr_t *pma[6];
#ifdef WITHOUT_XOPEN
pthread_mutexattr_t ma[1];
#else
pthread_mutexattr_t ma[5];
#endif

/* Test data type */
typedef struct _td {
	pthread_t child;
	int id;
	pthread_mutex_t mtx;
	int error;
	struct _td *next;	/* It is a chained list */
} testdata_t;

/* Thread attribute object */
pthread_attr_t ta;

/*****
 * Level 2 - grandchild function
 */
void *sub(void *arg)
{
	testdata_t *td = (testdata_t *) arg;
	td->error = pthread_mutex_lock(&(td->mtx));
	if (td->error != 0) {
		/* Print out the error */
		output("PROBLEM: Unable to lock the mutex in thread %i\n",
		       td->id);
	} else {
		td->error = pthread_mutex_unlock(&(td->mtx));
		if (td->error != 0) {
			UNRESOLVED(td->error,
				   "Mutex unlock failed. Mutex data was corrupted?");
		}
	}

	return NULL;
}

/*****
 * Level 1 - child function
 */
void *threaded(void *arg)
{
	testdata_t *td = (testdata_t *) arg;
	int ret;
	int ret_create;
	pthread_t ch;

	ret = pthread_mutex_lock(&m);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to lock 'm' in child");
	}
	/* Mark this thread as started */
	counter++;

	/* Initialize the mutex with the mutex attribute */
	ret = pthread_mutex_init(&(td->mtx), pma[td->id % 6]);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to initialize a mutex");
	}

	/* Lock the mutex */
	td->error = pthread_mutex_lock(&(td->mtx));
	if (td->error != 0) {
		/* If the lock failed, we stop now */
		ret = pthread_mutex_unlock(&m);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to unlock 'm' in child");
		}
		return NULL;
	}

	/* Create the child thread */
	ret_create = pthread_create(&ch, &ta, sub, arg);

	/* Wait for the condition */
	while (do_it) {
		ret = pthread_cond_wait(&cnd, &m);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to wait for condvar");
		}
	}
	ret = pthread_mutex_unlock(&m);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to unlock 'm' in child");
	}

	/* Unlock the mutex and release the child */
	ret = pthread_mutex_unlock(&(td->mtx));
	if (ret != 0) {
		UNRESOLVED(ret,
			   "Mutex unlock failed. Mutex data was corrupted?");
	}

	/* If the child exists, join it now */
	if (ret_create == 0) {
		ret = pthread_join(ch, NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "Grandchild join failed");
		}
	}

	/* Destroy the test mutex */
	ret = pthread_mutex_destroy(&(td->mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Test mutex destroy failed. Corrupted data?");
	}

	/* We're done */
	return NULL;
}

/*****
 * Level 0 - main function
 */
int main(int argc, char *argv[])
{
	int ret;
	int i;
	int errors;
	testdata_t sentinel;
	testdata_t *cur, *tmp;

	output_init();

#if VERBOSE > 1
	output("Test starting, initializing data\n");
#endif

	do_it = 1;
	errors = 0;
	counter = 0;
	sentinel.next = NULL;
	sentinel.id = 0;
	cur = &sentinel;

	/* Initialize the 6 pma objects */
	pma[0] = NULL;
	pma[1] = &ma[0];
	ret = pthread_mutexattr_init(pma[1]);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex attribute init failed");
	}
#ifdef WITHOUT_XOPEN
	/* We only have default attributes objects */
	pma[2] = pma[0];
	pma[4] = pma[0];
	pma[3] = pma[1];
	pma[5] = pma[1];
#if VERBOSE > 1
	output("Default mutex attribute object was initialized\n");
#endif
#else
	/* We can use the different mutex types */
	for (i = 0; i < 4; i++) {
		pma[i + 2] = &ma[i + 1];
		ret = pthread_mutexattr_init(pma[i + 2]);
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex attribute init failed");
		}
		ret = pthread_mutexattr_settype(pma[i + 2], types[i]);
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex attribute settype failed");
		}
	}
#if VERBOSE > 1
	output("%d types of mutex attribute objects were initialized\n",
	       sizeof(types) / sizeof(types[0]));
#endif
#endif

	/* Initialize the thread attribute object */
	ret = pthread_attr_init(&ta);
	if (ret != 0) {
		UNRESOLVED(ret, "Thread attribute init failed");
	}
	ret = pthread_attr_setstacksize(&ta, sysconf(_SC_THREAD_STACK_MIN));
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to set stack size to minimum value");
	}

	/* Lock m */
	ret = pthread_mutex_lock(&m);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to lock 'm' in main");
	}
#if VERBOSE > 1
	output("Ready to create the threads, processing...\n");
#endif

	/* create the threads */
	while (1) {
		tmp = malloc(sizeof(testdata_t));
		if (tmp == NULL) {
			/* We cannot create anymore testdata */
			break;
		}

		/* We have a new test data structure */
		ret = pthread_create(&(tmp->child), &ta, threaded, tmp);
		if (ret != 0) {
			/* We cannot create more threads */
			free((void *)tmp);
			break;
		}

		cur->next = tmp;
		tmp->id = cur->id + 1;
		tmp->error = 0;
		cur = tmp;

		/* The new thread was created, let's start it */
		do {
			/* Unlock m so the thread can acquire it */
			ret = pthread_mutex_unlock(&m);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unlock 'm' failed in main loop");
			}
			/* Make sure the thread has a chance to run */
			sched_yield();
			/* Get m back */
			ret = pthread_mutex_lock(&m);
			if (ret != 0) {
				UNRESOLVED(ret, "Lock 'm' failed in main loop");
			}
			/* If the counter has been incremented, this means this child is in the cond wait loop */
		} while (counter != cur->id);
	}

	/* Unable to create more threads, let's signal the cond and join the threads */
#if VERBOSE > 1
	if (tmp == NULL) {
		output("Cannot malloc more memory for the test data.\n");
	} else {
		output("Cannot create another thread (error: %d).\n", ret);
	}
	output("The children will now be signaled.\n");
#endif
	do_it = 0;
	ret = pthread_cond_broadcast(&cnd);
	if (ret != 0) {
		UNRESOLVED(ret, "Cond broadcast failed");
	}

	ret = pthread_mutex_unlock(&m);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to unlock m after broadcast");
	}
#if VERBOSE > 1
	output("The children are terminating. We will join them.\n");
#endif

	/* All the threads are terminating, we can join the children and destroy the testdata */
	cur = &sentinel;
	while (cur->next != NULL) {
		/* Remove the first item from the list */
		tmp = cur->next;
		cur->next = tmp->next;

		/* Join the thread from the current item */
		ret = pthread_join(tmp->child, NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to join a child");
		}

		/* get the useful data */
		if (tmp->error != 0)
			errors++;

		/* Free the memory */
		free((void *)tmp);
	}

	/* We are done */

	/* Exit */
	if (errors == 0) {
#if VERBOSE > 1
		output("The test passed successfully.\n");
		output("  %i mutex were created and locked.\n", counter);
		output("  No error was encountered\n");
#endif
		PASSED;
	} else {
#if VERBOSE > 0
		output("The test failed.\n");
		output("  %i mutex were created.\n", counter);
		output("  %i lock operation failed.\n", errors);
#endif
		FAILED("There may be an issue in scalability");
	}
}
