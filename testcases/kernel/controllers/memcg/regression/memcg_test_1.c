/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2009 FUJITSU LIMITED                                         */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/* Author: Li Zefan <lizf@cn.fujitsu.com>                                     */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define MAP_FLAGS		(MAP_PRIVATE|MAP_ANONYMOUS|MAP_LOCKED)

#define LOOP			40

#define FORKED_PROC_COUNT	10

int main(void)
{
	char buf[10];
	int i;
	int loop;
	int pid;
	int size = getpagesize();
	int fd = open("memcg/0/tasks", O_WRONLY);

	if (fd < 0)
		return 1;

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
