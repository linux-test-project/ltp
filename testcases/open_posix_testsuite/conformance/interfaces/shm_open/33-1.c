/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_open() function sets errno = EACCES if the shared memory
 * object does not exists and permission to create the shared memory object is
 * denied.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SHM_NAME "/posixtest_33-1"

int main() {
	int fd, result;

        /* This test should be run under standard user permissions */
        if (getuid() == 0) {
                puts("Run this test case as a Regular User, but not ROOT");
                return PTS_UNTESTED;
        }


	result = shm_unlink(SHM_NAME);
	if(result != 0 && errno != ENOENT) { 
		/* The shared memory object exist and shm_unlink can not 
		   remove it. */
		perror("An error occurs when calling shm_unlink()");
		return PTS_UNRESOLVED;
	}
	errno = 0;
	
	fd = shm_open(SHM_NAME, O_RDWR|O_CREAT, 0);

	if( fd!=-1 && !errno){
		printf("You have permission to create a shared memory object.\n");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}else if(fd==-1 && errno==EACCES){
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	perror("Unexpected error");
	return PTS_UNRESOLVED;

}
