
/*
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *               2004 Open Source Development Lab
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
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <limits.h>

#include "test.h"

#define NUM_CHILDREN 8

char *check_zero(unsigned char *buf, int size)
{
	unsigned char *p;

	p = buf;

	while (size > 0) {
		if (*buf != 0) {
			fprintf(stderr,
				"non zero buffer at buf[%d] => 0x%02x,%02x,%02x,%02x\n",
				buf - p, (unsigned int)buf[0],
				size > 1 ? (unsigned int)buf[1] : 0,
				size > 2 ? (unsigned int)buf[2] : 0,
				size > 3 ? (unsigned int)buf[3] : 0);
			fprintf(stderr, "buf %p, p %p\n", buf, p);
			return buf;
		}
		buf++;
		size--;
	}
	return 0;		/* all zeros */
}

int dio_read(char *filename)
{
	int fd;
	int r;
	void *bufptr;

	TEST(posix_memalign(&bufptr, 4096, 64 * 1024));
	if (TEST_RETURN) {
		tst_resm(TBROK | TRERRNO, "cannot malloc aligned memory");
		return -1;
	}

	while ((fd = open(filename, O_DIRECT | O_RDONLY)) < 0) {
	}
	fprintf(stderr, "dio_truncate: child reading file\n");
	while (1) {
		off_t offset;
		char *bufoff;

		/* read the file, checking for zeros */
		offset = lseek(fd, SEEK_SET, 0);
		do {
			r = read(fd, bufptr, 64 * 1024);
			if (r > 0) {
				if ((bufoff = check_zero(bufptr, r))) {
					fprintf(stderr,
						"non-zero read at offset %p\n",
						offset + bufoff);
					exit(1);
				}
				offset += r;
			}
		} while (r > 0);
	}
	return 0;
}

void dio_append(char *filename, int fill)
{
	int fd;
	void *bufptr;
	int i;
	int w;

	fd = open(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666);

	if (fd < 0) {
		perror("cannot create file");
		return;
	}

	TEST(posix_memalign(&bufptr, 4096, 64 * 1024));
	if (TEST_RETURN) {
		tst_resm(TBROK | TRERRNO, "cannot malloc aligned memory");
		close(fd);
		return;
	}

	memset(bufptr, fill, 64 * 1024);

	for (i = 0; i < 1000; i++) {
		if ((w = write(fd, bufptr, 64 * 1024)) != 64 * 1024) {
			fprintf(stderr, "write %d returned %d\n", i, w);
		}
	}
	close(fd);
}

int main(int argc, char **argv)
{
	char filename[PATH_MAX];
	int pid[NUM_CHILDREN];
	int num_children = 1;
	int i;

	snprintf(filename, sizeof(filename), "%s/aiodio/file",
		 getenv("TMP") ? getenv("TMP") : "/tmp");

	for (i = 0; i < num_children; i++) {
		if ((pid[i] = fork()) == 0) {
			/* child */
			return dio_read(filename);
		} else if (pid[i] < 0) {
			/* error */
			perror("fork error");
			break;
		} else {
			/* Parent */
			continue;
		}
	}

	/*
	 * Parent creates a zero file using DIO.
	 * Truncates it to zero
	 * Create another file with '0xaa'
	 */
	for (i = 0; i < 100; i++) {
		dio_append(filename, 0);
		truncate(filename, 0);
		dio_append("junkfile", 0xaa);
		truncate("junkfile", 0);
	}

	for (i = 0; i < num_children; i++) {
		kill(pid[i], SIGTERM);
	}

	return 0;
}
