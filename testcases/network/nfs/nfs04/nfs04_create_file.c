/*
 *
 *   Copyright (c) International Business Machines  Corp., 2003
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 *  FILE        : create_file.c
 *  PURPOSE	: Creates a text file of specified size.
 *  HISTORY	:
 *  	10/17/2003 Robbie Williamson
 *	  -Written
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/* set write buffer size to whatever floats your boat.  I usually use 1M */
#define BSIZE 1048576L

int main(int argc, char *argv[])
{
	off_t i;
	long bufnum;
	char buf[BSIZE];
	off_t fd;

	if (argc != 3 || atoi(argv[1]) < 1) {
		printf
		    ("usage:\n\tcreate_file <# of %ld buffers to write> <name of file to create>\n\t ex. # create_file 10 /tmp/testfile\n",
		     BSIZE);
		exit(3);
	}
	bufnum = strtol(argv[1], NULL, 0);
	printf("Started building a %lu megabyte file\n", bufnum);
	buf[0] = 'A';
	for (i = 1; i < BSIZE; i++)
		buf[i] = buf[i - 1] + 1;
	buf[BSIZE - 1] = 'Z';

	if ((fd = creat(argv[2], 0755)) == -1)
		perror("lftest: ");

	for (i = 0; i < bufnum; i++) {
		if (write(fd, buf, BSIZE) == -1)
			return -1;
		else {
			printf(".");
			fflush(stdout);
		}
		fsync(fd);
	}
	close(fd);
	printf("\nFinished building a %lu megabyte file\n", bufnum);
	return (0);
}
