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
 * Change History:
 *
 * 2/2004  Marty Ridgeway (mridge@us.ibm.com) Changes to adapt to LTP
 *
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"
#include "test.h"

char *TCID = "aiodio_append";

#ifdef HAVE_LIBAIO
#include <libaio.h>

#define NUM_CHILDREN 8

#include "common_checkzero.h"

int read_eof(char *filename)
{
	int fd;
	int i;
	int r;
	char buf[4096];

	while ((fd = open(filename, O_RDONLY)) < 0) {
		sleep(1);	/* wait for file to be created */
	}

	for (i = 0; i < 1000000; i++) {
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

#define NUM_AIO 16
#define AIO_SIZE 64*1024

/*
 * append to the end of a file using AIO DIRECT.
 */
void aiodio_append(char *filename)
{
	int fd;
	void *bufptr;
	int i;
	int w;
	struct iocb iocb_array[NUM_AIO];
	struct iocb *iocbs[NUM_AIO];
	off_t offset = 0;
	io_context_t myctx;
	struct io_event event;
	struct timespec timeout;

	fd = open(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666);
	if (fd < 0) {
		perror("cannot create file");
		return;
	}

	memset(&myctx, 0, sizeof(myctx));
	io_queue_init(NUM_AIO, &myctx);

	for (i = 0; i < NUM_AIO; i++) {
		TEST(posix_memalign(&bufptr, 4096, AIO_SIZE));
		if (TEST_RETURN) {
			tst_resm(TBROK | TRERRNO, "cannot malloc aligned memory");
			return;
		}
		memset(bufptr, 0, AIO_SIZE);
		io_prep_pwrite(&iocb_array[i], fd, bufptr, AIO_SIZE, offset);
		iocbs[i] = &iocb_array[i];
		offset += AIO_SIZE;
	}

	/*
	 * Start the 1st NUM_AIO requests
	 */
	if ((w = io_submit(myctx, NUM_AIO, iocbs)) < 0) {
		fprintf(stderr, "io_submit write returned %d\n", w);
	}

	/*
	 * As AIO requests finish, keep issuing more AIOs.
	 */
	for (; i < 1000; i++) {
		int n = 0;
		struct iocb *iocbp;

		n = io_getevents(myctx, 1, 1, &event, &timeout);
		if (n > 0) {
			iocbp = (struct iocb *)event.obj;

			if (n > 0) {
				io_prep_pwrite(iocbp, fd, iocbp->u.c.buf,
					       AIO_SIZE, offset);
				offset += AIO_SIZE;
				if ((w = io_submit(myctx, 1, &iocbp)) < 0) {
					fprintf(stderr,
						"write %d returned %d\n", i, w);
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	int pid[NUM_CHILDREN];
	int num_children = 1;
	int i;
	char *filename = argv[1];

	printf("Starting aio/dio append test...\n");

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

	aiodio_append(filename);

	for (i = 0; i < num_children; i++) {
		kill(pid[i], SIGTERM);
	}

	return 0;
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "test requires libaio and it's development packages");
}
#endif
