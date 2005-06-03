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
 * getpid() always returns the process ID of the calling thread/process. 
 
 * The steps are:
 *
 * -> create two threads and check they get the same getpid() return value.
 * -> create two processes and check they get different getpid() return value.
 * -> check that the child process getpid() return value matchs the fork() return 
      value in the parent process.
 
 * The test fails if any of the previous checks is not verified.
 
 */
 
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* We need the XSI extention for the mutex attributes
   and the mkstemp() routine */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
#endif

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <stdarg.h>
 #include <stdio.h>
 #include <stdlib.h> 
 #include <string.h>
 #include <unistd.h>

 #include <errno.h>
 #include <sys/wait.h>
 #include <sys/mman.h>

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
#ifndef WITHOUT_XOPEN

pid_t * sharedpid;

/* This will be executed by the child process */
void child(void)
{
	*sharedpid=getpid();
	exit(0);
}

/* This will be executed by the child thread */
void * threaded(void * arg)
{
	*(pid_t *)arg = getpid();
	return NULL;
}

/* The main test function. */
int main(int argc, char * argv[])
{
	int ret, status;
	long mf; /* Is memory mapping supported? */
	pid_t mypid, hispid, ctlpid;
	pthread_t child_thread;
	
	/* Initialize output */
	output_init();
	
	/* Get self PID */
	mypid=getpid();
	#if VERBOSE > 1
	output("Main pid: %d\n", mypid);
	#endif
	
	/* Get a child thread PID */
	ret = pthread_create(&child_thread, NULL, threaded, &hispid);
	if (ret != 0)  {  UNRESOLVED(ret, "Thread creation failed");  }
	ret = pthread_join(child_thread, NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "Thread join failed");  }
	
	#if VERBOSE > 1
	output("Thread pid: %d\n", hispid);
	#endif

	/* Compare threads PIDs */
	if (mypid != hispid)
	{
		FAILED("Child thread got a different return value from getpid()\n");
	}
	
	/* Test system abilities */
	mf =sysconf(_SC_MAPPED_FILES);
	
	#if VERBOSE > 0
	output("Test starting\n");
	output("System abilities:\n");
	output(" MF  : %li\n", mf);
	if (mf <= 0)
		output("Unable to test without shared data\n");
	#endif
	
	/* We need MF support for the process-cross testing */
	if (mf > 0)
	{
		/* We will place the child pid in a mmaped file */
		char filename[] = "/tmp/getpid-1-XXXXXX";
		void * mmaped;
		int fd;
		
		/* We now create the temp files */
		fd = mkstemp(filename);
		if (fd == -1)
		{ UNRESOLVED(errno, "Temporary file could not be created"); }
		
		/* and make sure the file will be deleted when closed */
		unlink(filename);
		
		#if VERBOSE > 1
		output("Temp file created (%s).\n", filename);
		#endif
		
		/* Fill the file up to 1 pagesize */
		ret = ftruncate(fd, sysconf(_SC_PAGESIZE));
		if (ret != 0)  {  UNRESOLVED(errno, "ftruncate operation failed");  }
		
		/* Now we can map the file in memory */
		mmaped = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (mmaped == MAP_FAILED)
		{ UNRESOLVED(errno, "mmap failed"); }
		
		/* Set the sharedpid pointer to this mmaped area */
		sharedpid = (pid_t *) mmaped;
		
		/* Our data is now in shared memory */
		#if VERBOSE > 1
		output("Shared memory is ready.\n");
		#endif
		
		/* Okay, let's create the child process */
		hispid=fork();
		if (hispid == (pid_t)-1) {  UNRESOLVED(errno, "Fork failed");  }
		
		/* Child process : */
		if (hispid == (pid_t)0)
			child();
		
		/* Otherwise, we're the parent */
		ctlpid = waitpid(hispid, &status, 0);
		if (ctlpid != hispid)  {  UNRESOLVED(errno, "waitpid waited for the wrong process");  }
		if (!WIFEXITED(status) || WEXITSTATUS(status))  
		{  UNRESOLVED(status, "The child process did not terminate as expected");  }

		#if VERBOSE > 1
		output("Child process pid: %d\n", hispid);
		#endif
		
		/* Check the child pid is the same as fork returned */
		if (hispid != *sharedpid)
		{  FAILED("getpid() in the child returned a different value than fork() in the parent");  }
		
		/* Check the child pid is different than the parent pid */
		if (hispid == mypid)
		{  FAILED("Both child and parent getpid() return values are equal");  }
	}
	
	#if VERBOSE > 0
	output("Test passed\n");
	#endif

	PASSED;
}

#else /* WITHOUT_XOPEN */
int main(int argc, char * argv[])
{
	output_init();
	UNTESTED("This test requires XSI features");
}
#endif
