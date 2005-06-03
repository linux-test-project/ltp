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


* This sample test aims to check the following assertions:
*
* If SA_SIGINFO is set in sa_flags and Real Time Signals extension is supported,
* sa_sigaction is used as the signal handling function.

* The steps are:
* -> test for RTS extension
* -> register a handler for SIGVTALRM with SA_SIGINFO, and a known function
*   as sa_sigaction
* -> raise SIGVTALRM, and check the function has been called.

* The test fails if the function is not called
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

#define SIGNAL SIGVTALRM

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

int called = 0;
void handler( int sig, siginfo_t *info, void *context )
{
	if ( info->si_signo != SIGNAL )
	{
		FAILED( "Wrong signal generated?" );
	}

	called = 1;
}

/* main function */
int main()
{
	int ret;
	long rts;

	struct sigaction sa;

	/* Initialize output */
	output_init();

	/* Test the RTS extension */
	rts = sysconf( _SC_REALTIME_SIGNALS );

	if ( rts < 0L )
	{
		UNTESTED( "This test needs the RTS extension" );
	}

	/* Set the signal handler */
	sa.sa_flags = SA_SIGINFO;

	sa.sa_sigaction = handler;

	ret = sigemptyset( &sa.sa_mask );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to empty signal set" );
	}

	/* Install the signal handler for SIGVTALRM */
	ret = sigaction( SIGNAL, &sa, 0 );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to set signal handler" );
	}

	if ( called )
	{
		FAILED( "The signal handler has been called when no signal was raised" );
	}

	ret = raise( SIGNAL );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to raise SIGVTALRM" );
	}

	if ( !called )
	{
		FAILED( "the sa_handler was not called whereas SA_SIGINFO was not set" );
	}


	/* Test passed */
#if VERBOSE > 0

	output( "Test passed\n" );

#endif

	PASSED;
}
