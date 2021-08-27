/*****************************************************************************/
/*                                                                           */
/*  Copyright (c) 2010 Mohamed Naufal Basheer                                */
/*                                                                           */
/*  This program is free software;  you can redistribute it and/or modify    */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either version 2 of the License, or        */
/*  (at your option) any later version.                                      */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY;  without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                */
/*  the GNU General Public License for more details.                         */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program;  if not, write to the Free Software             */
/*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA  */
/*                                                                           */
/*  File:    mem_process.c                                                   */
/*                                                                           */
/*  Purpose: act as a memory hog for the memcg_control tests                 */
/*                                                                           */
/*  Author:  Mohamed Naufal Basheer <naufal11@gmail.com >                    */
/*                                                                           */
/*****************************************************************************/

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Named pipe to act as a communication channel between
 * shell script & this process
 */
#define STATUS_PIPE "status_pipe"

int flag_exit;
int flag_allocated;
unsigned long memsize;

/*
 * process_options: process user specified options
 */
void process_options(int argc, char **argv)
{
	int c;
	char *end;

	opterr = 0;
	while ((c = getopt(argc, argv, "pm:")) != -1) {
		switch (c) {
		case 'm':
			memsize = strtoul(optarg, &end, 10);
			if (*end != '\0')
				errx(2, "invalid -m usage");
			break;
		default:
			errx(2, "invalid option specified");
		}
	}

	if (memsize <= 0)
		errx(3, "invalid usage");
}

/*
 * touch_memory: force physical memory allocation
 */
void touch_memory(char *p)
{
	int i;
	int pagesize = getpagesize();

	for (i = 0; i < (int)memsize; i += pagesize)
		p[i] = 0xef;
}

void mem_map(void)
{
	static char *p;

	if (flag_allocated) {
		if (munmap(p, memsize) == -1)
			err(5, "munmap failed");
	} else {
		p = mmap(NULL, memsize, PROT_READ | PROT_WRITE,
			 MAP_SHARED | MAP_ANONYMOUS, 0, 0);
		if (p == MAP_FAILED)
			err(4, "mmap failed");
		touch_memory(p);
	}
	flag_allocated = !flag_allocated;
}

/*
 * done: retrieve instructions from the named pipe
 */
char action(int fd)
{
	char ch;

	if (read(fd, &ch, 1) == -1)
		err(7, "Error reading named pipe");

	return ch;
}

int main(int argc, char **argv)
{
	int ret;
	int fd;
	char ch;

	process_options(argc, argv);

	ret = mkfifo(STATUS_PIPE, 0666);

	if (ret == -1 && errno != EEXIST)
		errx(1, "Error creating named pipe");

	if ((fd = open(STATUS_PIPE, O_RDONLY)) == -1)
		err(6, "Error opening named pipe");

	do {
		ch = action(fd);

		if (ch == 'm')
			mem_map();
	} while (ch != 'x');

	close(fd);

	remove(STATUS_PIPE);

	return 0;
}
