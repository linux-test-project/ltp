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
 * sched_rr_get_interval() returns 0 on success.
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"
#include <time.h>

int main(int argc, char **argv)
{	       
	struct timespec interval;
	int result = -2;

	interval.tv_sec = -1;
	interval.tv_nsec = -1;
	
	result = sched_rr_get_interval(0, &interval);
	
	if(result == 0 &&
	   interval.tv_sec >= 0 &&
	   interval.tv_nsec >= 0 &&
	   errno == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if( interval.tv_sec == -1 ) {
		printf("interval.tv_sec  not updated.\n");
		return PTS_FAIL;
	}
	
	if( interval.tv_nsec == -1 ) {
		printf("interval.tv_nsec  not updated.\n");
		return PTS_FAIL;
	}

	if(result != 0) {
		printf("Returned code != 0.\n");
		return PTS_FAIL;
	}

	if(errno != 0 ) {
		perror("Unexpected error");
		return PTS_FAIL;
	} else {
		perror("Unresolved test error");
		return PTS_UNRESOLVED;	
	}
	
}


