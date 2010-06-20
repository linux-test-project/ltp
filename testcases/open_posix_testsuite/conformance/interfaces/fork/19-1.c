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
* The opened message queue descriptors are copied to the child process and 
* refer to the same object.

* The steps are:
* -> Open a message queue descriptor.
* -> Send a message to this descriptor.
* -> fork
* -> check if that the child's message count for this descriptor is 1.

* The test fails if the child reports 0 message count
*  or if it fails to read the descriptor.

*/


/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

/* The main test function. */
int main( int argc, char * argv[] )
{
	int ret, status;
	pid_t child, ctl;

	mqd_t mq;
	char rcv[ 20 ];

	struct mq_attr mqa;


	/* Initialize output */
	output_init();

	/* Create a message queue descriptor */
	mqa.mq_maxmsg = 2;
	mqa.mq_msgsize = 20;

	mq = mq_open( "/fork_19_1_mq"
	              , O_RDWR | O_CREAT | O_NONBLOCK
	              , S_IRUSR | S_IWUSR
	              , &mqa );

	if ( mq == ( mqd_t ) - 1 )
	{
		UNRESOLVED( errno, "Failed to create the message queue descriptor" );
	}

	/* Send 1 message to this message queue */
	ret = mq_send( mq, "I'm your father...", 19, 0 );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to send the message" );
	}

	/* Check the message has been queued */
	ret = mq_getattr( mq, &mqa );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to get message queue attributes" );
	}

	if ( mqa.mq_curmsgs != 1 )
	{
		UNRESOLVED( -1, "The queue information does not show the new message" );
	}


	/* Create the child */
	child = fork();

	if ( child == ( pid_t ) - 1 )
	{
		UNRESOLVED( errno, "Failed to fork" );
	}

	/* child */
	if ( child == ( pid_t ) 0 )
	{
		ret = mq_getattr( mq, &mqa );

		if ( ret != 0 )
		{
			FAILED( "Failed to get message queue attributes in child" );
		}

		if ( mqa.mq_curmsgs != 1 )
		{
			FAILED( "The queue information does not show the message in child" );
		}

		/* Now, receive the message */
		ret = mq_receive( mq, rcv, 20, NULL );

		if ( ret != 19 )  /* expected message size */
		{
			UNRESOLVED( errno, "Failed to receive the message" );
		}

#if VERBOSE > 0
		output( "Received message: %s\n", rcv );

#endif

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

	/* Check the message has been unqueued */
	ret = mq_getattr( mq, &mqa );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to get message queue attributes the 2nd time" );
	}

	if ( mqa.mq_curmsgs != 0 )
	{
		FAILED( "The message received in child was not dequeued." );
	}

	/* Test passed */
#if VERBOSE > 0
	output( "Test passed\n" );

#endif
	PASSED;
}

