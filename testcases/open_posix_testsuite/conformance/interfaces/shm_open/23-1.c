/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the check for the existence of the shared memory object and the
 * creation of the object if it does not exist is atomic with respect to other
 * processes executing shm_open() naming the same shared memory object with
 * O_EXCL and O_CREAT set.
 *
 * This test launch NPROCESS processes which all try to open NLOOP shared
 * memory objects. If an unexpected error occurs or if the number of created
 * objects is not NLOOP, the test failed. In other case the test is unresolved.
 */

/* ftruncate was formerly an XOPEN extension. We define _XOPEN_SOURCE here to
   avoid warning if the implementation does not program ftruncate as a base 
   interface */
#define _XOPEN_SOURCE 600

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define NAME_SIZE 20
#define SHM_NAME "/posixtest_23-1_%i"

/* The processes communicate by a shared memory object */
#define SHM_RESULT_NAME "/result_23-1"
/* It's an int array with two field */
#define CONFLICT 0 /* Index of the conflict flag */
#define NCREAT    1 /* Index of number of created object */
#define NPROCESS 100 /* Number of concurrent processes */
#define NLOOP 1000   /* Number of shared memory object */

int main() {
	int child_pid[NPROCESS];
	int i, pid, fd, result_fd, *result, childno = -1;
	char name[NAME_SIZE];

	result_fd = shm_open(SHM_RESULT_NAME, 
			     O_RDWR|O_CREAT, 
			     S_IRUSR|S_IWUSR);
	if(result_fd == -1){
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}
	if(ftruncate(result_fd, sizeof(int)) != 0) {
		perror("An error occurs when calling ftruncate()");
		shm_unlink(SHM_RESULT_NAME);
		return PTS_UNRESOLVED;	
	}

	result = mmap(NULL, sizeof(int), PROT_WRITE, MAP_SHARED, result_fd, 0);
	if( result == MAP_FAILED) {
		perror("An error occurs when calling mmap()");
		shm_unlink(SHM_RESULT_NAME);
		return PTS_UNRESOLVED;	
	}	

	*result = 0;

	for(i=0; i<NPROCESS; i++) {
	        pid = fork();
		if(pid == -1){
			perror("An error occurs when calling fork()");
			return PTS_UNRESOLVED;
		} else if(pid == 0){
			childno = i;
			break;

			printf("This code should not be executed.\n");
                        return PTS_UNRESOLVED;
		}
		
		child_pid[i] = pid;
	}

	for(i=0; i<NLOOP; i++){
		sprintf(name, SHM_NAME, i);
		fd = shm_open(name, O_RDONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
		if(fd != -1)
			result[NCREAT]++;
		if(fd == -1 && errno != EEXIST){
			perror("Unexpected error");
			result[CONFLICT] = 1;
		}
			
	}


	if(result[CONFLICT] || result[NCREAT] != NLOOP){
		if(childno == -1) printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if(childno == -1){
		printf("No error detected, but the test can't be resolved.\n");
		for(i=0; i<NLOOP; i++){
			sprintf(name, SHM_NAME, i);
			shm_unlink(name);
		}
		shm_unlink(SHM_RESULT_NAME);
	}
	return PTS_UNRESOLVED;
}
