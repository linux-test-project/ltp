/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that when name begins with the slash character, then processes calling
 * shm_open() with the same value of name refer to the same shared memory
 * object, as long as that name has not been removed.
 *
 * Steps:
 *  1. Create a child process.
 * (The child process)
 *  2. Open a shared memory file O_RDWR with a name beginning with slash.
 *  3. Write a string in the file.
 * (The father)
 *  2. Open a shared memory file O_RDONLY with the same name.
 *  3. Read into the file.
 *  4. Check that it obtain the right string.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "posixtest.h"

#define BUF_SIZE 8
#define SHM_NAME "/posixtest_5-1"

char str[BUF_SIZE] = "qwerty";

int child_process()
{
	int fd;
	char *buf;

	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		kill(getppid(), SIGUSR1);
		return PTS_UNRESOLVED;
	}

	if (ftruncate(fd, BUF_SIZE) != 0) {
		perror("An error occurs when calling ftruncate()");
		kill(getppid(), SIGUSR1);
		return PTS_UNRESOLVED;
	}

	buf = mmap(NULL, BUF_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		perror("An error occurs when calling mmap()");
		kill(getppid(), SIGUSR1);
		return PTS_UNRESOLVED;
	}

	strcpy(buf, str);

	return PTS_PASS;
}

int main(void)
{
	int fd, child_pid;
	char *buf;

	child_pid = fork();
	if (child_pid == -1) {
		perror("An error occurs when calling fork()");
		return PTS_UNRESOLVED;
	} else if (child_pid == 0) {
		return child_process();
	}

	wait(NULL);

	fd = shm_open(SHM_NAME, O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	buf = mmap(NULL, BUF_SIZE, PROT_READ, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		perror("An error occurs when calling mmap()");
		return PTS_UNRESOLVED;
	}

	shm_unlink(SHM_NAME);

	if (strcmp(buf, str) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
