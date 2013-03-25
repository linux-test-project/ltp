/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * A destroyed 'attr' attributes object can be reinitialized using
 * pthread_attr_init(); the results of otherwise referencing the object
 * after it has been destroyed are undefined.
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Destroy that initialized attribute using pthread_attr_destroy()
 * 3.  Initialize the pthread_attr_t object again.  This should not result
 *     in an error.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_attr_t new_attr;

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

	/* Initialize attribute.  This shouldn't result in an error. */
	if (pthread_attr_init(&new_attr) != 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	} else {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

}
