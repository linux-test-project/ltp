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
 * Test that sched_setparam() sets errno == EPERM when the requesting process
 * does not have permission to set the scheduling parameters for the specified
 * process, or does not have the appropriate privilege to invoke schedparam().
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"



int main(){
	int result;
        struct sched_param param;

        /* We assume process Number 1 is created by root */
        /* and can only be accessed by root */ 
        /* This test should be run under standard user permissions */
        if (getuid() == 0) {
                puts("Run this test case as a Regular User, but not ROOT");
                return PTS_UNTESTED;
        }

	if(sched_getparam(0, &param) == -1) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	result = sched_setparam(1, &param);

	if(result == -1 && errno == EPERM) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if(errno != EPERM) {
	        perror("errno is not EPERM");
		return PTS_FAIL;
	} else {
		printf("The returned code is not -1.\n");
		return PTS_FAIL;
	}
}
