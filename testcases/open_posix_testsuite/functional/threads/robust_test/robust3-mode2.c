
/*
 *  Copyright (c) 2003, Intel Corporation. All rights reserved.
 *  Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 *  This file is licensed under the GPL license.  For the full content
 *  of this license, see the COPYING file at the top level of this
 *  source tree.
 */

/* In x-mode
 * There are several threads that share a mutex, When the owner of mutex is 
 * dead, a waiter locks the mutex and will get EOWNERDEAD. In 
 * PTHREAD_MUTEX_ROBUST_NP Mode, if the owner can't recover it to 
 * heathy state(not_recoverable state), then calling 
 * pthread_mutex_setconsistency_np will change the mutex state to 
 * ENOTRECOVERABLE. 
 */ 

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "test.h"

#define THREAD_NUM		2	

pthread_mutex_t	mutex;

void *thread_1(void *arg)
{
 	pthread_mutex_lock(&mutex);
	DPRINTF(stdout,"Thread 1 locked the mutex \n");
	pthread_exit(NULL);
	return NULL;
}
void *thread_2(void *arg) 
{
	int state;
	int rc;
	pthread_t self = pthread_self();
        int policy = SCHED_FIFO;
	struct sched_param param;
        memset(&param, 0, sizeof(param));
        param.sched_priority = sched_get_priority_min(policy);

        rc = pthread_setschedparam(self, policy, &param);
        if (rc != 0) {
             EPRINTF("UNRESOLVED: pthread_setschedparam %d %s",
		     rc, strerror(rc));
	     exit(UNRESOLVED);
        }

	rc = pthread_mutex_lock(&mutex);
	if (rc != EOWNERDEAD)  {
		EPRINTF("FAIL:pthread_mutex_lock didn't return EOWNERDEAD \n");
		exit(FAIL);
	}
	DPRINTF(stdout,"Thread 2 lock the mutex and return EOWNERDEAD \n");

	state = 1;	
	if (pthread_mutex_setconsistency_np(&mutex, state) == 0) {
		pthread_mutex_unlock(&mutex);
		rc = pthread_mutex_lock(&mutex);
		if (rc != ENOTRECOVERABLE) {
			EPRINTF("FAIL:The mutex does not set to "
				"ENOTRECOVERABLE when called "
				"pthread_mutex_setconsistency_np successfully "
				"in x-mode");
			pthread_mutex_unlock(&mutex);
			exit(FAIL);
		}
		else {
			DPRINTF(stdout,"PASS: The mutex is set to ENOTRECOVERABLE if "
				"successfully called "
				"pthreadmutex_setconsistency_np in x-mode\n");
			pthread_mutex_unlock(&mutex);
		}
	}
	else {
		pthread_mutex_unlock(&mutex);
		rc = pthread_mutex_lock(&mutex);
		if (rc != EOWNERDEAD) {
			EPRINTF("FAIL:The mutex shall not set to "
				"ENOTRECOVERABLE automaticly after unlock "
				"if can't recover it to normal state in x-mode");
			pthread_mutex_unlock(&mutex);
			exit(FAIL);
		}
		else {
			DPRINTF(stdout,"PASS: The mutex remains in "
				"EOWNERDEAD state if the calling to "
				"pthread_mutex_consistency_np fails "
				"(Why fails?) in x-mode \n");
			pthread_mutex_unlock(&mutex);
		}
	} 
	pthread_exit(NULL);
	return NULL;
}

int main() 
{
	pthread_mutexattr_t attr;
	pthread_t threads[THREAD_NUM];
	pthread_attr_t threadattr;
	int rc;

	rc = pthread_mutexattr_init(&attr);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_mutexattr_init %d %s",
			rc, strerror(rc));
		return UNRESOLVED;
	}
	rc = pthread_mutexattr_setrobust_np(&attr, PTHREAD_MUTEX_ROBUST_NP);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_mutexattr_setrobust_np %d %s", 
			rc, strerror(rc));
		return UNRESOLVED;
	}
	rc = pthread_mutex_init(&mutex, &attr);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_mutex_init %d %s", 
			rc, strerror(rc));
		return UNRESOLVED;
	}
	rc = pthread_attr_init(&threadattr);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_attr_init %d %s",
			rc, strerror(rc));	
		return UNRESOLVED;
	}
	rc = pthread_create(&threads[0], &threadattr, thread_1, NULL);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_create %d %s",
			rc, strerror(rc));	
		return UNRESOLVED;
	}
	pthread_join(threads[0], NULL);
	DPRINTF(stdout,"Thread 1 exit without unlock the mutex...\n ");

	rc = pthread_create(&threads[1], &threadattr, thread_2, NULL);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_create %d %s",
			rc, strerror(rc));	
		return UNRESOLVED;
	}
	pthread_join(threads[1], NULL );
	DPRINTF(stdout,"Thread 2 exit ...\n ");

	DPRINTF(stdout,"PASS: Test PASSED\n");
	return PASS;
}
 
