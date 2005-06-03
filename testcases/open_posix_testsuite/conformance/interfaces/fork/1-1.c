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
 * fork() creates a new process.
 
 * The steps are:
 * -> create a new process
 * -> the parent and the child sleep 1 sec (check concurrent execution)
 * -> the child posts a semaphore, the parents waits for thsi semaphore
 *    (check the child really executes)
 * -> join and check that total execution time is < 2 sec.
 
 * The test fails if the duration is > 2 seconds or if semaphore is not posted.
 
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
 #include <time.h>
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

#define SEM_NAME "/semfork1_1"

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

/* The main test function. */
int main(int argc, char * argv[])
{
	int ret, status;
	pid_t child, ctl;
	sem_t *sem;
	struct timespec tsini, tsfin;
	
	/* Initialize output */
	output_init();
	
	/* read current time */
	ret = clock_gettime(CLOCK_REALTIME, &tsini);
	if (ret != 0)  {  UNRESOLVED(errno, "Unable to read CLOCK_REALTIME clock");  }
	
	/* Set temporary value in tsfin for semaphore timeout */
	tsfin.tv_sec  = tsini.tv_sec + 3;
	tsfin.tv_nsec = tsini.tv_nsec;
	
	/* Create the child */
	child = fork();
	if (child == (pid_t) -1)  {  UNRESOLVED(errno, "Failed to fork");  }
	
	/* Open the semaphore */
	sem = sem_open(SEM_NAME, O_CREAT, O_RDWR, 0);
	if (sem == (sem_t *)SEM_FAILED)  {  UNRESOLVED(errno, "Failed to open the semaphore");  }
	
	/* sleep 1 second */
	sleep(1);
	
	/* child posts the semaphore and terminates */
	if (child == (pid_t) 0)
	{
		do { ret = sem_post(sem); }
		while ((ret == -1) && (errno == EINTR));
		if (ret != 0)  {  UNRESOLVED(errno, "Failed to post the semaphore");  }
		
		ret = sem_close(sem);
		if (ret != 0)  {  UNRESOLVED(errno, "Failed to close the semaphore");  }
		
		/* The child stops here */
		exit(0);
	}
	
	/* Parent waits for the semaphore */
	do { ret = sem_timedwait(sem, &tsfin); }
	while ((ret != 0) && (errno == EINTR));
	if (ret != 0)  
	{
		if (errno == ETIMEDOUT) { FAILED("The new process does not execute"); }
		UNRESOLVED(errno, "Failed to wait for the semaphore");  
	}
	
	/* We don't need the semaphore anymore */
	ret = sem_unlink(SEM_NAME);
	if (ret != 0)  {  UNRESOLVED(errno, "Unable to unlink the semaphore");  }
	ret = sem_close(sem);
	if (ret != 0)  {  UNRESOLVED(errno, "Failed to close the semaphore");  }
	
	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);
	if (ctl != child)  {  UNRESOLVED(errno, "Waitpid returned the wrong PID");  }
	if ((!WIFEXITED(status)) || (WEXITSTATUS(status) != 0))
	{
		UNRESOLVED(status, "Child exited abnormally");
	}
	
	/* Check the global duration */
	ret = clock_gettime(CLOCK_REALTIME, &tsfin);
	if (ret != 0)  {  UNRESOLVED(errno, "Unable to read CLOCK_REALTIME clock");  }
	
	if (tsfin.tv_nsec < tsini.tv_nsec)
		tsfin.tv_sec -= 1;
	
	status = tsfin.tv_sec - tsini.tv_sec;
	if (status >= 2)
	{
		/* the operation was more than 2 secs long */
		FAILED("the processes did not execute concurrently");
	}
	
	/* Test passed */
	#if VERBOSE > 0
	output("Test passed\n");
	#endif

	PASSED;
}


