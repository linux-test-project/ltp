/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *  FILE        : lftest.c
 *  DESCRIPTION : The purpose of this test is to verify the file size limitations of a filesystem.
 *                It writes one buffer at a time and lseeks from the beginning of the file to the
 *                end of the last write position.  The intent is to test lseek64.
 *  HISTORY:
 *           06/19/01  :  Written by Jeff Martin(martinjn@us.ibm.com) to test large files on jfs.
 *           07/12/01  :  Added timing.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

/* set write buffer size to whatever floats your boat.  I usually use 1M */
#define BSIZE 1048576L
char buf[BSIZE];

int main(int argc, char *argv[])
{
	off_t i;
	long bufnum;
	off_t fd;
	time_t time1, time2;
	int writecnt = 0, seekcnt = 0, diff;

	time1 = time(NULL);

	if (argc != 2 || atoi(argv[1]) < 1) {
		printf("usage:<# of %ld buffers to write>\n", BSIZE);
		exit(3);
	}
	bufnum = strtol(argv[1], NULL, 0);
	printf("Started building a %lu megabyte file @ %s\n", bufnum,
	       asctime(localtime(&time1)));

	buf[0] = 'A';
	for (i = 1; i < BSIZE; i++)
		buf[i] = '0';
	buf[BSIZE - 1] = 'Z';

	if ((fd = creat("large_file", 0755)) == -1)
		perror("lftest: ");

	for (i = 0; i < bufnum; i++) {
		if (write(fd, buf, BSIZE) == -1)
			return -1;
		else {
			printf(".");
			writecnt++;
			fflush(stdout);
		}
		fsync(fd);
		if (lseek(fd, (i + 1) * BSIZE, 0) == -1)
			return -1;
		else
			seekcnt++;
	}
	close(fd);
	time2 = time(NULL);
	printf("\nFinished building a %lu megabyte file @ %s\n", bufnum,
	       asctime(localtime(&time2)));
	diff = time2 - time1;
	printf("Number of Writes: %d\n"
	       "Number of Seeks: %d\n"
	       "Total time for test to run: %d minute(s) and %d seconds\n",
	       writecnt, seekcnt, diff / 60, diff % 60);

	return 0;
}
