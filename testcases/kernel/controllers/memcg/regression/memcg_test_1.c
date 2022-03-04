// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2009 FUJITSU LIMITED
// Author: Li Zefan <lizf@cn.fujitsu.com>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define MAP_FLAGS		(MAP_PRIVATE|MAP_ANONYMOUS|MAP_LOCKED)

#define LOOP			40

#define FORKED_PROC_COUNT	10

int main(int argc, char *argv[])
{
	char buf[10];
	int i;
	int loop;
	int pid;
	int fd;
	int size = getpagesize();

	if (argc != 2) {
		perror("Invalid num of args");
		exit(1);
	}

	fd = open(argv[1], O_WRONLY);
	if (fd < 0) {
		perror("Could not open tasklist");
		exit(1);
	}

	for (loop = 0; loop < LOOP; loop++) {
		for (i = 0; i < FORKED_PROC_COUNT; i++) {
			pid = fork();
			if (pid == 0) {
				char *p;

				sprintf(buf, "%d", getpid());
				write(fd, buf, 10);
				fsync(fd);

				p = mmap(NULL, size, PROT_READ | PROT_WRITE,
					 MAP_FLAGS, 0, 0);

				if (p == MAP_FAILED) {
					perror("mmap failed");
					exit(1);
				} else
					exit(0);
			}
		}

		for (i = 0; i < FORKED_PROC_COUNT; i++)
			wait(NULL);
	}

	close(fd);

	return 0;
}
