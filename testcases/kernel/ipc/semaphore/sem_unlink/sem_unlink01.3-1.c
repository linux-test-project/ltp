/*
    Copyright (c) 2002, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content 
    of this license, see the COPYING file at the top level of this 
    source tree.
 */
/*
   sem_unlink test case that attempts to create a new semaphore with write
   permissions, then try to unlink this semaphore which should give 
   permission error (EACCES).
 */

#include <pwd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"



#define TEST "3-1"
#define FUNCTION "sem_unlink"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "


int main()
{
	sem_t   *mysemp;
	char semname[50];
        struct passwd *ltpuser;

        /* Get the user id "nobody" */
        if ((ltpuser = getpwnam("nobody")) == NULL) {
        puts("nobody not found in /etc/passwd");
        return PTS_UNTESTED;
        }

        /* Switch to "nobody" */
        setuid(ltpuser->pw_uid);

       /* if (getuid() == 0) {
                puts("Run this test case as a Regular User, but not ROOT");
		return PTS_UNTESTED;
        }*/

	sprintf(semname, "/tmp/" FUNCTION "_" TEST "_%d", getpid());


	/* Opening the same existance SEM with write mode */
	mysemp = sem_open(semname, O_CREAT, 0222 , 1);

	if ( mysemp  == SEM_FAILED || mysemp == NULL)
	{
  		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if ( sem_unlink(semname) == -1 ) {
		perror(ERROR_PREFIX "sem_unlink");
		return PTS_UNRESOLVED;
	}

	if (errno == EACCES )  {
		puts("TEST PASSED");
		sem_close(mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
