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
 * If the mutex type is PTHREAD_MUTEX_RECURSIVE,
 * and a thread attempts to unlock a mutex that it does not own,
 * an error is returned.

 * The steps are:
 *  -> Initialize and lock a recursive mutex
 *  -> create a child thread which tries to unlock this mutex. *
 */

 /*
  * - adam.li@intel.com 2004-05-20
  *   Add to PTS. Please refer to http://nptl.bullopensource.org/phpBB/
  *   for general information
  */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <errno.h>		/* needed for EPERM test */

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

pthread_mutex_t m;

/** child thread function **/
void *threaded(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret;
	ret = pthread_mutex_unlock(&m);
	if (ret == 0) {
		UNRESOLVED(ret,
			   "Unlocking a not owned recursive mutex succeeded");
	}

	if (ret != EPERM)	/* This is a "may" assertion */
		output
		    ("Unlocking a not owned recursive mutex did not return EPERM\n");

	return NULL;
}

/** parent thread function **/
int main(void)
{
	int ret;
	pthread_mutexattr_t ma;
	pthread_t th;

	output_init();

#if VERBOSE >1
	output("Initialize the PTHREAD_MUTEX_RECURSIVE mutex\n");
#endif

	ret = pthread_mutexattr_init(&ma);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex attribute init failed");
	}

	ret = pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
	if (ret != 0) {
		UNRESOLVED(ret, "Set type recursive failed");
	}

	ret = pthread_mutex_init(&m, &ma);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex init failed");
	}
#if VERBOSE >1
	output("Lock the mutex\n");
#endif

	ret = pthread_mutex_lock(&m);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex lock failed");
	}

	/* destroy the mutex attribute object */
	ret = pthread_mutexattr_destroy(&ma);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex attribute destroy failed");
	}
#if VERBOSE >1
	output("Create the thread\n");
#endif

	ret = pthread_create(&th, NULL, threaded, NULL);
	if (ret != 0) {
		UNRESOLVED(ret, "Thread creation failed");
	}

	/* Let the thread terminate */
	ret = pthread_join(th, NULL);
	if (ret != 0) {
		UNRESOLVED(ret, "Thread join failed");
	}
#if VERBOSE >1
	output("Joined the thread\n");
#endif

	/* We can clean everything and exit */
	ret = pthread_mutex_unlock(&m);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex unlock failed. Mutex got corrupted?");
	}

	PASSED;
}
