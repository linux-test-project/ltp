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
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.


* This sample test aims to check the following assertion:
*
* The new process has only one thread.

* The steps are:
* -> create a thread.
* -> fork
* -> Check that the thread is not running in the new process image.

* The test fails if the thread is executing in the child process.

*/


/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
 #include <stdarg.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>

#include <sys/wait.h>
 #include <errno.h>

#include <semaphore.h>
#include <fcntl.h>

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
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

sem_t * sem;

/* Thread function */
void * threaded( void * arg )
{
	int ret = 0;

	do
	{
		ret = sem_wait( sem );
	}
	while ( ( ret != 0 ) && ( errno == EINTR ) );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "failed to wait for the semaphore in child" );
	}

	if ( *( pid_t * ) arg != getpid() )
	{
		FAILED( "The thread is executing in the child process" );
	}

	return NULL;
}

/* The main test function. */
int main( int argc, char * argv[] )
{
	int ret, status;
	pid_t child, ctl;
	pthread_t th;

	/* Initialize output */
	output_init();

	ctl = getpid();

	/* Initialize the semaphore */
	sem = sem_open( "/fork_21_1", O_CREAT, O_RDWR, 0 );

	if ( sem == ( sem_t * ) SEM_FAILED )
	{
		UNRESOLVED( errno, "Failed to open the semaphore" );
	}

	sem_unlink( "/fork_21_1" );


	/* Create thread */
	ret = pthread_create( &th, NULL, threaded, &ctl );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to create the thread" );
	}


	/* Create the child */
	child = fork();

	if ( child == ( pid_t ) - 1 )
	{
		UNRESOLVED( errno, "Failed to fork" );
	}

	/* We post the semaphore twice */
	do
	{
		ret = sem_post( sem );
	}
	while ( ( ret != 0 ) && ( errno == EINTR ) );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to post the semaphore" );
	}

	/* child */
	if ( child == ( pid_t ) 0 )
	{
		/* sleep a little while to let the thread execute in case it exists */
		sleep( 1 );

		/* We're done */
		exit( PTS_PASS );
	}

	/* Parent joins the child */
	ctl = waitpid( child, &status, 0 );

	if ( ctl != child )
	{
		UNRESOLVED( errno, "Waitpid returned the wrong PID" );
	}

	if ( ( !WIFEXITED( status ) ) || ( WEXITSTATUS( status ) != PTS_PASS ) )
	{
		FAILED( "Child exited abnormally" );
	}

	/* Destroy everything */
	ret = sem_close( sem );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to close the semaphore" );
	}

	ret = pthread_join( th, NULL );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to join the thread in parent" );
	}

	/* Test passed */
#if VERBOSE > 0
	output( "Test passed\n" );

#endif
	PASSED;
}

