/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 Steps:
 1. Call pthread_sigmask with a randomly generated 
 value of  how that is checked to make sure it does not equal any of the three defined
 values of how which are SIG_SETMASK, SIG_BLOCK, or SIG_UNBLOCK. This should
 cause pthread_sigmask() to return EINVAL. If it doesn't, then fail, otherwise pass.

*/

#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int get_rand() {

	int r;
	r=rand();
	if ((r == SIG_BLOCK) || (r == SIG_SETMASK) || (r == SIG_UNBLOCK)) {
		r = get_rand();
	}
	return r;
}

void *a_thread_func() {

	int r=get_rand();
	sigset_t actl;
/*
	printf("SIG_SETMASK=%d\n", SIG_SETMASK);
	printf("SIG_BLOCK=%d\n", SIG_BLOCK);
	printf("SIG_UNBLOCK=%d\n", SIG_UNBLOCK);
	printf("r=%d\n", r);
*/
	sigemptyset(&actl);
	sigaddset(&actl, SIGABRT);

	if (pthread_sigmask(r, &actl, NULL) != EINVAL) {
		perror("pthread_sigmask() did not fail even though invalid how parameter was passed to it.\n");
		pthread_exit((void*)-1);
	}

	printf("PASS: pthread_sigmask returned the correct error value.\n");
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

