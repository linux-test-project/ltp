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
 * Test that sched_rr_get_interval() returns -1 on failure.
 *
 * The test create a child process which exit immediately and call
 * sched_rr_get_interval with the pid of defunct child.
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "posixtest.h"
#include <time.h>

int main(int argc, char **argv)
{	       

	struct timespec interval;
	int result = -2, child_pid, stat_loc;

	/* Create a child process which exit immediately */
	child_pid = fork();
	if(child_pid == -1){
	  perror("An error occurs when calling fork()");
	  return PTS_UNRESOLVED;
	} else if (child_pid == 0){
	  exit(0);
	}

	/* Wait for the child process to exit */
	if(wait(&stat_loc) == -1){
	  perror("An error occurs when calling wait()");
	  return PTS_UNRESOLVED;
	}
	
	/* Assume the pid is not yet reatributed to an other process */
	result = sched_rr_get_interval(child_pid, &interval);

	if(result == -1 && errno == ESRCH) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	
	if( errno != ESRCH ) {
		perror("Returned error is not ESRCH");
		return PTS_FAIL;
	}

	if( result == 0 ) {
		printf("Returned code == 0.\n");
		return PTS_FAIL;
	} else {
		perror("Unresolved test error");
		return PTS_UNRESOLVED;	
	}
	
}


