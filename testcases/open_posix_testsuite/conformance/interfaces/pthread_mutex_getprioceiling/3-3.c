/*
 * Copyright (c) 2010, Ngie Cooper.
 *
 * Test that pthread_mutex_getprioceiling() fails because:
 *
 * [EINVAL]
 *     The protocol attribute of mutex is PTHREAD_PRIO_NONE.
 *
 * by explicitly specifying the protocol as PTHREAD_PRIO_NONE.
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Explicitly set the protocol using PTHREAD_PRIO_NONE.
 * 3.  Call pthread_mutex_getprioceiling() to obtain the prioceiling.
 *
 */

#include <pthread.h>
#include <errno.h>
#include <pwd.h>
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

	pthread_mutexattr_t mutex_attr;
	pthread_mutex_t mutex;
	int error, prioceiling;

	error = pthread_mutexattr_init(&mutex_attr);
	if (error) {
		printf("pthread_mutexattr_init failed: %s\n", strerror(error));
		return PTS_UNRESOLVED;
	}

	/*
	 * The default protocol is PTHREAD_PRIO_NONE according to
	 * pthread_mutexattr_getprotocol.
	 */
	error = pthread_mutexattr_setprotocol(&mutex_attr,
					      PTHREAD_PRIO_INHERIT);
	if (error) {
		printf("pthread_mutexattr_setprotocol failed: %s\n",
		       strerror(error));
		return PTS_UNRESOLVED;
	}

	/* Initialize a mutex object */
	error = pthread_mutex_init(&mutex, &mutex_attr);
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

	(void)pthread_mutexattr_destroy(&mutex_attr);
	(void)pthread_mutex_destroy(&mutex);

	return (error == EINVAL ? PTS_PASS : PTS_UNRESOLVED);
#else
	printf("pthread_mutex_getprioceiling not supported");
	return PTS_UNSUPPORTED;
#endif

}
