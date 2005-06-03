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
 * pthread_create creates a thread with attributes as specified in the attr parameter.
 
 * The steps are:
 *
 * -> See test 1-5.c for details
 * -> This one will test the scheduling behavior is correct.
 
 */
 
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* Some routines are part of the XSI Extensions */
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

 #include <sched.h>
 #include <semaphore.h>
 #include <errno.h>
 #include <assert.h>
 #include <sys/wait.h>
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

/* The value below shall be >= to the # of CPU on the test architecture */
#define NCPU 	(4)

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/
#define STD_MAIN /* This allows main() to be defined in the included file */

#include "threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/


/* The 2 following functions are used for the scheduling tests */
void * lp_func(void * arg)
{
	int * ctrl = (int *) arg;
	*ctrl=2;
	return NULL;
}

void * hp_func(void * arg)
{
	int *ctrl = (int *) arg;
	int dummy=0, i;
	do
	{
		/* some dummy task */
		dummy += 3;
		dummy %= 17;
		dummy *= 47;
		for (i=0; i<1000000000; i++);
		#if VERBOSE > 6
		output("%p\n", pthread_self());
		#endif
	}
	while (*ctrl == 0);
	return NULL;
}


void * threaded (void * arg)
{
	int ret = 0;
	
	#if VERBOSE > 4
	output("Thread %i starting...\n", sc);
	#endif
	
   /* Scheduling (priority) tests */
	if ((sysconf(_SC_THREAD_PRIORITY_SCHEDULING) > 0) 
		&& (scenarii[sc].explicitsched != 0) 
		&& (scenarii[sc].schedpolicy != 0)
		&& (scenarii[sc].schedparam == 1))
	{
		/* We will create NCPU threads running with a high priority with the same sched policy policy
		 and one with a low-priority.
		  The low-priority thread should not run until the other threads stop running,
		 unless the machine has more than NCPU processors... */
		
		pthread_t hpth[NCPU]; 	/* High priority threads */
		pthread_t lpth; 	/* Low Priority thread */
		int ctrl;		/* Check value */
		pthread_attr_t ta;
		struct sched_param sp;
		int policy;
		int i=0;
		struct timespec now, timeout;
		
		/* Start with checking we are executing with the required parameters */
		ret = pthread_getschedparam(pthread_self(), &policy, &sp);
		if (ret != 0)  {  UNRESOLVED(ret , "Failed to get current thread policy");  }
		
		if (((scenarii[sc].schedpolicy == 1) && (policy != SCHED_FIFO))
		   || ((scenarii[sc].schedpolicy == 2) && (policy != SCHED_RR)))
		{
			FAILED("The thread is not using the scheduling policy that was required");
		}
		if (((scenarii[sc].schedparam == 1) && (sp.sched_priority != sched_get_priority_max(policy)))
		   || ((scenarii[sc].schedparam ==-1) && (sp.sched_priority != sched_get_priority_min(policy))))
		{
			
			FAILED("The thread is not using the scheduling parameter that was required");
		}
		
		ctrl = 0; /* Initial state */
		
		/* Get the policy information */
		ret = pthread_attr_getschedpolicy(&scenarii[sc].ta, &policy);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to read sched policy");  }
		
		/* We put a timeout cause the test might lock the machine when it runs */
		alarm(60);
		
		/* Create the high priority threads */
		ret = pthread_attr_init(&ta);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to initialize a thread attribute object");  }
		
		ret = pthread_attr_setinheritsched(&ta, PTHREAD_EXPLICIT_SCHED);
		if (ret != 0)  {  UNRESOLVED(ret, "Unable to set inheritsched attribute");  }
		
		ret = pthread_attr_setschedpolicy(&ta, policy);
		if (ret != 0)  {  UNRESOLVED(ret, "Unable to set the sched policy");  }
		
		sp.sched_priority = sched_get_priority_max(policy) - 1;
		
		ret = pthread_attr_setschedparam(&ta, &sp);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to set the sched param");  }
		
		#if VERBOSE > 1
		output("Starting %i high- and 1 low-priority threads.\n", NCPU);
		#endif
		for (i=0; i<NCPU; i++)
		{
			ret = pthread_create(&hpth[i], &ta, hp_func, &ctrl);
			if (ret != 0)  {  UNRESOLVED(ret, "Failed to create enough threads");  }
		}
		#if VERBOSE > 5
		output("The %i high-priority threads are running\n", NCPU);
		#endif
		
		/* Create the low-priority thread */
		sp.sched_priority = sched_get_priority_min(policy);
		
		ret = pthread_attr_setschedparam(&ta, &sp);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to set the sched param");  }
		
		ret = pthread_create(&lpth, &ta, lp_func, &ctrl);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to create enough threads");  }
		
		/* Keep going */
		ret = clock_gettime(CLOCK_REALTIME, &now);
		if (ret != 0)  {  UNRESOLVED(errno, "Failed to read current time");  }
		
		timeout.tv_sec = now.tv_sec;
		timeout.tv_nsec = now.tv_nsec + 500000000;
		while (timeout.tv_nsec >= 1000000000)
		{
			timeout.tv_sec++;
			timeout.tv_nsec -= 1000000000;
		}
		
		do
		{
			if (ctrl != 0)
			{
				output("The low priority thread executed. This might be normal if you have more than %i CPU.\n", NCPU + 1);
				FAILED("Low priority thread executed -- the sched parameters are ignored?");
			}
			ret = clock_gettime(CLOCK_REALTIME, &now);
			if (ret != 0)  {  UNRESOLVED(errno, "Failed to read current time");  }
			#if VERBOSE > 5
			output("Time: %d.%09d (to: %d.%09d)\n", now.tv_sec, now.tv_nsec, timeout.tv_sec, timeout.tv_nsec);
			#endif
		}
		while ((now.tv_sec <= timeout.tv_sec) && (now.tv_nsec <= timeout.tv_nsec));
		
		/* Ok the low priority thread did not execute :) */
		
		/* tell the other high priority to terminate */
		ctrl = 1;  
		
		for (i=0; i<NCPU; i++)
		{
			ret = pthread_join(hpth[i], NULL);
			if (ret != 0)  {  UNRESOLVED(ret, "Failed to join a thread");  }
		}
		
		/* Ok so now the low priority should execute when we stop this one (or earlier). */
		ret = pthread_join(lpth, NULL);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to join the low priority thread");  }
		
		/* We just check that it executed */
		if (ctrl != 2)  {  FAILED("Joined the low-priority thread but it did not execute.");  }
		
		#if VERBOSE > 1
		output("The scheduling parameter was set accordingly to the thread attribute.\n");
		#endif
		
		/* We're done. */
		ret = pthread_attr_destroy(&ta);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy a thread attribute object");  }
	}
	
	/* Post the semaphore to unlock the main thread in case of a detached thread */
	do { ret = sem_post(&scenarii[sc].sem); }
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1)  {  UNRESOLVED(errno, "Failed to post the semaphore");  }
	
	return arg;
}

