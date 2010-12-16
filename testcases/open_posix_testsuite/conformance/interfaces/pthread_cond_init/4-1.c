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
 * The function fails and returns ENOMEM if there is not enough memory.

 * The steps are:
 * * Fork
 * * New process sets its memory resource limit to a minimum value, then
 *  -> Allocate all the available memory
 *  -> call pthread_cond_init()
 *  -> free the memory
 *  -> Checks that pthread_cond_init() returned 0 or ENOMEM.
 * * Parent process waits for the child.
 */

 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L

 /* We need the setrlimit() function from X/OPEN standard */
 #ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <errno.h>
 #include <signal.h>
 #include <sys/wait.h>
 #include <sys/resource.h>
 #include <stdarg.h>

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

int main(int argc, char * argv[])
{
	pid_t child;

	pthread_cond_t  cnd;
	pthread_condattr_t ca[4];
	pthread_condattr_t *pca[5];

	int ret=0;
	int i;
	int retini[5] = {-1,-1,-1,-1,-1};
	int retdtr[5]= {-1,-1,-1,-1,-1};

	void * ptr, *ptr_prev=NULL;

	int sz = 0;
	struct rlimit rl;

	int status=0;

	output_init();

	child = fork();

	if (child == (pid_t)-1)
	{ UNRESOLVED(errno, "Fork failed"); }

	if (child != 0) /* We are the father */
	{
		if (child != waitpid(child, &status, 0))
		{  UNRESOLVED(errno, "Waitpid failed"); }

		if (WIFSIGNALED(status))
		{ UNRESOLVED(WTERMSIG(status),
			"The child process was killed."); }

		if (WIFEXITED(status))
			return WEXITSTATUS(status);

		UNRESOLVED(0, "Child process neither returned nor was killed.");
	}

	/* Only the child goes further */

	/* We initialize the different cond attributes */
	for (i=0; (i<4) && (ret == 0); i++)
	{
		pca[i] = &ca[i];
		ret = pthread_condattr_init(pca[i]);
	}
	if (ret)
	{ UNRESOLVED(ret, "Cond attribute init failed"); }
	pca[4] = (pthread_condattr_t *) NULL;

	ret = pthread_condattr_setpshared(pca[0], PTHREAD_PROCESS_SHARED);
	if (ret != 0) {  UNRESOLVED(ret, "Cond attribute PSHARED failed");  }
	ret = pthread_condattr_setpshared(pca[1], PTHREAD_PROCESS_SHARED);
	if (ret != 0) {  UNRESOLVED(ret, "Cond attribute PSHARED failed");  }

	if (sysconf(_SC_MONOTONIC_CLOCK) > 0)
	{
		ret = pthread_condattr_setclock(pca[1], CLOCK_MONOTONIC);
		if (ret != 0) {  UNRESOLVED(ret, "Cond set monotonic clock failed");  }
		ret = pthread_condattr_setclock(pca[2], CLOCK_MONOTONIC);
		if (ret != 0) {  UNRESOLVED(ret, "Cond set monotonic clock failed");  }
	}

	sz = sysconf(_SC_PAGESIZE);

	/* Limit the process memory to a small value (64Mb for example). */
	rl.rlim_max=1024*1024*64;
	rl.rlim_cur=1024*1024*64;
	if ((ret = setrlimit(RLIMIT_AS,  &rl)))
	{ UNRESOLVED(ret, "Memory limitation failed"); }

	#if VERBOSE > 1
	output("Ready to take over memory. Page size is %d\n", sz);
	#endif

	/* Allocate all available memory */
	while (1)
	{
		ptr = malloc(sz); /* Allocate one page of memory */
		if (ptr == NULL)
			break;
		#if VERBOSE > 1
		ret++;
		#endif
		*(void **)ptr = ptr_prev; /* Write into the allocated page */
		ptr_prev = ptr;
	}
	#if VERBOSE > 1
	output("%d pages were allocated before failure\n", ret);
	ret = 0;
	#endif

	while (1)
	{
		ptr = malloc(sizeof(void*)); /* Allocate every remaining bits of memory */
		if (ptr == NULL)
			break;
		#if VERBOSE > 1
		ret++;
		#endif
		*(void **)ptr = ptr_prev; /* Keep track of allocated memory */
		ptr_prev = ptr;
	}
	#if VERBOSE > 1
	output("%d additional spaces were allocated before failure\n", ret);
	ret = 0;
	#endif
	if (errno != ENOMEM)
		UNRESOLVED(errno, "Memory not full");

	/* Now that memory is full, we try to initialize a cond */
	for (i=0; i<5; i++)
	{
		retini[i] = pthread_cond_init(&cnd, pca[i]);
		if (!retini[i]) /* If cond has been initialized, we destroy it */
			retdtr[i] = pthread_cond_destroy(&cnd);
	}

	/* We can now free the memory */
	while (ptr_prev != NULL)
	{
		ptr = ptr_prev;
		ptr_prev = *(void **)ptr;
		free(ptr);
	}

	#if VERBOSE > 1
	output("Memory is released\n");
	#endif

	for (i=0; i<4; i++)
		pthread_condattr_destroy(pca[i]);

	for (i=0; i<5; i++)
	{
		if (retini[i] != 0 && retini[i] !=ENOMEM)
		{  FAILED("Cond init returned a wrong error code when no memory was left"); }

		if (retini[i] == 0)
		{
			#if VERBOSE > 0
			output("Cond (%i) initialization succeeded when memory is full\n", i);
			#endif
			if (retdtr[i] != 0)
			{  UNRESOLVED(retdtr[i],  "Cond destroy failed for a cond inilialized under full memory"); }
		}
		#if VERBOSE > 0
		else
		{
			output("Cond (%i) initialization failed with ENOMEM\n", i);
		}
		#endif
	}
	PASSED;
}

#else /* WITHOUT_XOPEN */
int main(int argc, char * argv[])
{
	output_init();
	UNTESTED("This test requires XSI features");
}
#endif