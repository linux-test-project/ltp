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
 * If attr is NULL, the effect is the same as passing the address 
 * of a default condition variable attributes object.

 * The steps are:
 * -> Create two cond vars, one with NULL attribute and 
 *    the other with a default attribute.
 * -> Compare those two cond vars:
 *    -> If the Thread Process-shared Synchronization is supported,
 *       does both condvars have the same behavior across 
 *       different process? (Beware of the spurious wakeups).
 *       (Steps to achieve this goal:
 *          - Have the two cond vars created in shared memory.
 *          - The associated mutex are process-shared and in shared memory also.
 *          - Fork
 *          - One thread in parent process and one thread in child process
 *            wait for the cond.
 *          - Broadcast the cond from the parent process; 
 *            then sleep for a while and check if the child process was awaken.
 *          - Do the same with the other condvar; then compare the result.
 *       )
 */
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* We need the XSI extention for the mkstemp() routine 
  * - we could rewrite this test without this routine...
  */
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
 #include <unistd.h>

 #include <errno.h>
 #include <sys/wait.h>
 #include <sys/mman.h>
 #include <semaphore.h>
 #include <string.h>
 #include <signal.h>
 
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

/* The data shared between both processes */
typedef struct
{
	pthread_mutex_t mtxN; /* Mutex for Null attr condvar */
	pthread_mutex_t mtxD; /* Mutex for Default attr condvar */
	pthread_mutex_t mtxP; /* Mutex for Pshared condvar */
	pthread_cond_t  cndN;
	pthread_cond_t  cndD;
	pthread_cond_t  cndP;
	int cntrN;
	int cntrD;
	int cntrP;
	char bool;
	sem_t semA;
	sem_t semB;
	int result;
	pid_t child;
} globaldata_t;

/* The data shared between the threads in the child process */
typedef struct
{
	pthread_mutex_t * pmtx;
	pthread_cond_t * pcnd;
	int * pcntr;
	char * pbool;
	sem_t * psem;
	pthread_t *pth;
} datatest_t;

/* The data shared with the signal handlers in the child process */
struct
{
	sem_t * psem;
	pthread_t th[3];
} sigdata;

/****
 * The child threads signal handler.
 * 
 */
void childsighdl(int sig)
{
	int ret=0;
	ret = sem_post(sigdata.psem);
	if (ret != 0)
	{  UNRESOLVED(errno, "[child] Unable to post semaphore in thread signal handler");  }
}

/****
 * The child main signal handler
 *
 */
void mainsighdl(int sig)
{
	/* We assume this signal handler is never called before the sigdata structure is initialized */
	int ret = 0;
	
	ret = pthread_kill(sigdata.th[0], SIGUSR2);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to kill a child thread");  }
	ret = pthread_kill(sigdata.th[1], SIGUSR2);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to kill a child thread");  }
	ret = pthread_kill(sigdata.th[2], SIGUSR2);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to kill a child thread");  }
}
 
/****
 * child_process_th 
 *  Child process's thread function
 *  
 */
void * child_process_th(void * arg)
{
	int ret=0;
	datatest_t * dt = (datatest_t *) arg;
	struct sigaction sa;
	
	/* Register into the signal data structure */
	*(dt->pth) = pthread_self();
	
	/* Set the signal action and mask for the child thread: Will call childsighdl when SIGUSR2 is received. */
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = childsighdl;
	ret = sigaction (SIGUSR2, &sa, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to register action for SIGUSR2");  }
	
	/* Any signal other than SIGUSR2 is blocked. */
	sigfillset(&sa.sa_mask);
	sigdelset(&sa.sa_mask, SIGUSR2);
	ret = pthread_sigmask (SIG_SETMASK, &sa.sa_mask, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to set signal mask in child thread");  }
	
	
	/* lock the mutex */
	ret = pthread_mutex_lock(dt->pmtx);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child][th] Unable to lock mutex.");  }
	
	/* We can now let the parent thread start its work */
	do { ret = sem_post(dt->psem); }
	while ((ret != 0) && (errno == EINTR));
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to post semaphore");  }
	
	/* Now, do the actual test: wait for the cond */
	do 
	{
		*(dt->pcntr) += 1;
		ret = pthread_cond_wait(dt->pcnd, dt->pmtx);
		*(dt->pcntr) += 1;
	} while ((ret == 0) && (*(dt->pbool) == 0));
	
	if (ret != 0)
	{
		#if VERBOSE > 1
		output("[child][thr] Unable to wait for the cond: %d - %s", ret, strerror(ret) );
		#endif
		*(dt->pcntr) = 0; /* this will signal the parent thread that an error happened */
	}
	
	/* unlock the mutex */
	ret = pthread_mutex_unlock(dt->pmtx);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child][th] Unable to unlock mutex.");  }
	
	return NULL;
}

/****
 * child_process
 *  Child process main thread
 *  This function is called in the child process just after the fork.
 */
int child_process(globaldata_t * gd)
{
	int ret;
	datatest_t dtN, dtD, dtP;
	pthread_t thN, thD, thP;
	
	struct sigaction sa;
	
	/* Initialize the datatest structures for the sub threads */
	dtN.pmtx = &(gd->mtxN);
	dtN.pcnd = &(gd->cndN);
	dtN.pcntr = &(gd->cntrN);
	dtN.pbool = &(gd->bool);
	dtN.psem  = &(gd->semA);
	dtN.pth = &(sigdata.th[0]);
	dtD.pmtx = &(gd->mtxD);
	dtD.pcnd = &(gd->cndD);
	dtD.pcntr = &(gd->cntrD);
	dtD.pbool = &(gd->bool);
	dtD.psem  = &(gd->semA);
	dtD.pth = &(sigdata.th[1]);
	dtP.pmtx = &(gd->mtxP);
	dtP.pcnd = &(gd->cndP);
	dtP.pcntr = &(gd->cntrP);
	dtP.pbool = &(gd->bool);
	dtP.psem  = &(gd->semA);
	dtP.pth = &(sigdata.th[2]);
	
	sigdata.psem = &(gd->semA);
	
	/* Register the signal handler: mainsighdl will be called when this thread receives SIGUSR1 */
	sigemptyset (&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR2);
	sa.sa_flags = 0;
	sa.sa_handler = mainsighdl;
	ret = sigaction (SIGUSR1, &sa, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to register action for SIGUSR1");  }
	
	/* Set this thread's signal mask: we only accept SIGUSR1 */
	sigfillset (&sa.sa_mask);
	sigdelset(&sa.sa_mask, SIGUSR1);
	ret = pthread_sigmask (SIG_SETMASK, &sa.sa_mask, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to set signal mask in child main thread");  }
	
	/* We start the test threads */
	ret = pthread_create(&thN, NULL, child_process_th, &dtN);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to create thread N");  }
	
	ret = pthread_create(&thD, NULL, child_process_th, &dtD);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to create thread D");  }

	ret = pthread_create(&thP, NULL, child_process_th, &dtP);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to create thread P");  }

	/* We wait for the parent process to let us signal the conditions */
	do { ret = sem_wait(&(gd->semB)); }
	while ((ret != 0) && (errno == EINTR));
	if (ret != 0)
	{  UNRESOLVED(errno, "[child] Unable to wait for semaphore B");  }
	
	/* Now signal both conditions. Changing the boolean was carried on by the parent */
	ret = pthread_cond_signal(&(gd->cndN));
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to signal cond N");  }
	
	ret = pthread_cond_signal(&(gd->cndD));
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to signal cond D");  }
	
	ret = pthread_cond_signal(&(gd->cndP));
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to signal cond P");  }
	
	/* Nothing more to do until the threads terminate */
	ret = pthread_join(thN, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to join the thread N");  }
	
	ret = pthread_join(thD, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to join the thread D");  }
	
	ret = pthread_join(thP, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "[child] Unable to join the thread P");  }
	
	return 0;
}

/****
 * parent_process
 *  This function is called in the parent process just after the fork.
 *  Don't call the UNRESOLVED macro here as it would orphan the child thread!
 *  Instead, the macro LOC_URSLVD(x, s) is provided. The usage is the same.
 */
#define LOC_URSLVD(x,s) \
{ \
	output("UNRESOLVED:\n Got error %d - %s\n with message %s\n", x, strerror(x), s);  \
	gd->bool = 1; \
	ret = sem_post(&(gd->semB)); \
	if (ret != 0) \
	{ \
		kill(gd->child, SIGKILL); \
		/* If the call fails, this probably means the process is already dead */ \
	} \
	return x; \
}

int parent_process(globaldata_t * gd)
{
	int ret, tmp, i;
	
	/* Wait for the child to be ready */
	ret = sem_wait(&(gd->semA));
	if (ret != 0)
	{  LOC_URSLVD(errno,"[parent] Unable to wait for sem A (1)");  }
	
	ret = sem_wait(&(gd->semA));
	if (ret != 0)
	{  LOC_URSLVD(errno,"[parent] Unable to wait for sem A (2)");  }
	
	ret = sem_wait(&(gd->semA));
	if (ret != 0)
	{  LOC_URSLVD(errno,"[parent] Unable to wait for sem A (3)");  }
	
	#if VERBOSE > 1
	output("[parent] Threads are ready...\n");
	#endif
	
	/* Now let the threads either enter the wait or exit if an error occured */
	ret = pthread_mutex_lock(&(gd->mtxN));
	if (ret != 0)
	{  LOC_URSLVD(ret,"[parent] Unable to lock mutex N");  }
	
	ret = pthread_mutex_lock(&(gd->mtxD));
	if (ret != 0)
	{  LOC_URSLVD(ret,"[parent] Unable to lock mutex D");  }
	
	ret = pthread_mutex_lock(&(gd->mtxP));
	if (ret != 0)
	{  LOC_URSLVD(ret,"[parent] Unable to lock mutex P");  }

	if (gd->cntrP == 0)
	{
		/* There was an unexpected error */
		LOC_URSLVD(0,"[parent] The pshared condvar reported an error");
	}
	/* Check the threads status */
	if ((gd->cntrN == 0) && (gd->cntrD == 0)) /* Both threads got an error */
	{
		/* the test has passed */
		gd->result = 0;
	}
	if ((gd->cntrN == 0) && (gd->cntrD != 0)) /* thread N got an error */
	{
		/* the test has failed */
		gd->result = gd->cntrD;
	}
	if ((gd->cntrN != 0) && (gd->cntrD == 0)) /* thread D got an error */
	{
		/* the test has failed */
		gd->result = gd->cntrN;
	}
	if ((gd->cntrN != 0) && (gd->cntrD != 0)) /* Neither thread got an error - we can test further */
	{
		#if VERBOSE > 1
		output("[parent] Both threads are waiting for the condition right now.\n");
		#endif
		
		/* We can unlock the mutexs so the threads can go out from pthread_cond_wait */
		ret = pthread_mutex_unlock(&(gd->mtxN));
		if (ret != 0)
		{  LOC_URSLVD(ret,"[parent] Unable to unlock mutex N");  }
		
		ret = pthread_mutex_unlock(&(gd->mtxD));
		if (ret != 0)
		{  LOC_URSLVD(ret,"[parent] Unable to unlock mutex D");  }
		
		ret = pthread_mutex_unlock(&(gd->mtxP));
		if (ret != 0)
		{  LOC_URSLVD(ret,"[parent] Unable to unlock mutex P");  }
		
		for (i=0; i<100; i++)
		{
			/* We try to signal the conditions */
			tmp = pthread_cond_signal(&(gd->cndN));
			ret = pthread_cond_signal(&(gd->cndD));
			if (ret != tmp)
			{  LOC_URSLVD(ret>tmp?ret:tmp, "[parent] Signaling the conditions give different error codes");  }
			#if VERBOSE > 1
			if (i==0)
			{
				output("[parent] Signaling the condition returned %d for both condvar\n", ret);
			}
			#endif
			ret = pthread_cond_signal(&(gd->cndP));
			if (ret != 0)
			{  LOC_URSLVD(ret, "[parent] Signaling the pshared condition failed");  }

			
			/* Make sure the child process's threads were scheduled */
			/* We kill it and wait for both threads to have the signal handled.
			  This will mean the threads were scheduled */
			ret = kill(gd->child, SIGUSR1);
			if (ret != 0)
			{  LOC_URSLVD(errno, "[parent] Killing child thread with USR1 failed");  }
		
			ret = sem_wait(&(gd->semA));
			if (ret != 0)
			{  LOC_URSLVD(errno,"[parent] Unable to wait for sem A (4)");  }
			
			ret = sem_wait(&(gd->semA));
			if (ret != 0)
			{  LOC_URSLVD(errno,"[parent] Unable to wait for sem A (5)");  }
			
			ret = sem_wait(&(gd->semA));
			if (ret != 0)
			{  LOC_URSLVD(errno,"[parent] Unable to wait for sem A (6)");  }
			
			sched_yield();
			
			/* We try to broadcast the conditions */
			tmp = pthread_cond_broadcast(&(gd->cndN));
			ret = pthread_cond_broadcast(&(gd->cndD));
			if (ret != tmp)
			{  LOC_URSLVD(ret>tmp?ret:tmp, "[parent] Broadcasting the conditions give different error codes");  }
			#if VERBOSE > 1
			if (i==0)
			{
				output("[parent] Broadcasting the condition returned %d for both condvar\n", tmp);
			}
			#endif
			ret = pthread_cond_broadcast(&(gd->cndP));
			if (ret != 0)
			{  LOC_URSLVD(ret, "[parent] Broadcasting the pshared conditions failed");  }
			
			/* Make sure the child process's threads were scheduled */
			/* We kill it and wait for both threads to have the signal handled.
			  This will mean the threads were scheduled */
			ret = kill(gd->child, SIGUSR1);
			if (ret != 0)
			{  LOC_URSLVD(errno, "[parent] Killing child thread with USR1 failed");  }
		
			ret = sem_wait(&(gd->semA));
			if (ret != 0)
			{  LOC_URSLVD(errno,"[parent] Unable to wait for sem A (7)");  }
			
			ret = sem_wait(&(gd->semA));
			if (ret != 0)
			{  LOC_URSLVD(errno,"[parent] Unable to wait for sem A (8)");  }

			ret = sem_wait(&(gd->semA));
			if (ret != 0)
			{  LOC_URSLVD(errno,"[parent] Unable to wait for sem A (9)");  }
			
			sched_yield();
		}

		
		/* Now we relock the mutexes */
		ret = pthread_mutex_lock(&(gd->mtxN));
		if (ret != 0)
		{  LOC_URSLVD(ret,"[parent] Unable to lock mutex N");  }
		
		ret = pthread_mutex_lock(&(gd->mtxD));
		if (ret != 0)
		{  LOC_URSLVD(ret,"[parent] Unable to lock mutex D");  }
		
		ret = pthread_mutex_lock(&(gd->mtxP));
		if (ret != 0)
		{  LOC_URSLVD(ret,"[parent] Unable to lock mutex P");  }
		
		/* We compare the counters values */
		if (gd->cntrP == 0)
		{  LOC_URSLVD(0, "[parent] The pshared condvar failed");  }
		
		if ((gd->cntrN == 0) && (gd->cntrD == 0))
		{
			#if VERBOSE > 1
			output("[parent] Both cond wait have failed\n");
			#endif
			gd->result = 0; /* The test has passed */
		}
		if ((gd->cntrN == 0) && (gd->cntrD != 0))
		{
			#if VERBOSE > 1
			output("[parent] cond wait has failed for N\n");
			#endif
			gd->result = gd->cntrD; /* The test has failed */
		}
		if ((gd->cntrN != 0) && (gd->cntrD == 0))
		{
			#if VERBOSE > 1
			output("[parent] cond wait has failed for D\n");
			#endif
			gd->result = gd->cntrN; /* The test has failed */
		}
		if ((gd->cntrN != 0) && (gd->cntrD != 0)) /* None of the condwait returned an error */
		{
			if (((gd->cntrN & 1) != 1) || ((gd->cntrD & 1) != 1))
			{
				output("N:%d D:%d\n",gd->cntrN,gd->cntrD);
				LOC_URSLVD((gd->cntrN & 1)?gd->cntrD:gd->cntrN,"[parent] Even counter - pshared mutex failure");
			}
			
			gd->result = 0;
			
			#if VERBOSE > 1
			output("[parent] Reports:\n");
			output("[parent]  Process-shared condvar was awaken : %i times (reference)\n", gd->cntrP >> 1);
			output("[parent]  Null attribute condvar was awaken : %i times\n", gd->cntrN >> 1);
			output("[parent]  Default attr.  condvar was awaken : %i times\n", gd->cntrD >> 1);
			#endif
			
			if ((gd->cntrN == gd->cntrP) && (gd->cntrD != gd->cntrP))
			{
				#if VERBOSE > 1
				output("[parent] Null condvar seems to be process-shared while Default condvar seems not\n");
				#endif
				gd->result = gd->cntrD;
			}
			if ((gd->cntrN != gd->cntrP) && (gd->cntrD == gd->cntrP))
			{
				#if VERBOSE > 1
				output("[parent] Default condvar seems to be process-shared while Null condvar seems not\n");
				#endif
				gd->result = gd->cntrN;
			}
			
		}
	}
	
	#if VERBOSE > 1
	output("[parent] Threads tests are finished, about to stop them...\n");
	#endif
	gd->bool = 1; /* the threads can now terminate */

	ret = pthread_mutex_unlock(&(gd->mtxN));
	if (ret != 0)
	{  LOC_URSLVD(ret,"[parent] Unable to unlock mutex N");  }
	
	ret = pthread_mutex_unlock(&(gd->mtxD));
	if (ret != 0)
	{  LOC_URSLVD(ret,"[parent] Unable to unlock mutex D");  }
			
	ret = pthread_mutex_unlock(&(gd->mtxP));
	if (ret != 0)
	{  LOC_URSLVD(ret,"[parent] Unable to unlock mutex P");  }
			
	/* Let the child thread terminate */
	ret = sem_post(&(gd->semB));
	if (ret != 0)
	{  LOC_URSLVD(errno, "[parent] Failed to post the semaphore B");  }
	
	return 0;
}

/****
 * do_tps_test
 *  This function will take care of testing the condvars
 *  when shared between processes.
 */
int do_tps_test(void)
{
	int ret=0;
	globaldata_t * gd;
	pthread_condattr_t ca;
	pthread_mutexattr_t ma;
	pid_t wrc, child;

	size_t sz;
	char filename[] = "/tmp/cond_init_1-3-XXXXXX";
	void * mmaped;
	char * tmp;
	int fd;
	
	int status;
	int rc2;

	
	/* We now create the temp files */
	fd = mkstemp(filename);
	if (fd == -1)
	{ UNRESOLVED(errno, "Temporary file could not be created"); }
	
	/* and make sure the file will be deleted when closed */
	unlink(filename);
	
	#if VERBOSE > 1
	output("Temp file created (%s).\n", filename);
	#endif
	
	sz= (size_t)sysconf(_SC_PAGESIZE);
	
	tmp = calloc(1, sz);
	if (tmp == NULL)
	{ UNRESOLVED(errno, "Memory allocation failed"); }
	
	/* Write the data to the file.  */
	if (write (fd, tmp, sz) != (ssize_t) sz)
	{ UNRESOLVED(sz, "Writting to the file failed"); }
	
	free(tmp);
	
	/* Now we can map the file in memory */
	mmaped = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (mmaped == MAP_FAILED)
	{ UNRESOLVED(errno, "mmap failed"); }
	
	gd = (globaldata_t *) mmaped;
	
	/* Our datatest structure is now in shared memory */
	#if VERBOSE > 1
	output("Shared memory created successfully.\n");
	output("Initializing data...\n");
	#endif
	
	/* Initialize the objects attributes */
	ret = pthread_mutexattr_init(&ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize mutex attribute object");  }
	
	ret = pthread_condattr_init(&ca);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize condvar attribute object");  }
	
	ret = pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to set the mutex attribute as process shared");  }
	
	/* Initialize the synchronization objects */
	ret = pthread_mutex_init(&(gd->mtxN), &ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize the mutex N in shared memory");  }
	
	ret = pthread_mutex_init(&(gd->mtxD), &ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize the mutex D in shared memory");  }
	
	ret = pthread_mutex_init(&(gd->mtxP), &ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize the mutex P in shared memory");  }
	
	/* we don't need the mutex attribute object anymore */
	ret = pthread_mutexattr_destroy(&ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to destroy the mutex attribute object");  }
	
	/* Now we'll initialize the cond vars */
	ret = pthread_cond_init(&(gd->cndN), NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize the Null attribute condvar");  }
	
	ret = pthread_cond_init(&(gd->cndD), &ca);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize the Default attribute condvar");  }
	
	ret = pthread_condattr_setpshared(&ca, PTHREAD_PROCESS_SHARED);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to set the cond attribute as process shared");  }
	
	ret = pthread_cond_init(&(gd->cndP), &ca);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize the Pshared condvar");  }
	
	ret = pthread_condattr_destroy(&ca);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to destroy the condvar attribute object");  }
	
	/* We initialize the other values in the test structure */
	gd->cntrN = 0;
	gd->cntrD = 0;
	gd->cntrP = 0;
	gd->bool = 0;
	gd->result = 0;
	ret = sem_init(&(gd->semA), 1, 0);
	if (ret != 0)
	{  UNRESOLVED(errno, "Unable to initialize semaphore A");  }
	
	ret = sem_init(&(gd->semB), 1, 0);
	if (ret != 0)
	{  UNRESOLVED(errno, "Unable to initialize semaphore B");  }
	
	/* Initializations are OK */
	#if VERBOSE > 1
	output("All initializations OK, proceed to the test (fork).\n");
	#endif
	
	child = fork();
	if (child == -1)
	{  UNRESOLVED(errno, "Fork failed");  }
	
	if (child == 0)
	{
		/* We are the child */
		ret = child_process(gd);
		#if VERBOSE > 1
		output("[child] Test function returned %d.\n", ret);
		#endif
		exit(ret);
	}
	
	/* We are the parent */
	gd->child = child;
	rc2 = parent_process(gd);
	#if VERBOSE > 1
	output("[parent] Test function returned %d.\n", rc2);
	#endif
	
	/* In any case we must wait for the child */
	wrc = waitpid(child, &status, 0);
	if (wrc != child)
	{
		output("Expected pid: %i. Got %i\n", (int)child, (int)wrc);
		UNRESOLVED(errno, "Waitpid failed"); 
	}
	
	if (WIFSIGNALED(status))
	{ 
		output("Child process killed with signal %d\n",WTERMSIG(status)); 
		UNRESOLVED( rc2 , "Child process was killed"); 
	}
	
	if (WIFEXITED(status))
	{
		ret = WEXITSTATUS(status);
	}
	else
	{
		UNRESOLVED( rc2, "Child process was neither killed nor exited");
	}
	
	#if VERBOSE > 1
	output("[parent] Successfully waited for child process.\n");
	#endif
	
	/* The return value from the parent is in 'rc2' and 
	 * 'ret' contains the child return code.
	 * The test status is in gd->result
	 */
	if (rc2 != 0)
	{
		UNRESOLVED(ret, "Parent process failed");
	}
	
	if (ret != 0)
	{
		UNRESOLVED(ret, "Child process failed"); 
	}
	
	#if VERBOSE > 1
	output("Destroying the data.\n");
	#endif
	
	/* We can now destroy all the datas */
	ret = sem_destroy(&(gd->semB));
	if (ret != 0)
	{  UNRESOLVED(errno, "Unable to destroy the semaphore B");  }
	
	ret = sem_destroy(&(gd->semA));
	if (ret != 0)
	{  UNRESOLVED(errno, "Unable to destroy the semaphore A");  }
	
	ret = pthread_cond_destroy(&(gd->cndP));
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to destroy the P condvar");  }

	ret = pthread_cond_destroy(&(gd->cndD));
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to destroy the D condvar");  }
	
	ret = pthread_cond_destroy(&(gd->cndN));
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to destroy the N condvar");  }
	
	ret = pthread_mutex_destroy(&(gd->mtxP));
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to destroy the P mutex");  }
	
	ret = pthread_mutex_destroy(&(gd->mtxD));
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to destroy the D mutex");  }
	
	ret = pthread_mutex_destroy(&(gd->mtxN));
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to destroy the N mutex");  }
	
	rc2 = gd->result;

	#if VERBOSE > 1
	output("Unmapping shared memory.\n");
	#endif
	
	ret = munmap(mmaped, sz);
	if (ret != 0)
	{  UNRESOLVED(errno, "Memory unmapping failed");  }
	
	return rc2;
}

/****
 * Main function
 *  This one is responsible for executing the previous functions 
 *  according to the supported features.
 */
int main(int argc, char * argv[])
{
	long opt_TPS, opt_MF;
	int ret=0;

	output_init();
	
	#if VERBOSE > 1
	output("Test starting...\n");
	#endif

	opt_MF =sysconf(_SC_MAPPED_FILES);
	opt_TPS=sysconf(_SC_THREAD_PROCESS_SHARED);
	
	#if VERBOSE > 1
	output("Memory Mapped Files option : %li\n", opt_MF);
	output("Thread Process-shared Synchronization option: %li\n", opt_TPS);
	#endif
	
	if ((opt_TPS != -1L) && (opt_MF != -1L))
	{
		#if VERBOSE > 0
		output("Starting process test\n");
		#endif
		ret = do_tps_test();
	}
	else
	{
		UNTESTED("This test requires unsupported features");
	}

	if (ret != 0)
	{  FAILED("The cond vars behave differently across processes.");  }

	#if VERBOSE > 1
	output("Test terminated successfully\n");
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

