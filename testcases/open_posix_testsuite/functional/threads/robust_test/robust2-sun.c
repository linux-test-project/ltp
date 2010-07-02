
/*
 *  Copyright (c) 2003, Intel Corporation. All rights reserved.
 *  Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 *  This file is licensed under the GPL license.  For the full content
 *  of this license, see the COPYING file at the top level of this
 *  source tree.
 */

/* Sun mode
 * There are several threads that share a mutex, When the owner of mutex is 
 * dead, a waiter lock the mutex and will get EOWNERDEAD. In 
 * PTHREAD_MUTEX_ROBUST_SUN_NP Mode, if the owner think he can recover it 
 * to heathy state, he will call pthread_mutex_consistent_np to make 
 * the mutex consistent, if the call succeeds, the state of the mutex 
 * will change back to normal, if the call fails, the state of the mutex 
 * will remain as EOWNERDEAD.
 */ 

/*
 * XXX: pthread_mutexattr_setrobust_np and PTHREAD_MUTEX_ROBUST_SUN_NP isn't
 * POSIX.
 */
#error "Uses GNU-isms; needs fixing."
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "test.h"

#define THREAD_NUM	2	

pthread_mutex_t	mutex;

void *thread_1(void *arg)
{
 	pthread_mutex_lock(&mutex);
	DPRINTF(stdout,"Thread 1 lock the mutex\n");
	pthread_exit(NULL);
	return NULL;
}
void *thread_2(void *arg) 
{
	pthread_t self = pthread_self();
	int policy = SCHED_FIFO;
	struct sched_param	param;
	param.sched_priority = sched_get_priority_min(policy);
	int rc;
	
        rc = pthread_setschedparam(self, policy, &param); 
        if (rc != 0) {
	    EPRINTF("UNRESOLVED: pthread_setschedparam: %d %s",
	            rc, strerror(rc));
            exit(UNRESOLVED);
        }
#if __linux__
	if (pthread_mutex_lock(&mutex) != EOWNERDEAD)  {
		EPRINTF("FAIL:pthread_mutex_lock didn't return EOWNERDEAD");
		exit(FAIL);
	}
	DPRINTF(stdout,"Thread 2 locked the mutex and return EOWNERDEAD\n");

	if (pthread_mutex_consistent_np(&mutex) == 0) {
		pthread_mutex_unlock(&mutex);
		if (pthread_mutex_lock(&mutex) != 0) {
			EPRINTF("FAIL: The mutex didn't transit to normal state"
				" when calling to pthread_mutex_consistent_np"
			        " successfully in Sun Compability mode");
			pthread_mutex_unlock(&mutex);
			exit(FAIL);
		}
		else {
			DPRINTF(stdout,"PASS: The mutex transitted to normal state "
				"when calling to pthread_mutex_consistent_np "
				"successfully in Sun compability mode\n");
			pthread_mutex_unlock(&mutex);
		}
	}
	else {
		pthread_mutex_unlock(&mutex);
		if (pthread_mutex_lock(&mutex) != ENOTRECOVERABLE) {
			EPRINTF("FAIL:The mutex should remain as EOWNERDEAD "
				"if call pthread_mutex_consistent_np fails, "
				"so the mutex will transit to ENOTRECOVERABLE "
				"when unlocked in Sun compatibility mode");
		        pthread_mutex_unlock(&mutex);
			exit(FAIL);
		}
		else {
			DPRINTF(stdout,"PASS: The mutex transitted to ENOTRECOVERABLE "
				"state when pthread_mutex_consistent_np fails "
				"in Sun compatibility mode (why fails?)\n");
			pthread_mutex_unlock(&mutex);
		}
	}
#endif
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
#if __linux__
	rc = pthread_mutexattr_setrobust_np(&attr, PTHREAD_MUTEX_ROBUST_SUN_NP);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_mutexattr_setrobust_np %d %s", 
			rc, strerror(rc));
		return UNRESOLVED;
	}
#endif
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
	DPRINTF(stdout,"Thread 1 exited without unlock...\n");

	rc = pthread_create(&threads[1], &threadattr, thread_2, NULL);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_create %d %s", 
			rc, strerror(rc));
		return UNRESOLVED;
	}
	pthread_join(threads[1], NULL);
	DPRINTF(stdout,"Thread 2 exited ...\n");

	DPRINTF(stdout,"PASS: Test PASSED\n");
	return PASS;
}
