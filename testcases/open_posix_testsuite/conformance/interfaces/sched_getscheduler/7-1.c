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
 * Test that sched_getscheduler() sets errno == EPERM when the requesting 
 * process does not have permission
 */

#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int main(int argc, char **argv)
{	       

	int result = -1;

	/* We assume process Number 1 is created by root */
	/* and can only be accessed by root */ 
	/* This test should be run under standard user permissions */
	if (getuid() == 0) {
		puts("Run this test case as a Regular User, but not ROOT");
		return PTS_UNTESTED;
	}

	result = sched_getscheduler( 1 );
	
	if(result == -1 && errno == EPERM) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	if(result == 0) {
		printf("The function sched_getscheduler has successed.\n");
		return PTS_UNRESOLVED;
	}
	if(errno != EPERM ) {
		perror("errno is not EPERM");
		return PTS_FAIL;
	} else {
		perror("Unresolved test error");
		return PTS_UNRESOLVED;	
	}        

}


