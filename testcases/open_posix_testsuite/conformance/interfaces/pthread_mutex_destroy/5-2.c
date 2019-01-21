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
 *
 * It is safe to destroy an initialized unlocked mutex.

 * The steps are:
 * -> Initialize a mutex with a given attribute.
 * -> Lock the mutex
 * -> unlock the mutex
 * -> Destroy the mutex -- this shall return 0.

 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
#ifndef WITHOUT_XOPEN

struct _scenar {
	int m_type;		/* Mutex type to use */
	int m_pshared;		/* 0: mutex is process-private (default) ~ !0: mutex is process-shared, if supported */
	char *descr;		/* Case description */
} scenarii[] = {
	{
	PTHREAD_MUTEX_DEFAULT, 0, "Default mutex"}
	, {
	PTHREAD_MUTEX_NORMAL, 0, "Normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 0, "Errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 0, "Recursive mutex"}

	, {
	PTHREAD_MUTEX_DEFAULT, 1, "Pshared mutex"}
	, {
	PTHREAD_MUTEX_NORMAL, 1, "Pshared Normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, "Pshared Errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, "Pshared Recursive mutex"}
};

#define NSCENAR (sizeof(scenarii)/sizeof(scenarii[0]))

/* Main function */
int main(void)
{
	int ret;
	unsigned int i;
	pthread_mutex_t mtx;
	pthread_mutexattr_t ma[NSCENAR + 1];
	pthread_mutexattr_t *pma[NSCENAR + 2];
	long pshared;

	/* Initialize output routine */
	output_init();

	/* System abilities */
	pshared = sysconf(_SC_THREAD_PROCESS_SHARED);

	/* Initialize the mutex attributes objects */
	for (i = 0; i < NSCENAR; i++) {
		ret = pthread_mutexattr_init(&ma[i]);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[parent] Unable to initialize the mutex attribute object");
		}

		/* Set the mutex type */
		ret = pthread_mutexattr_settype(&ma[i], scenarii[i].m_type);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to set mutex type");
		}

		/* Set the pshared attributes, if supported */
		if ((pshared > 0) && (scenarii[i].m_pshared != 0)) {
			ret =
			    pthread_mutexattr_setpshared(&ma[i],
							 PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the mutex process-shared");
			}
		}
	}
	/* Default mutexattr object */
	ret = pthread_mutexattr_init(&ma[i]);
	if (ret != 0) {
		UNRESOLVED(ret,
			   "[parent] Unable to initialize the mutex attribute object");
	}

	/* Initialize the pointer array */
	for (i = 0; i < NSCENAR + 1; i++)
		pma[i] = &ma[i];

	/* NULL pointer */
	pma[i] = NULL;

	/* Ok, we can now proceed to the test */
#if VERBOSE > 0
	output("Attributes are ready, proceed to the test\n");
#endif

	for (i = 0; i < NSCENAR + 2; i++) {
#if VERBOSE > 1
		char *nul = "NULL";
		char *def = "Default";
		char *stri;
		if (i < NSCENAR)
			stri = scenarii[i].descr;
		if (i == NSCENAR)
			stri = def;
		if (i == NSCENAR + 1)
			stri = nul;

		output("Init with: %s\n", stri);
#endif

		ret = pthread_mutex_init(&mtx, pma[i]);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to init the mutex");
		}

		ret = pthread_mutex_lock(&mtx);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to lock the mutex");
		}

		ret = pthread_mutex_unlock(&mtx);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to unlcok the mutex");
		}

		ret = pthread_mutex_destroy(&mtx);
		if (ret != 0) {
			FAILED
			    ("Failed to destroy an initialized unlocked mutex");
		}

	}

#if VERBOSE > 0
	output("Test passed; destroying the test data\n");
#endif

	for (i = 0; i < NSCENAR + 1; i++) {
		ret = pthread_mutexattr_destroy(&ma[i]);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to destroy a mutex attribute object");
		}
	}

	PASSED;
}

#else /* WITHOUT_XOPEN */
int main(void)
{
	output_init();
	UNTESTED("This test requires XSI features");
}
#endif
