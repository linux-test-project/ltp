/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_open() function sets errno = ENFILE if too many file
 * descriptors are currently open in the system.
 *
 * Since the 
 */


#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_40-1"

int child_max;
int *child_pid;

void kill_children(int n) {
	int i;

	for(i=0; i<n; i++){
		kill(child_pid[i], SIGTERM);
	}
}

/*void sigint_handler(int signum) {
	kill_children(child_max);
	exit(PTS_UNRESOLVED);
}*/

void child_process() {
	int sig, fd = 0;
	sigset_t sigset;

	while(fd != -1) {
		fd = shm_open(SHM_NAME, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
	}

	if(errno == ENFILE){
		kill(getppid(), SIGCHLD);
		exit(PTS_PASS);
	} else if(errno == EMFILE){
		kill(getppid(), SIGUSR1);
	} else {
		perror("Unexpected error");
		kill(getppid(), SIGUSR2);
		exit(PTS_FAIL);
	}

	if(sigemptyset(&sigset) != 0) {
		perror("An error occurs when calling sigemptyset()");
		exit(1);
	}
	if(sigaddset(&sigset,SIGTERM) != 0) {
		perror("An error occurs when calling sigaddset()");
		exit(1);
	}
	if(sigwait(&sigset, &sig) != 0) {
		perror("An error occurs when calling sigwait()");
		exit(1);
	}
	exit(PTS_PASS);
}


int main() {
	sigset_t sigset;
	int sig = SIGUSR1, i = 0;

	child_max = sysconf(_SC_CHILD_MAX);
	if(child_max == -1) {
		perror("An error occurs when calling sysconf()");
		return PTS_UNRESOLVED;
        }
	child_pid = malloc(child_max*sizeof(int));
	if(child_pid == NULL) {
		perror("An error occurs when calling malloc()");
		return PTS_UNRESOLVED;
        }

	/*if(signal(SIGINT, sigint_handler) == SIG_ERR){
		perror("An error occurs when calling signal()");
		return PTS_UNRESOLVED;
        }*/

	if(sigemptyset(&sigset) != 0) {
		perror("An error occurs when calling sigemptyset()");
		exit(1);
	}
	if(sigaddset(&sigset,SIGUSR1) != 0) {
		perror("An error occurs when calling sigaddset()");
		exit(1);
	}	
	if(sigaddset(&sigset,SIGUSR2) != 0) {
		perror("An error occurs when calling sigaddset()");
		exit(1);
	}
	if(sigaddset(&sigset,SIGCHLD) != 0) {
		perror("An error occurs when calling sigaddset()");
		exit(1);
	}
	
	while(sig == SIGUSR1){
		
		child_pid[i] = fork();
		if(child_pid[i] == -1){			
			perror("An error occurs when calling fork()");
			kill_children(i);
			shm_unlink(SHM_NAME);
			return PTS_UNRESOLVED;
		} else if(child_pid[i] == 0) {
			child_process();
		}

		i++;
		if(sigwait(&sigset, &sig) != 0) {
			perror("An error occurs when calling sigwait()");
			kill_children(i);
			shm_unlink(SHM_NAME);
			return PTS_UNRESOLVED;
		}

	}		

	shm_unlink(SHM_NAME);

	if(sig == SIGCHLD) {
		kill_children(i);
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if(sig == SIGUSR2) {
		kill_children(i);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("This code should not be executed.\n");
	return PTS_UNRESOLVED;
}
