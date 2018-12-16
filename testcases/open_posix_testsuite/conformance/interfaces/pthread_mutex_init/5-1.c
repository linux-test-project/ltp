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

 * This sample test aims to check the following assertion:
 * The function fails and returns ENOMEM if there is not enough memory.

 * The steps are:
 * * Fork
 * * New process sets its memory resource limit to a minimum value, then
 *  -> Allocate all the available memory
 *  -> call pthread_mutex_init()
 *  -> free the memory
 *  -> Checks that pthread_mutex_init() returned 0 or ENOMEM.
 * * Parent process waits for the child.
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <stdarg.h>

/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"
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
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

int main(void)
{
	pid_t child;

	pthread_mutex_t mtx;
	pthread_mutexattr_t ma[4];
	pthread_mutexattr_t *pma[5];

	int ret = 0;
	int i;
	int retini[5] = { -1, -1, -1, -1, -1 };
	int retdtr[5] = { -1, -1, -1, -1, -1 };

	void *ptr, *ptr_prev = NULL;

	int sz = 0;
	struct rlimit rl;

	int status = 0;

	output_init();

	child = fork();

	if (child == (pid_t) - 1) {
		UNRESOLVED(errno, "Fork failed");
	}

	if (child != 0) {	/* We are the father */
		if (child != waitpid(child, &status, 0)) {
			UNRESOLVED(errno, "Waitpid failed");
		}

		if (WIFSIGNALED(status)) {
			UNRESOLVED(WTERMSIG(status),
				   "The child process was killed.");
		}

		if (WIFEXITED(status))
			return WEXITSTATUS(status);

		UNRESOLVED(0, "Child process neither returned nor was killed.");
	}

	/* Only the child goes further */

	/* We initialize the different mutex attributes */
	for (i = 0; (i < 4) && (ret == 0); i++) {
		pma[i] = &ma[i];
		ret = pthread_mutexattr_init(pma[i]);
	}
	if (ret) {
		UNRESOLVED(ret, "Mutex attribute init failed");
	}
	pma[4] = NULL;

	if ((ret = pthread_mutexattr_settype(pma[0], PTHREAD_MUTEX_NORMAL))) {
		UNRESOLVED(ret, "Mutex attribute NORMAL failed");
	}
	if ((ret = pthread_mutexattr_settype(pma[0], PTHREAD_MUTEX_DEFAULT))) {
		UNRESOLVED(ret, "Mutex attribute DEFAULT failed");
	}
	if ((ret = pthread_mutexattr_settype(pma[0], PTHREAD_MUTEX_RECURSIVE))) {
		UNRESOLVED(ret, "Mutex attribute RECURSIVE failed");
	}
	if ((ret = pthread_mutexattr_settype(pma[0], PTHREAD_MUTEX_ERRORCHECK))) {
		UNRESOLVED(ret, "Mutex attribute ERRORCHECK failed");
	}

	sz = sysconf(_SC_PAGESIZE);

	/* Limit the process memory to a small value (64Mb for example). */
	rl.rlim_max = 1024 * 1024 * 64;
	rl.rlim_cur = 1024 * 1024 * 64;
	if ((ret = setrlimit(RLIMIT_AS, &rl))) {
		UNRESOLVED(ret, "Memory limitation failed");
	}
#if VERBOSE > 1
	output("Ready to take over memory. Page size is %d\n", sz);
#endif

	/* Allocate all available memory */
	while (1) {
		ptr = malloc(sz);	/* Allocate one page of memory */
		if (ptr == NULL)
			break;
#if VERBOSE > 1
		ret++;
#endif
		*(void **)ptr = ptr_prev;	/* Write into the allocated page */
		ptr_prev = ptr;
	}
#if VERBOSE > 1
	output("%d pages were allocated before failure\n", ret);
	ret = 0;
#endif

	while (1) {
		ptr = malloc(sizeof(void *));	/* Allocate every remaining bits of memory */
		if (ptr == NULL)
			break;
#if VERBOSE > 1
		ret++;
#endif
		*(void **)ptr = ptr_prev;	/* Keep track of allocated memory */
		ptr_prev = ptr;
	}
#if VERBOSE > 1
	output("%d additional spaces were allocated before failure\n", ret);
	ret = 0;
#endif
	if (errno != ENOMEM)
		UNRESOLVED(errno, "Memory not full");

	/* Now that memory is full, we try to initialize a mutex */
	for (i = 0; i < 5; i++) {
		retini[i] = pthread_mutex_init(&mtx, pma[i]);
		if (!retini[i])	/* If mutex has been initialized, we destroy it */
			retdtr[i] = pthread_mutex_destroy(&mtx);
	}

	/* We can now free the memory */
	while (ptr_prev != NULL) {
		ptr = ptr_prev;
		ptr_prev = *(void **)ptr;
		free(ptr);
	}

#if VERBOSE > 1
	output("Memory is released\n");
#endif

	for (i = 0; i < 4; i++)
		pthread_mutexattr_destroy(pma[i]);

	for (i = 0; i < 5; i++) {
		if (retini[i] != 0 && retini[i] != ENOMEM) {
			FAILED
			    ("Mutex init returned a wrong error code when no memory was left");
		}

		if (retini[i] == 0) {
#if VERBOSE > 0
			output
			    ("Mutex initialization for attribute %d succeeds when memory is full\n",
			     i);
#endif
			if (retdtr[i] != 0) {
				UNRESOLVED(retdtr[i],
					   "Mutex destroy failed on mutex inilialized under heavy loaded memory");
			}
		}
#if VERBOSE > 0
		else {
			output
			    ("Mutex initialization for attribute %d fails with ENOMEM when memory is full\n",
			     i);
		}
#endif
	}
	PASSED;
}
