/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_open() function sets errno = EACCES if O_TRUNC is
 * specified and write permission is denied
 */

/* FIXME : ambiguite de la spec */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_34-1"

int main() {
	int fd;

        /* This test should be run under standard user permissions */
        if (getuid() == 0) {
                puts("Run this test case as a Regular User, but not ROOT");
                return PTS_UNTESTED;
        }

	fd = shm_open(SHM_NAME, O_RDWR|O_CREAT, S_IRUSR);
	if(fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}
	
	fd = shm_open(SHM_NAME, O_RDWR | O_TRUNC, 0);
	
	if(fd == -1 && errno == EACCES) {
		printf("Test PASSED\n");
		shm_unlink(SHM_NAME);
		return PTS_PASS;
	} else if(fd != -1) {
		printf("shm_open success.\n");
		shm_unlink(SHM_NAME);
		return PTS_FAIL;
	}
	
	perror("shm_open");
	return PTS_FAIL;
}
