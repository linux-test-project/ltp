/*
    Copyright (c) 2002, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content 
    of this license, see the COPYING file at the top level of this 
    source tree.
 */
/*
   sem_open test case attempts create a long named SEM name that that should
   failed upon creating it.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include "posixtest.h"

#define TEST "7-1"
#define FUNCTION "sem_open"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "


int main()
{
	sem_t   *mysemp;
	char semname[NAME_MAX];
    	int i;


	sprintf(semname, "/tmp/" FUNCTION "_" TEST "_%d", getpid());

	for (i=0; i< NAME_MAX; i++) {  /* Making it longer than 255 */
		strcat(semname,"d");
	}

	mysemp = sem_open(semname, O_CREAT,0,1);
	if (( mysemp  == SEM_FAILED ) && (errno == ENAMETOOLONG )) 
	{
		puts("TEST PASS");
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED: sem_open should have not been created because of the ENAMETOOLONG");
		sem_unlink(semname);
		return PTS_FAIL;
	}

}
