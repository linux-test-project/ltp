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
* The function gets the scheduling policy and parameter of 
* the specified thread. The priority value is the one last 
* set with pthread_setschedparam, pthread_setschedprio or 
* pthread_create


* The steps are:
* -> create a new thread with a known scheduling policy & param.
* -> check the created thread has the required policy & param.
* -> change the policy with pthread_setschedparam & check the result.
* -> change the param with pthread_setschedprio & check the result.

* The test fails if an inconsistency is detected.

*/

/******************************************************************************/
/*********************** standard includes ************************************/
/******************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sched.h>
#include <errno.h>

/******************************************************************************/
/***********************   Test framework   ***********************************/
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
/***************************** Configuration **********************************/
/******************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/******************************************************************************/
/*****************************    Test case   *********************************/
/******************************************************************************/

/* This function checks the thread policy & priority */
void check_param( pthread_t thread, int policy, int priority )
{
	int ret = 0;

	int t_pol;

	struct sched_param t_parm;

	/* Check the priority is valid */

	if ( priority == -1 )
	{
		UNRESOLVED( errno, "Wrong priority value" );
	}

	/* Get the thread's parameters */
	ret = pthread_getschedparam( thread, &t_pol, &t_parm );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to get thread's parameters" );
	}

	if ( t_pol != policy )
	{
		FAILED( "The thread's policy is not as expected" );
	}

	if ( t_parm.sched_priority != priority )
	{
		FAILED( "The thread's priority is not as expected" );
	}
}

/* thread function */
void * threaded ( void * arg )
{
	int ret = 0;

	/* check the thread attributes have been applied
	  (we only check what is reported, not the real behavior) 
	 */
	check_param( pthread_self(), SCHED_RR, sched_get_priority_min( SCHED_RR ) );

	ret = pthread_barrier_wait( arg );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	/* Nothing to do while the main thread sets the scheduling policy */

	ret = pthread_barrier_wait( arg );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	/* Check the scheduling policy was changed
	  (we only check what is reported, not the real behavior) 
	 */
	check_param( pthread_self(), SCHED_FIFO, sched_get_priority_min( SCHED_FIFO ) );

	ret = pthread_barrier_wait( arg );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	/* Nothing to do while the main thread sets the scheduling parameter */

	ret = pthread_barrier_wait( arg );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	/* Check the scheduling parameter has been changed
	  (we only check what is reported, not the real behavior) 
	 */
	check_param( pthread_self(), SCHED_FIFO, sched_get_priority_max( SCHED_FIFO ) );

	ret = pthread_barrier_wait( arg );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	return NULL;
}

/* The main test function. */
int main( int argc, char *argv[] )
{
	int ret = 0;
	pthread_t child;
	pthread_attr_t ta;
	pthread_barrier_t bar;

	struct sched_param sp;

	/* Initialize output routine */
	output_init();

	ret = pthread_barrier_init( &bar, NULL, 2 );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to init barrier" );
	}

	/* Create the attribute object with a known scheduling policy */
	ret = pthread_attr_init( &ta );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to initialize thread attribute" );
	}

	ret = pthread_attr_setinheritsched( &ta, PTHREAD_EXPLICIT_SCHED );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to set inherit sched" );
	}

	sp.sched_priority = sched_get_priority_min( SCHED_RR );

	if ( sp.sched_priority == -1 )
	{
		UNRESOLVED( errno, "Failed to get min priority" );
	}

	ret = pthread_attr_setschedparam( &ta, &sp );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to set attribute param" );
	}

	ret = pthread_attr_setschedpolicy( &ta, SCHED_RR );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to set attribute policy" );
	}

	/* Create the thread with this attribute */
	ret = pthread_create( &child, &ta, threaded, &bar );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "thread creation failed (you may need more priviledges)" );
	}

	/* Wait while the thread checks its policy
	  (we only check what is reported, not the real behavior) 
	 */
	check_param( child, SCHED_RR, sched_get_priority_min( SCHED_RR ) );


	ret = pthread_barrier_wait( &bar );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	/* Change the threads policy */
	sp.sched_priority = sched_get_priority_min( SCHED_FIFO );

	if ( sp.sched_priority == -1 )
	{
		UNRESOLVED( errno, "Failed to get min priority" );
	}

	ret = pthread_setschedparam( child, SCHED_FIFO, &sp );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to change running thread's policy" );
	}

	ret = pthread_barrier_wait( &bar );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	/* Wait while the thread checks its policy
	  (we only check what is reported, not the real behavior) 
	 */
	check_param( child, SCHED_FIFO, sched_get_priority_min( SCHED_FIFO ) );

	ret = pthread_barrier_wait( &bar );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	/* Change the thread priority */
	sp.sched_priority = sched_get_priority_max( SCHED_FIFO );

	if ( sp.sched_priority == -1 )
	{
		UNRESOLVED( errno, "Failed to get max priority" );
	}

	ret = pthread_setschedprio( child, sp.sched_priority );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to raise thread's priority" );
	}

	ret = pthread_barrier_wait( &bar );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	/* The thread checks its priority
	  (we only check what is reported, not the real behavior) 
	 */
	check_param( child, SCHED_FIFO, sched_get_priority_max( SCHED_FIFO ) );

	ret = pthread_barrier_wait( &bar );

	if ( ( ret != 0 ) && ( ret != PTHREAD_BARRIER_SERIAL_THREAD ) )
	{
		UNRESOLVED( ret, "barrier wait failed" );
	}

	ret = pthread_join( child, NULL );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to join the thread" );
	}

	PASSED;
}


