/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
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
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.


* This sample test aims to check the following assertion:
*
*  If the semaphore count is 0, the call blocks until the semphore can be 
* locked or the operation is interrupted by a signal. 


* The steps are:
* -> Initialize a semaphore to 0 count
* -> Register a signal handler for SIGALRM which post the semaphore
* -> save current time
* -> set an alarm to 1 second
* -> sem_wait
* -> Verify either errno is EINTR or no error occured.
* -> Verify 1 sec has elapsed.

* The test fails if the call did not block.

*/


/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/******************************************************************************/
/*************************** standard includes ********************************/
/******************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <semaphore.h>
#include <signal.h>
#include <errno.h>

/******************************************************************************/
/***************************   Test framework   *******************************/
/******************************************************************************/
#include "testfrmw.h"
#include "testfrmw.c" 
/* This header is responsible for defining the following macros:
 * UNRESOLVED(ret, descr);  
 *    where descr is a description of the error and ret is an int 
 *   (error code for example)
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

/******************************************************************************/
/**************************** Configuration ***********************************/
/******************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/
sem_t sem;

void handler( int sig )
{
	int ret;
	ret = sem_post( &sem );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to post semaphore" );
	}
}


/* The main test function. */
int main( int argc, char * argv[] )
{
	int ret;

	struct timespec ts_ref, ts_fin;

	struct sigaction sa;

	/* Initialize output */
	output_init();

	/* Initialize semaphore */
	ret = sem_init( &sem, 0, 0 );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to init semaphore" );
	}

	/* Register signal handler */
	sa.sa_flags = 0;

	sa.sa_handler = handler;

	ret = sigemptyset( &sa.sa_mask );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to empty signal set" );
	}

	sigaction( SIGALRM, &sa, 0 );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to set signal handler" );
	}

	/* Save current time */
	ret = clock_gettime( CLOCK_REALTIME, &ts_ref );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Unable to read clock" );
	}

	/* Alarm */
	alarm( 1 );

	/* Wait for the semaphore */
	ret = sem_wait( &sem );

	if ( ( ret != 0 ) && ( errno != EINTR ) )
	{
		UNRESOLVED( errno, "Failed to wait for the semaphore" );
	}

	/* Check that 1 second has really elapsed */
	ret = clock_gettime( CLOCK_REALTIME, &ts_fin );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Unable to read clock" );
	}

	if ( ( ( ts_fin.tv_sec - ts_ref.tv_sec ) * 1000000000 ) + ( ts_fin.tv_nsec - ts_ref.tv_nsec ) < 1000000000 )
	{
		output( "Ts: %d.%9.9d  ->  %d.%9.9d\n", ts_ref.tv_sec, ts_ref.tv_nsec, ts_fin.tv_sec, ts_fin.tv_nsec );
		FAILED( "The sem_wait call did not block" );
	}

	/* Destroy the semaphore */
	ret = sem_destroy( &sem );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to sem_destroy" );
	}

	/* Test passed */
#if VERBOSE > 0

	output( "Test passed\n" );

#endif

	PASSED;
}


