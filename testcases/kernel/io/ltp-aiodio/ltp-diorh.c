/*
 *   Copyright (C) 2003,2004 Red Hat, Inc.  All rights reserved.
 *
 *   The contents of this file may be used under the terms of the GNU
 *   General Public License version 2 (the "GPL")
 *
 *   Author: Stephen C. Tweedie <sct@redhat.com>
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
 *
 * Module: .c
 */

/*
 * Change History:
 *
 * 2/2004  Marty Ridgeway (mridge@us.ibm.com) Changes to adapt to LTP
 *
 */

#define _XOPEN_SOURCE 600
#define _GNU_SOURCE
#define MAX_ITERATIONS 250

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define BIGSIZE 128*1024*1024
#define READSIZE 32*1024*1024
#define WRITESIZE 32*1024*1024

int pagesize;
char *iobuf;
int pass = 0;

void assert(const char *what, int assertion)
{
	if (assertion)
		return;
	perror(what);
	exit(1);
}

void do_buffered_writes(int fd, int pattern)
{
	int rc;
	int offset;

	memset(iobuf, pattern, WRITESIZE);
	for (offset = 0; offset + WRITESIZE <= BIGSIZE; offset += WRITESIZE) {
		rc = pwrite(fd, iobuf, WRITESIZE, offset);
		assert("pwrite", rc >= 0);
		if (rc != WRITESIZE) {
			fprintf(stderr, "Pass %d: short write (%d out of %d)\n",
				pass, rc, WRITESIZE);
			exit(1);
		}
		fsync(fd);
	}
}

int do_direct_reads(char *filename)
{
	int fd;
	int offset;
	int rc, i;
	int *p;

	fd = open(filename, O_DIRECT | O_RDONLY, 0);
	assert("open", fd >= 0);

	for (offset = 0; offset + READSIZE <= BIGSIZE; offset += READSIZE) {
		rc = pread(fd, iobuf, READSIZE, offset);
		assert("pread", rc >= 0);
		if (rc != READSIZE) {
			fprintf(stderr, "Pass: %d short read (%d out of %d)\n",
				pass, rc, READSIZE);
			exit(1);
		}
		for (i = 0, p = (int *)iobuf; i < READSIZE; i += 4) {
			if (*p) {
				fprintf(stderr,
					"Pass: %d Found data (%08x) at offset %d+%d\n",
					pass, *p, offset, i);
				close(fd);
				return 1;
			}
			p++;
		}
	}
	close(fd);
	return 0;
}

int main(int argc, char *argv[])
{
	char *filename;
	int fd;
	int pid;
	int err;
	int bufsize;

	if (argc != 2) {
		fprintf(stderr, "Needs a filename as an argument.\n");
		exit(1);
	}

	filename = argv[1];

	pagesize = getpagesize();
	bufsize = READSIZE;
	if (WRITESIZE > READSIZE)
		bufsize = WRITESIZE;
	err = posix_memalign((void **)&iobuf, pagesize, bufsize);
	if (err) {
		fprintf(stderr, "Error allocating %d aligned bytes.\n",
			bufsize);
		exit(1);
	}

	fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0666);
	assert("open", fd >= 0);

	do {

		assert("ftruncate", ftruncate(fd, BIGSIZE) == 0);
		fsync(fd);

		pid = fork();
		assert("fork", pid >= 0);

		if (!pid) {
			do_buffered_writes(fd, 0);
			exit(0);
		}

		err = do_direct_reads(filename);

		wait4(pid, NULL, WNOHANG, 0);

		if (err)
			break;

		/* Fill the file with a known pattern so that the blocks
		 * on disk can be detected if they become exposed. */
		do_buffered_writes(fd, 1);
		fsync(fd);

		assert("ftruncate", ftruncate(fd, 0) == 0);
		fsync(fd);
	} while (pass++ < MAX_ITERATIONS);

	if (!err) {
		fprintf(stdout, "ltp-diorh: Completed %d iterations OK \n",
			pass);
	}

	return err;
}
