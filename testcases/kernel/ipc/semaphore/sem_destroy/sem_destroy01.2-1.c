/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content 
    of this license, see the COPYING file at the top level of this 
    source tree.
 */

/*
 * Semaphore might not be destroyed until you reinitializes the 
 * semaphore by another call of sem_init.
*/

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "2-1"
#define FUNCTION "sem_destroy"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "


int main ()
{
	sem_t   mysemp;
	int status;

	if ( sem_init (&mysemp, 0, 2) == -1 ) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	if (sem_trywait(&mysemp) == -1 ) {
		perror(ERROR_PREFIX "trywait");
                return PTS_UNRESOLVED;
        }

        if( sem_destroy(&mysemp) == -1 ) {
		perror(ERROR_PREFIX "destroy");
                return PTS_UNRESOLVED;
        }

	/* Try to lock the semaphore after destroying it */
	status = sem_trywait(&mysemp); 
	if (status == -1) {
		perror(ERROR_PREFIX "trywait");
                return PTS_UNRESOLVED;
        }

	/* reinitialize by another call of sem_init */
	if ( sem_init (&mysemp, 0, 2) == -1 ) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	if (sem_trywait(&mysemp) == -1 ) {
		perror(ERROR_PREFIX "trywait");
                return PTS_UNRESOLVED;
        }

	printf("status %i\n", status);
        if(( sem_destroy(&mysemp) == 0 ) && ( status == 0 )) {
                puts("TEST PASSED");
                return PTS_PASS;
        } else {
                puts("TEST FAILED");
                return PTS_FAIL;
	}
}

