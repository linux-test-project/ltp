/*
 * Copyright (c) 2010, Ngie Cooper.
 *
 * Test that pthread_mutex_getprioceiling() fails because:
 *
 * [EINVAL]
 *     The protocol attribute of mutex is PTHREAD_PRIO_NONE.
 *
 * by not specifying PTHREAD_PRIO_NONE and noting that the default (as per
 * pthread_mutexattr_getprotocol) is PTHREAD_PRIO_NONE.
 *
 * Steps:
 * 1.  Initialize a mutex via pthread_mutex_init.
 * 2.  Do not modify the mutex.
 * 3.  Call pthread_mutex_getprioceiling() to obtain the prioceiling.
 *
 */

#include <pthread.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
#if defined(_SC_PRIORITY_SCHEDULING)

	if (sysconf(_SC_PRIORITY_SCHEDULING) == -1) {
		printf("PRIORITY_SCHEDULING not supported\n");
		return PTS_UNSUPPORTED;
	}

	pthread_mutex_t mutex;
	int error, prioceiling;

	/*
	 * The default protocol is PTHREAD_PRIO_NONE according to
	 * pthread_mutexattr_getprotocol.
	 */

	/* Initialize a mutex object */
	error = pthread_mutex_init(&mutex, NULL);
	if (error) {
		printf("pthread_mutex_init failed: %s\n", strerror(error));
		return PTS_UNRESOLVED;
	}

	/* Get the prioceiling of the mutex. */
	error = pthread_mutex_getprioceiling(&mutex, &prioceiling);
	if (error) {
		if (error == EINVAL) {
			printf("pthread_mutex_getprioceiling failed as "
			       "expected\n");
		} else {
			printf("pthread_mutex_getprioceiling did not fail as "
			       "expected: %s\n", strerror(error));
		}
	} else
		printf("pthread_mutex_getprioceiling passed unexpectedly\n");

	(void)pthread_mutex_destroy(&mutex);

	return (error == EINVAL ? PTS_PASS : PTS_FAIL);
#else
	printf("pthread_mutex_getprioceiling not supported");
	return PTS_UNSUPPORTED;
#endif

}
