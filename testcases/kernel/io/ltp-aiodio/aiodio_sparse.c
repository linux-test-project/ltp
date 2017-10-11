/*
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *               2004 Open Source Development Lab
 *
 * Copyright (c) 2004 Marty Ridgeway <mridge@us.ibm.com>
 *
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
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

#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <limits.h>
#include <getopt.h>


#include "config.h"
#include "test.h"
#include "safe_macros.h"

char *TCID = "aiodio_sparse";
int TST_TOTAL = 1;

#ifdef HAVE_LIBAIO
#include <libaio.h>

#define NUM_CHILDREN 1000

int debug;
int fd;

static void setup(void);
static void cleanup(void);
static void usage(void);

#include "common_sparse.h"

/*
 * do async DIO writes to a sparse file
 */
int aiodio_sparse(int fd, int align, int writesize, int filesize, int num_aio)
{
	int i, w;
	struct iocb **iocbs;
	off_t offset;
	io_context_t myctx;
	struct io_event event;
	int aio_inflight;

	if ((num_aio * writesize) > filesize)
		num_aio = filesize / writesize;

	memset(&myctx, 0, sizeof(myctx));
	io_queue_init(num_aio, &myctx);

	iocbs = malloc(sizeof(struct iocb *) * num_aio);
	for (i = 0; i < num_aio; i++) {
		if ((iocbs[i] = malloc(sizeof(struct iocb))) == 0) {
			tst_resm(TBROK | TERRNO, "malloc()");
			return 1;
		}
	}

	/*
	 * allocate the iocbs array and iocbs with buffers
	 */
	offset = 0;
	for (i = 0; i < num_aio; i++) {
		void *bufptr;

		TEST(posix_memalign(&bufptr, align, writesize));
		if (TEST_RETURN) {
			tst_resm(TBROK | TRERRNO, "cannot allocate aligned memory");
			return 1;
		}
		memset(bufptr, 0, writesize);
		io_prep_pwrite(iocbs[i], fd, bufptr, writesize, offset);
		offset += writesize;
	}

	/*
	 * start the 1st num_aio write requests
	 */
	if ((w = io_submit(myctx, num_aio, iocbs)) < 0) {
		tst_resm(TBROK, "io_submit() returned %i", w);
		return 1;
	}

	if (debug)
		tst_resm(TINFO, "io_submit() returned %d", w);

	/*
	 * As AIO requests finish, keep issuing more AIO until done.
	 */
	aio_inflight = num_aio;

	while (offset < filesize) {
		int n;
		struct iocb *iocbp;

		if (debug)
			tst_resm(TINFO,
				 "aiodio_sparse: offset %p filesize %d inflight %d",
				 &offset, filesize, aio_inflight);

		if ((n = io_getevents(myctx, 1, 1, &event, 0)) != 1) {
			if (-n != EINTR)
				tst_resm(TBROK, "io_getevents() returned %d",
					 n);
			break;
		}

		if (debug)
			tst_resm(TINFO,
				 "aiodio_sparse: io_getevent() returned %d", n);

		aio_inflight--;

		/*
		 * check if write succeeded.
		 */
		iocbp = (struct iocb *)event.obj;
		if (event.res2 != 0 || event.res != iocbp->u.c.nbytes) {
			tst_resm(TBROK,
				 "AIO write offset %lld expected %ld got %ld",
				 iocbp->u.c.offset, iocbp->u.c.nbytes,
				 event.res);
			break;
		}

		if (debug)
			tst_resm(TINFO,
				 "aiodio_sparse: io_getevent() res %ld res2 %ld",
				 event.res, event.res2);

		/* start next write */
		io_prep_pwrite(iocbp, fd, iocbp->u.c.buf, writesize, offset);
		offset += writesize;
		if ((w = io_submit(myctx, 1, &iocbp)) < 0) {
			tst_resm(TBROK, "io_submit failed at offset %ld",
				 offset);
			break;
		}

		if (debug)
			tst_resm(TINFO, "io_submit() return %d", w);

		aio_inflight++;
	}

	/*
	 * wait for AIO requests in flight.
	 */
	while (aio_inflight > 0) {
		int n;
		struct iocb *iocbp;

		if ((n = io_getevents(myctx, 1, 1, &event, 0)) != 1) {
			tst_resm(TBROK, "io_getevents failed");
			break;
		}
		aio_inflight--;
		/*
		 * check if write succeeded.
		 */
		iocbp = (struct iocb *)event.obj;
		if (event.res2 != 0 || event.res != iocbp->u.c.nbytes) {
			tst_resm(TBROK,
				 "AIO write offset %lld expected %ld got %ld",
				 iocbp->u.c.offset, iocbp->u.c.nbytes,
				 event.res);
		}
	}

	return 0;
}

static void usage(void)
{
	fprintf(stderr, "usage: dio_sparse [-n children] [-s filesize]"
		" [-w writesize]\n");
	exit(1);
}

int main(int argc, char **argv)
{
	char *filename = "aiodio_sparse";
	int pid[NUM_CHILDREN];
	int num_children = 1;
	int i;
	long alignment = 512;
	int writesize = 65536;
	int filesize = 100 * 1024 * 1024;
	int num_aio = 16;
	int children_errors = 0;
	int c;
	int ret;

	while ((c = getopt(argc, argv, "dw:n:a:s:i:")) != -1) {
		char *endp;
		switch (c) {
		case 'd':
			debug++;
			break;
		case 'i':
			num_aio = atoi(optarg);
			break;
		case 'a':
			alignment = strtol(optarg, &endp, 0);
			alignment = (int)scale_by_kmg((long long)alignment,
						      *endp);
			break;
		case 'w':
			writesize = strtol(optarg, &endp, 0);
			writesize =
			    (int)scale_by_kmg((long long)writesize, *endp);
			break;
		case 's':
			filesize = strtol(optarg, &endp, 0);
			filesize =
			    (int)scale_by_kmg((long long)filesize, *endp);
			break;
		case 'n':
			num_children = atoi(optarg);
			if (num_children > NUM_CHILDREN) {
				fprintf(stderr,
					"number of children limited to %d\n",
					NUM_CHILDREN);
				num_children = NUM_CHILDREN;
			}
			break;
		case '?':
			usage();
			break;
		}
	}

	setup();
	tst_resm(TINFO, "Dirtying free blocks");
	dirty_freeblocks(filesize);

	fd = SAFE_OPEN(cleanup, filename,
		O_DIRECT | O_WRONLY | O_CREAT | O_EXCL, 0600);
	SAFE_FTRUNCATE(cleanup, fd, filesize);

	tst_resm(TINFO, "Starting I/O tests");
	signal(SIGTERM, SIG_DFL);
	for (i = 0; i < num_children; i++) {
		switch (pid[i] = fork()) {
		case 0:
			SAFE_CLOSE(NULL, fd);
			read_sparse(filename, filesize);
			break;
		case -1:
			while (i-- > 0)
				kill(pid[i], SIGTERM);

			tst_brkm(TBROK | TERRNO, cleanup, "fork()");
		default:
			continue;
		}
	}
	tst_sig(FORK, DEF_HANDLER, cleanup);

	ret = aiodio_sparse(fd, alignment, writesize, filesize, num_aio);

	tst_resm(TINFO, "Killing childrens(s)");

	for (i = 0; i < num_children; i++)
		kill(pid[i], SIGTERM);

	for (i = 0; i < num_children; i++) {
		int status;
		pid_t p;

		p = waitpid(pid[i], &status, 0);
		if (p < 0) {
			tst_resm(TBROK | TERRNO, "waitpid()");
		} else {
			if (WIFEXITED(status) && WEXITSTATUS(status) == 10)
				children_errors++;
		}
	}

	if (children_errors)
		tst_resm(TFAIL, "%i children(s) exited abnormally",
			 children_errors);

	if (!children_errors && !ret)
		tst_resm(TPASS, "Test passed");

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	tst_tmpdir();
}

static void cleanup(void)
{
	if (fd > 0 && close(fd))
		tst_resm(TWARN | TERRNO, "Failed to close file");

	tst_rmdir();
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, "test requires libaio and it's development packages");
}
#endif
