/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Test that sched_setscheduler() sets errno == EINVAL when the policy value is
 * not defined in the sched.h header.
 *
 * Assume that the header does not defined a scheduling policy with a value
 * of -1. (It is more coherent with the specificationS of sched_getscheduler
 * and sched_setscheduler for which the result code -1 indicate an error.)
 * If no error occurs whith -1, the test will run sched_setscheduler with the
 * very improbable policy value INVALID_POLICY.
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

/* There is no chance that a scheduling policy has such a value */
#define INVALID_POLICY -27367

int main(void)
{
	int result;
	struct sched_param param;

	param.sched_priority = 0;

	result = sched_setscheduler(0, -1, &param);

	if (result == -1 && errno == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (errno == EPERM) {
		printf
		    ("This process does not have the permission to set its own scheduling policy.\nTry to launch this test as root.\n");
		return PTS_UNRESOLVED;
	} else if (errno == 0) {
		printf
		    ("No error occurs, check if -1 a valid value for the scheduling policy.\n");
	} else {
		perror("Unknow error");
		return PTS_FAIL;
	}

	printf("Testing with very improbable policy value %i:\n",
	       INVALID_POLICY);

	result = sched_setscheduler(0, INVALID_POLICY, &param);

	if (result == -1 && errno == EINVAL) {
		printf("Test PASSED with policy value %i\n", INVALID_POLICY);
		return PTS_PASS;
	} else if (errno == 0) {
		printf
		    ("No error occurs, could %i be a valid value for the scheduling policy ???\n",
		     INVALID_POLICY);
		return PTS_UNRESOLVED;
	} else {
		perror("Unknow error");
		return PTS_FAIL;
	}

}
