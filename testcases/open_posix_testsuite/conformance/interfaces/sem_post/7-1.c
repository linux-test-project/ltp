
/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content 
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

/* sem_wait will return successfully when sem_post will unlock the 
 * semaphore from another process.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "posixtest.h"


#define TEST "7-1"
#define FUNCTION "sem_post"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "


int main()
{  
        sem_t *mysemp;
        char semname[20];
	int pid;

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	/* Semaphore started out locked */
	mysemp = sem_open(semname, O_CREAT, 0, 0);
        if( mysemp == SEM_FAILED || mysemp == NULL ) {
        	perror(ERROR_PREFIX "sem_open");
               	return PTS_UNRESOLVED;
       	}

	pid = fork();
	if (pid > 0) // parent to unlock semaphore
	{  
	       	if( sem_wait(mysemp) == -1 ) {
			puts ("TEST FAILED");
			return PTS_FAIL;
		} else {
			puts("TEST PASSED");
			sem_close(mysemp);
			sem_unlink(semname);
			return PTS_PASS;
		}
	}
	else if (pid == 0)  // child to lock semaphore
	{  
		sleep(1);
		if (sem_post(mysemp) == -1 ) {
			perror(ERROR_PREFIX "sem_post");
			return PTS_FAIL;
		}
	return PTS_UNSUPPORTED;
	}
return PTS_UNSUPPORTED;
}
