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
* The file locks are not inherited by the child process.

* The steps are:
* -> lock stdout
* -> fork
* -> child creates a thread
* -> child thread trylock stdout
* -> join the child

* The test fails if the child thread cannot lock the file 
* -- this would mean the child process got stdout file lock ownership.

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
/* Thread function */
void * threaded( void * arg )
{
	int ret;
	ret = ftrylockfile( stdout );

	if ( ret != 0 )
	{
		FAILED( "The child process is owning the file lock." );
	}

#if VERBOSE > 1

	output( "The file lock was not inherited in the child process\n" );

#endif

	return NULL;
}

/* The main test function. */
int main( int argc, char * argv[] )
{
	int ret, status;
	pid_t child, ctl;
	pthread_t ch;

	/* Initialize output */
	output_init();

	/* lock the stdout file */
	flockfile( stdout );

	/* Create the child */
	child = fork();

	if ( child == ( pid_t ) - 1 )
	{
		UNRESOLVED( errno, "Failed to fork" );
	}

	/* child */
	if ( child == ( pid_t ) 0 )
	{

		ret = pthread_create( &ch, NULL, threaded, NULL );

		if ( ret != 0 )
		{
			UNRESOLVED( ret, "Failed to create a thread" );
		}

		ret = pthread_join( ch, NULL );

		if ( ret != 0 )
		{
			UNRESOLVED( ret, "Failed to join the thread" );
		}

		/* We're done */
		exit( PTS_PASS );
	}

	/* Parent sleeps for a while to create contension in case the file lock is inherited */
	sleep( 1 );

	funlockfile( stdout );

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

	/* Test passed */
#if VERBOSE > 0

	output( "Test passed\n" );

#endif

	PASSED;
}


