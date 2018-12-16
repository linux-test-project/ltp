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
 * The pthread_exit() routine never returns to its caller

 * The steps are:
 *
 * -> create some threads with different attributes
 * -> in the thread call pthread_exit
 * -> if the function returns, the test fails.
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
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
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

/* main is defined in the next file */
#define STD_MAIN
#include "../testfrmw/threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

/* Thread routine */
void *threaded(void *arg)
{
	int ret = 0;

	/* Signal we're done (especially in case of a detached thread) */
	do {
		ret = sem_post(&scenarii[sc].sem);
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	pthread_exit(arg);

	FAILED("pthread_exit() returned");

	/* Compiler complaisance */
	return NULL;
}
