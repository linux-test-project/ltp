/*
    Copyright (c) 2002, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content 
    of this license, see the COPYING file at the top level of this 
    source tree.
 */
/*
 * ERROR: ENAMETOOLONG: Trying to unlink a named semaphore which exceeds the
 * maximum of NAME_MAX.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include "posixtest.h"

#define TEST "5-1"
#define FUNCTION "sem_unlink"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "


int main()
{
	sem_t   *mysemp;
	char semname[NAME_MAX];
    	int i;


	sprintf(semname, "/tmp/" FUNCTION "_" TEST "_%d", getpid());

	for (i=0; i< NAME_MAX ; i++) {  /* Making the longer than 255 */
		strcat(semname,"d");
	}

	mysemp = sem_open(semname, O_CREAT,0,1);
/*
	len = strlen(semname);
	printf("LENGTH %i\n", len);
*/
	if ((sem_unlink(semname) == -1) && (errno == ENAMETOOLONG ) ) {
                puts("TEST PASSED");
                return PTS_PASS;
        } else {
                printf("TEST FAILED ERROR IS: %s\n", strerror(errno));
                return PTS_FAIL;
        }

}
