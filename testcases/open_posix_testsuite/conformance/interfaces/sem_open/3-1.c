/*
    Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content 
    of this license, see the COPYING file at the top level of this 
    source tree.
 */
/*
   open_sem test case that attempts to open a new semaphore with read 
   permissions, close it, then create the same semaphore with write 
   permissions which should come up with denial access.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"


#define TEST "3-1"
#define FUNCTION "sem_open"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "


int main()
{
	sem_t   *mysemp;
	char semname[50];


	if (getuid() == 0) {
		puts("Run this test case as a Regular User, but not ROOT");
		return PTS_UNTESTED;
	}

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	/*Trying to open the first Sem with read mode */
	mysemp = sem_open(semname, O_CREAT, 0444, 1);

	/* Opening the same existance SEM with write mode */
	mysemp = sem_open(semname, O_CREAT, 0222 , 1);

	if ( mysemp  != SEM_FAILED )
	{
  		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if (errno == EACCES )  {
		puts("TEST PASSED");
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
