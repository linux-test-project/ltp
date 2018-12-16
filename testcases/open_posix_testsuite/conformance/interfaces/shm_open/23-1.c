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

/* adam.li: 2004-04-30: Rewrite the test case. The idea is that with
   O_CREAT and O_EXCL specified, to shm_open() a object can only success
   once, although multiple processes might open with the same name at the
   same time.
 */

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

#define NAME_SIZE 20
#define SHM_NAME "/posixtest_23-1_%d"

/* The processes communicate by a shared memory object */
#define SHM_RESULT_NAME "/result_23-1"

#define NPROCESS 1000		/* Number of concurrent processes */
#define NLOOP 1000		/* Number of shared memory object */

char name[NAME_SIZE];
int *create_cnt;
sem_t *sem;

int child_func(void)
{
	int i, fd;
	struct timespec ts = {.tv_sec = 0,.tv_nsec = 0 };
	int msec = 0;

	sleep(1);
	srand(time(NULL));
	for (i = 0; i < NLOOP; i++) {
		sprintf(name, SHM_NAME, i);
		fd = shm_open(name, O_RDONLY | O_CREAT | O_EXCL,
			      S_IRUSR | S_IWUSR);
		if (fd != -1) {
			sem_wait(sem);
			//fprintf(stderr, "%d: %d\n", getpid(), *create_cnt);
			(*create_cnt)++;
			sem_post(sem);
		}
		/* get a random number [0, 20] */
		msec = (int)(20.0 * rand() / RAND_MAX);
		ts.tv_nsec = msec * 1000000;
		nanosleep(&ts, NULL);
	}
	return 0;
}

int main(void)
{
	int i, pid, result_fd;
	char semname[20];

	snprintf(semname, 20, "/sem23-1_%d", getpid());
	sem = sem_open(semname, O_CREAT, 0777, 1);
	if (sem == SEM_FAILED || sem == NULL) {
		perror("error at sem_open");
		return PTS_UNRESOLVED;
	}
	sem_unlink(semname);

	result_fd = shm_open(SHM_RESULT_NAME,
			     O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (result_fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}
	shm_unlink(SHM_RESULT_NAME);
	if (ftruncate(result_fd, sizeof(*create_cnt)) != 0) {
		perror("An error occurs when calling ftruncate()");
		shm_unlink(SHM_RESULT_NAME);
		return PTS_UNRESOLVED;
	}

	create_cnt = mmap(NULL, sizeof(*create_cnt), PROT_WRITE | PROT_READ,
			  MAP_SHARED, result_fd, 0);
	if (create_cnt == MAP_FAILED) {
		perror("An error occurs when calling mmap()");
		shm_unlink(SHM_RESULT_NAME);
		return PTS_UNRESOLVED;
	}

	*create_cnt = 0;

	for (i = 0; i < NPROCESS; i++) {
		pid = fork();
		if (pid == -1) {
			perror("An error occurs when calling fork()");
			return PTS_UNRESOLVED;
		} else if (pid == 0) {
			child_func();
			exit(0);
		}
	}

	while (wait(NULL) > 0) ;

	for (i = 0; i < NLOOP; i++) {
		sprintf(name, SHM_NAME, i);
		shm_unlink(name);
	}

	fprintf(stderr, "create_cnt: %d\n", *create_cnt);
	if (*create_cnt != NLOOP) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	return PTS_PASS;
}
