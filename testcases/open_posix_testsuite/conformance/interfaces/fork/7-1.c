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
* The child process gets a copy of the parent message catalog descriptor.


* The steps are:
* -> Create a message catalog file from the "messcat_src.txt" file
* -> Open this catalog
* -> fork
* -> Check that the child can read from the message catalog. 

* The test fails if the message catalog is read in the parent and not in the child.

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

#include <nl_types.h>

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

#define PATH_OFFSET "conformance/interfaces/fork/"

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

void read_catalog( nl_catd cat, char * who )
{
	char * msg = NULL;
	int i, j;
	errno = 0;

#if VERBOSE > 0

	output( "Reading the message catalog from %s...\n", who );
#endif

	for ( i = 1; i <= 2; i++ )
	{
		for ( j = 1; j <= 2; j++ )
		{
			msg = catgets( cat, i, j, "not found" );

			if ( errno != 0 )
			{
				UNRESOLVED( errno, "catgets returned an error" );
			}

#if VERBOSE > 1
			output( "set %i msg %i: %s\n", i, j, msg );

#endif

		}
	}

#if VERBOSE > 0
	output( "Message catalog read successfully in %s\n", who );

#endif
}

/* The main test function. */
int main( int argc, char * argv[] )
{
	int ret, status;
	pid_t child, ctl;

	nl_catd messcat;

	/* Initialize output */
	output_init();


	/* Generate the message catalog file from the text sourcefile */

	if ( system( NULL ) )
	{
		ret = system( "gencat mess.cat " PATH_OFFSET "messcat_src.txt" );

		if ( ret != 0 )
		{
			output( "Unable to find messcat_src.txt in standard directory %s\n", PATH_OFFSET );
			output( "Trying local dir\n" );
			ret = system( "gencat mess.cat messcat_src.txt" );

			if ( ret != 0 )
			{
				output( "Could not find the source file for message catalog.\n" \
				        "You may need to execute gencat yourself.\n" );
			}
		}
	}

	/* Try opening the message catalog file */
	messcat = catopen( "./mess.cat", 0 );

	if ( messcat == ( nl_catd ) - 1 )
	{
		UNRESOLVED( errno, "Could not open ./mess.cat. You may need to do a gencat before executing this testcase" );
	}

	/* Read the message catalog */
	read_catalog( messcat, "parent" );


	/* Create the child */
	child = fork();

	if ( child == ( pid_t ) - 1 )
	{
		UNRESOLVED( errno, "Failed to fork" );
	}

	/* child */
	if ( child == ( pid_t ) 0 )
	{
		read_catalog( messcat, "child" );

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

	/* We can now clean up the message catalog file */
	ret = catclose( messcat );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to close the message catalog" );
	}

	/* Try removing the message catalog file */
	system( "rm -f mess.cat" );

	/* Test passed */
#if VERBOSE > 0

	output( "Test passed\n" );

#endif

	PASSED;
}


