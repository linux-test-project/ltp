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
 *
 
 
 * This sample test aims to check the following assertion:
 * The function does not return an error code of EINTR 


 * The steps are:
 * 
 * -> Create a thread which loops on pthread_cond_init and pthread_cond_destroy
 *      operations.
 * -> Create another thread which loops on sending a signal to the first thread.
 * 
 * 
 */

 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <semaphore.h>
 #include <errno.h>
 #include <signal.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdarg.h>
 #include <stdlib.h>
 
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
#define WITH_SYNCHRO
#ifndef VERBOSE
#define VERBOSE 2
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
char do_it=1;
unsigned long count_ope=0;
pthread_mutex_t count_protect = PTHREAD_MUTEX_INITIALIZER;
#ifdef WITH_SYNCHRO
sem_t semsig1;
sem_t semsig2;
unsigned long count_sig=0;
#endif
sem_t semsync;

typedef struct 
{
	pthread_t   	*thr;
	int	sig;
#ifdef WITH_SYNCHRO
	sem_t	*sem;
#endif
} thestruct;

/* the following function keeps on sending the signal to the thread pointed by arg
 *  If WITH_SYNCHRO is defined, the target thread has a handler for the signal */
void * sendsig (void * arg)
{
	thestruct *thearg = (thestruct *) arg;
	int ret;
	while (do_it)
	{
		#ifdef WITH_SYNCHRO
		if ((ret = sem_wait(thearg->sem)))
		{ UNRESOLVED(errno, "Sem_wait in sendsig"); }
		count_sig++;
		#endif

		if ((ret = pthread_kill (*(thearg->thr), thearg->sig)))
		{ UNRESOLVED(ret, "Pthread_kill in sendsig"); }
		
	}
	
	return NULL;
}

/* Next are the signal handlers. */
void sighdl1(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig1))
	{ UNRESOLVED(errno, "Sem_post in signal handler 1"); }
#endif
}
void sighdl2(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig2))
	{ UNRESOLVED(errno, "Sem_post in signal handler 2"); }
#endif	
}

/* The following function loops on init/destroy some condvars (with different attributes)
 * it does check that no error code of EINTR is returned */

void * threaded(void * arg)
{
	pthread_condattr_t ca[4], *pca[5];
	pthread_cond_t c[5];
	int i;
	int ret;
	
	/* We need to register the signal handlers */
	struct sigaction sa;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighdl1;
	if ((ret = sigaction (SIGUSR1, &sa, NULL)))
	{ UNRESOLVED(ret, "Unable to register signal handler1"); }
	sa.sa_handler = sighdl2;
	if ((ret = sigaction (SIGUSR2, &sa, NULL)))
	{ UNRESOLVED(ret, "Unable to register signal handler2"); }
 	
	/* Initialize the different cond attributes */
	pca[4]=NULL;
	
	for (i=0; i<4; i++)
	{
		pca[i]=&ca[i];
		if ((ret = pthread_condattr_init(pca[i])))
		{ UNRESOLVED(ret, "pthread_condattr_init"); }
	}
	
		ret = pthread_condattr_setpshared(pca[0], PTHREAD_PROCESS_SHARED);
		if (ret != 0) {  UNRESOLVED(ret, "Cond attribute PSHARED failed");  }
		ret = pthread_condattr_setpshared(pca[1], PTHREAD_PROCESS_SHARED);
		if (ret != 0) {  UNRESOLVED(ret, "Cond attribute PSHARED failed");  }
		
		#if VERBOSE >1
		output("PShared condvar attributes initialized\n");
		#endif
		if (sysconf(_SC_MONOTONIC_CLOCK) > 0)
		{
			ret = pthread_condattr_setclock(pca[1], CLOCK_MONOTONIC);
			if (ret != 0) {  UNRESOLVED(ret, "Cond set monotonic clock failed");  }
			ret = pthread_condattr_setclock(pca[2], CLOCK_MONOTONIC);
			if (ret != 0) {  UNRESOLVED(ret, "Cond set monotonic clock failed");  }
			#if VERBOSE >1
			output("Alternative clock condvar attributes initialized\n");
			#endif
		}
	
	/*	We are ready to proceed */
	while (do_it)
	{
		for (i=0; i<5; i++)
		{
			ret = pthread_cond_init(&c[i], pca[i]);
			if (ret == EINTR)
			{
				FAILED("pthread_cond_init returned EINTR");
			}
			if (ret != 0)
			{
				UNRESOLVED(ret, "pthread_cond_init failed");
			}
			ret = pthread_cond_destroy(&c[i]);
			if (ret == EINTR)
			{
				FAILED("pthread_cond_destroy returned EINTR");
			}
			if (ret != 0)
			{
				UNRESOLVED(ret, "pthread_cond_destroy failed");
			}
			pthread_mutex_lock(&count_protect);
			count_ope++;
			pthread_mutex_unlock(&count_protect);
		}
	}

	/* Now we can destroy the mutex attributes objects */
	for (i=0; i<4; i++)
	{
		if ((ret = pthread_condattr_destroy(pca[i])))
		{ UNRESOLVED(ret, "pthread_condattr_init"); }
	}
	
	do {
		ret = sem_wait(&semsync);
	} while (ret && (errno ==  EINTR));
	if (ret)
	{ UNRESOLVED(errno, "Could not wait for sig senders termination"); }
	
	return NULL;
}

/* Main function */
int main (int argc, char * argv[])
{
	int ret;
	pthread_t th_work, th_sig1, th_sig2;
	thestruct arg1, arg2;
	
	output_init();

	#ifdef WITH_SYNCHRO
	if (sem_init(&semsig1, 0, 1))
	{ UNRESOLVED(errno, "Semsig1  init"); }
	if (sem_init(&semsig2, 0, 1))
	{ UNRESOLVED(errno, "Semsig2  init"); }
	#endif
	
	if (sem_init(&semsync, 0, 0))
	{ UNRESOLVED(errno, "semsync init"); }
	
	if ((ret = pthread_create(&th_work, NULL, threaded, NULL)))
	{ UNRESOLVED(ret, "Worker thread creation failed"); }
	
	arg1.thr = &th_work;
	arg2.thr = &th_work;
	arg1.sig = SIGUSR1;
	arg2.sig = SIGUSR2;
#ifdef WITH_SYNCHRO
	arg1.sem = &semsig1;
	arg2.sem = &semsig2;
#endif
	
	if ((ret = pthread_create(&th_sig1, NULL, sendsig, (void *)&arg1)))
	{ UNRESOLVED(ret, "Signal 1 sender thread creation failed"); }
	if ((ret = pthread_create(&th_sig2, NULL, sendsig, (void *)&arg2)))
	{ UNRESOLVED(ret, "Signal 2 sender thread creation failed"); }
	
	/* Let's wait for a while now */
	sleep(1);
	
	/* Now stop the threads and join them */
	do { do_it=0; }
	while (do_it);
	
	if ((ret = pthread_join(th_sig1, NULL)))
	{ UNRESOLVED(ret, "Signal 1 sender thread join failed"); }
	if ((ret = pthread_join(th_sig2, NULL)))
	{ UNRESOLVED(ret, "Signal 2 sender thread join failed"); }
	
	if (sem_post(&semsync))
	{ UNRESOLVED(errno, "could not post semsync"); }
	
	if ((ret = pthread_join(th_work, NULL)))
	{ UNRESOLVED(ret, "Worker thread join failed"); }

	#if VERBOSE > 0
	output("Test executed successfully.\n");
	output("  %d condvars initialization and destruction were done.\n", count_ope);
	#ifdef WITH_SYNCHRO
	output("  %d signals were sent meanwhile.\n", count_sig);
	#endif 
	#endif	
	PASSED;
}
