/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that pthread_attr_destroy()
 *  shall destroy a thread attributes object.  An implementation may cause
 *  pthread_attr_destroy() to set 'attr' to an implementation-defined invalid
 *  value.
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Destroy that initialized attribute using pthread_attr_destroy()
 * 3.  Using pthread_attr_create(), pass to it the destroyed attribute. It
 *     should return the error EINVAL, the value specified by 'attr' is
 *     is invalid.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

void *a_thread_func()
{

	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	pthread_attr_t new_attr;
	int ret;

	/* Initialize attribute */
	if (pthread_attr_init(&new_attr) != 0) {
		perror("Cannot initialize attribute object\n");
		return PTS_UNRESOLVED;
	}

	/* Destroy attribute */
	if (pthread_attr_destroy(&new_attr) != 0) {
		perror("Cannot destroy the attribute object\n");
		return PTS_UNRESOLVED;
	}

	/* Creating a thread, passing to it the destroyed attribute, should
	 * result in an error value of EINVAL (invalid 'attr' value). */
	ret = pthread_create(&new_th, &new_attr, a_thread_func, NULL);

	if (ret == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if ((ret != 0) && ((ret == EPERM) || (ret == EAGAIN))) {
		perror("Error created a new thread\n");
		return PTS_UNRESOLVED;
	} else if (ret == 0) {
		printf
		    ("Test PASSED: NOTE*: Though returned 0 when creating a thread with a destroyed attribute, this behavior is compliant with garbage-in-garbage-out. \n");
		return PTS_PASS;
	} else {
		printf
		    ("Test FAILED: (1) Incorrect return code from pthread_create(); %d not EINVAL  or  (2) Error in pthread_create()'s behavior in returning error codes \n",
		     ret);
		return PTS_FAIL;
	}

}
