// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *               2004 Open Source Development Lab
 *               2004  Marty Ridgeway <mridge@us.ibm.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Append zeroed data to a file using libaio while other processes are doing
 * buffered reads and check if the buffer reads always see zero.
 */

#define _GNU_SOURCE

#include "tst_test.h"

#ifdef HAVE_LIBAIO
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libaio.h>
#include "common.h"

static volatile int *run_child;

static char *str_numchildren;
static char *str_writesize;
static char *str_numaio;
static char *str_appends;

static int numchildren = 8;
static long long writesize = 64 * 1024;
static int numaio = 16;
static int appends = 1000;
static long long alignment;

/*
 * append to the end of a file using AIO DIRECT.
 */
static void aiodio_append(char *filename, int bcount, long long align, long long ws, int naio)
{
	int fd;
	void *bufptr;
	int i;
	int w;
	struct iocb iocb_array[naio];
	struct iocb *iocbs[naio];
	off_t offset = 0;
	io_context_t myctx;
	struct io_event event;

	fd = SAFE_OPEN(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666);

	/*
	 * Prepare AIO write context.
	 */
	memset(&myctx, 0, sizeof(myctx));
	w = io_queue_init(naio, &myctx);
	if (w < 0)
		tst_brk(TBROK, "io_queue_init: %s", tst_strerrno(-w));

	for (i = 0; i < naio; i++) {
		bufptr = SAFE_MEMALIGN(align, ws);
		memset(bufptr, 0, ws);
		io_prep_pwrite(&iocb_array[i], fd, bufptr, ws, offset);
		iocbs[i] = &iocb_array[i];
		offset += ws;
	}

	/*
	 * Start the 1st AIO requests.
	 */
	w = io_submit(myctx, naio, iocbs);
	if (w < 0) {
		io_destroy(myctx);
		tst_brk(TBROK, "io_submit (multiple): %s", tst_strerrno(-w));
	}

	/*
	 * As AIO requests finish, keep issuing more AIOs.
	 */
	for (; i < bcount; i++) {
		int n = 0;
		struct iocb *iocbp;

		n = io_getevents(myctx, 1, 1, &event, NULL);
		if (n > 0) {
			iocbp = (struct iocb *)event.obj;
			io_prep_pwrite(iocbp, fd, iocbp->u.c.buf, ws, offset);
			offset += ws;
			w = io_submit(myctx, 1, &iocbp);
			if (w < 0) {
				io_destroy(myctx);
				tst_brk(TBROK, "io_submit (single): %s", tst_strerrno(-w));
			}
		}
	}
}

static void setup(void)
{
	struct stat sb;
	int maxaio;

	if (tst_parse_int(str_numaio, &numaio, 1, INT_MAX))
		tst_brk(TBROK, "Number of async IO blocks '%s'", str_numaio);

	SAFE_FILE_SCANF("/proc/sys/fs/aio-max-nr", "%d", &maxaio);
	tst_res(TINFO, "Maximum AIO blocks: %d", maxaio);

	if (numaio > maxaio)
		tst_res(TCONF, "Number of async IO blocks passed the maximum (%d)", maxaio);

	if (tst_parse_int(str_numchildren, &numchildren, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of children '%s'", str_numchildren);

	if (tst_parse_filesize(str_writesize, &writesize, 1, LLONG_MAX))
		tst_brk(TBROK, "Size of the file to write '%s'", str_writesize);

	if (tst_parse_int(str_appends, &appends, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of appends '%s'", str_appends);

	SAFE_STAT(".", &sb);
	alignment = sb.st_blksize;

	run_child = SAFE_MMAP(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (run_child) {
		*run_child = 0;
		SAFE_MUNMAP((void *)run_child, sizeof(int));
	}
}

static void run(void)
{
	char *filename = "aiodio_append";
	int status;
	int i, pid;

	*run_child = 1;

	for (i = 0; i < numchildren; i++) {
		if (!SAFE_FORK()) {
			io_read_eof(filename, run_child);
			return;
		}
	}

	pid = SAFE_FORK();
	if (!pid) {
		aiodio_append(filename, appends, alignment, writesize, numaio);
		return;
	}

	tst_res(TINFO, "Child %i appends to a file", pid);

	for (;;) {
		if (SAFE_WAITPID(pid, NULL, WNOHANG))
			break;

		sleep(1);

		if (!tst_remaining_runtime()) {
			tst_res(TINFO, "Test out of runtime, exiting");
			kill(pid, SIGKILL);
			SAFE_WAITPID(pid, NULL, 0);
			break;
		}
	}

	if (SAFE_WAITPID(-1, &status, WNOHANG))
		tst_res(TFAIL, "Non zero bytes read");
	else
		tst_res(TPASS, "All bytes read were zeroed");

	*run_child = 0;

	SAFE_UNLINK(filename);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.max_runtime = 1800,
	.options = (struct tst_option[]) {
		{"n:", &str_numchildren, "Number of threads (default 16)"},
		{"s:", &str_writesize, "Size of the file to write (default 64K)"},
		{"c:", &str_appends, "Number of appends (default 1000)"},
		{"b:", &str_numaio, "Number of async IO blocks (default 16)"},
		{}
	},
	.skip_filesystems = (const char *[]) {
		"tmpfs",
		NULL
	},
};
#else
TST_TEST_TCONF("test requires libaio and its development packages");
#endif
