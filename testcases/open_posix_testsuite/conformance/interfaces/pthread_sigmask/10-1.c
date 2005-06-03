/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 Attempt to add SIGKILL and SIGSTOP to the process's signal mask and 
 verify that:
 - They do not get added.
 - pthread_sigmask() does not return -1.
*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

void *a_thread_func()
{
	sigset_t set1, set2;
	int pthread_sigmask_return_val = 1;

	sigemptyset(&set1);
	sigemptyset(&set2);
	sigaddset(&set1, SIGKILL);
	sigaddset(&set1, SIGSTOP);
	pthread_sigmask_return_val = pthread_sigmask(SIG_SETMASK, &set1, NULL);
	pthread_sigmask(SIG_SETMASK, NULL, &set2);

	if (sigismember(&set2, SIGKILL)) {
		printf("FAIL: SIGKILL was added to the signal mask\n");
		pthread_exit((void*)-1);
	} 
	if (sigismember(&set2, SIGSTOP)) {
		printf("FAIL: SIGSTOP was added to the signal mask\n");
		pthread_exit((void*)-1);
	}
	if (pthread_sigmask_return_val != 0) {
		printf("FAIL: pthread_sigmask returned %d. System should be able to enforce blocking un-ignorable signals without causing pthread_sigmask() to return an error.\n", pthread_sigmask_return_val);
		pthread_exit((void*)-1);
	}

	printf("Test PASSED\n");
	pthread_exit((void*)0);

        /* To please some compilers */
	return NULL;
}

int main() {

        int *thread_return_value;

        pthread_t new_thread;

        if (pthread_create(&new_thread, NULL, a_thread_func, NULL) != 0) {
                perror("Error creating new thread\n");
                return PTS_UNRESOLVED;
        }

        if (pthread_join(new_thread, (void*)&thread_return_value) != 0) {
                perror("Error in pthread_join()\n");
                return PTS_UNRESOLVED;
        }

        if ((long)thread_return_value != 0) {
                if ((long)thread_return_value == 1) {
                        printf ("Test UNRESOLVED\n");
                        return PTS_UNRESOLVED;
                }
                else if ((long)thread_return_value == -1) {
                        printf ("Test FAILED\n");
                        return PTS_FAIL;
                }
                else {
                        printf ("Test UNRESOLVED\n");
                        return PTS_UNRESOLVED;
                }
        }
        return PTS_PASS;
}
