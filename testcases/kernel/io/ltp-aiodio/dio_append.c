
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Module: .c
 */

/*
 * Change History:
 *
 * 2/2004  Marty Ridgeway (mridge@us.ibm.com) Changes to adapt to LTP
 *
 */
/*
 * dio_append - append zeroed data to a file using O_DIRECT while
 *	a 2nd process is doing buffered reads and check if the buffer
 *	reads always see zero.
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <limits.h>
#define NUM_CHILDREN 8

char *check_zero(unsigned char *buf, int size)
{
	unsigned char *p;

	p = buf;

	while (size > 0) {
		if (*buf != 0) {
			fprintf(stderr, "non zero buffer at buf[%d] => 0x%02x,%02x,%02x,%02x\n",
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
	return 0;	/* all zeros */
}

int read_eof(char *filename)
{
	int fd;
	int i;
	int r;
	char buf[4096];

	while ((fd = open(filename, O_RDONLY)) < 0) {
		sleep(1);	/* wait for file to be created */
	}

	for (i = 0 ; i < 1000000; i++) {
		off_t offset;
		char *bufoff;

		offset = lseek(fd, SEEK_END, 0);
		r = read(fd, buf, 4096);
		if (r > 0) {
			if ((bufoff = check_zero(buf, r))) {
				fprintf(stderr, "non-zero read at offset %p\n",
					offset + bufoff);
				exit(1);
			}
		}
	}
  return 0;
}

void dio_append(char *filename)
{
	int fd;
	void *bufptr;
	int i;
	int w;

	fd = open(filename, O_DIRECT|O_WRONLY|O_CREAT, 0666);

	if (fd < 0) {
		perror("cannot create file");
		return;
	}

	if (posix_memalign(&bufptr, 4096, 64*1024)) {
		perror("cannot malloc aligned memory");
		return;
	}

	memset(bufptr, 0, 64*1024);
	for (i = 0; i < 1000; i++) {
		if ((w = write(fd, bufptr, 64*1024)) != 64*1024) {
			fprintf(stderr, "write %d returned %d\n", i, w);
		}
	}
}

int main(int argc, char **argv)
{
	char filename[PATH_MAX];
	int pid[NUM_CHILDREN];
	int num_children = 1;
	int i;

	snprintf(filename, sizeof(filename), "%s/aiodio/file",
		getenv("TMP") ? getenv("TMP") : "/tmp");

	printf("Begin dio_append test...\n");

	for (i = 0; i < num_children; i++) {
		if ((pid[i] = fork()) == 0) {
			/* child */
			return read_eof(filename);
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
	 * Parent appends to end of file using direct i/o
	 */

	dio_append(filename);

	for (i = 0; i < num_children; i++) {
		kill(pid[i], SIGTERM);
	}
  return 0;
}