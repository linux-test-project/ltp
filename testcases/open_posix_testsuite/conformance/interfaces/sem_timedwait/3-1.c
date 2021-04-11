/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/* This tests case will open a locked semaphore.  The time will tick 5 times
 * until the absolute time passes.  The sempahore will unlock, then the
 * sem_timedwait call will immediately lock again.
 */


#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef __linux__
#include <features.h>
#endif
#include <semaphore.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{

	struct timespec ts;
	sem_t mysemp;
	int i = 0;
	int val;

	if (sem_init(&mysemp, 0, 0) == -1) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec = time(NULL);
	ts.tv_nsec = 0;

	while (sem_timedwait(&mysemp, &ts) == -1) {
		ts.tv_sec += 1;
//              printf("%s \n", asctime(localtime(&ts.tv_sec)));
		i++;
//              printf("i=%d\n",i);
		if (i == 5) {
			sem_post(&mysemp);
		}
	}

	/* Value of Semaphore */
	if (sem_getvalue(&mysemp, &val) == -1) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}

	/* Checking if the value of the Semaphore after lock & unlock */
	if (val == 0) {
		puts("TEST PASSED: Sem unlocked after 5 timeouts");
		sem_destroy(&mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
