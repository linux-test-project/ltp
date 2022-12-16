// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2004 Stephen Hemminger <shemminger@osdl.org>
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 * Copyright (c) 2004 Marty Ridgeway <mridge@us.ibm.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Copy file by using an async I/O state machine.
 *
 * - Start read request
 * - When read completes turn it into a write request
 * - When write completes decrement counter and free up resources
 */

#define _GNU_SOURCE

#include "tst_test.h"

#ifdef HAVE_LIBAIO
#include <libaio.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include "common.h"

static const char *srcname = "srcfile.bin";
static const char *dstname = "dstfile.bin";

static char *str_aio_blksize;
static char *str_filesize;
static char *str_aionum;
static char *str_oflag;

static long long aio_blksize = 64 * 1024;
static long long filesize = 1 * 1024 * 1024;
static long long alignment;
static int aionum = 16;
static int srcflags = O_RDONLY;
static int dstflags = O_WRONLY;

static int srcfd;
static int dstfd;
static long long busy;
static long long tocopy;
static struct iocb **iocb_free;
static int iocb_free_count;

#ifndef howmany
# define howmany(x, y)  (((x) + ((y) - 1)) / (y))
#endif

static void fill_with_rand_data(int fd, long long size)
{
	const int bufsize = 64 * 1024;
	const int lower = 'a';
	const int upper = 'z';
	char buf[bufsize];
	long long i = 0, j;
	long long length, towrite;

	srand(time(NULL));

	for (j = 0; j < bufsize; j++)
		buf[j] = (rand() % (upper - lower + 1)) + lower;

	if (size <= bufsize) {
		SAFE_WRITE(0, fd, buf, size);
		return;
	}

	while (i < size) {
		if (!tst_remaining_runtime())
			tst_brk(TCONF, "Out of runtime!");

		length = rand() % (bufsize / 2) + bufsize / 2;
		towrite = MIN(length, size - i);

		i += towrite;

		SAFE_WRITE(1, fd, buf, towrite);
	}

	SAFE_FSYNC(fd);
}

static void async_init(void)
{
	int i;
	char *buff;

	iocb_free = SAFE_MALLOC(aionum * sizeof(struct iocb *));
	for (i = 0; i < aionum; i++) {
		iocb_free[i] = SAFE_MALLOC(sizeof(struct iocb));
		buff = SAFE_MEMALIGN(alignment, aio_blksize);

		io_prep_pread(iocb_free[i], -1, buff, aio_blksize, 0);
	}

	iocb_free_count = i;
}

static struct iocb *get_iocb(void)
{
	if (!iocb_free_count)
		return 0;

	return iocb_free[--iocb_free_count];
}

static void put_iocb(struct iocb *io)
{
	iocb_free[iocb_free_count++] = io;
}

static void async_write_done(LTP_ATTRIBUTE_UNUSED io_context_t ctx, struct iocb *iocb, long res, long res2)
{
	int iosize = iocb->u.c.nbytes;

	if (res != iosize)
		tst_brk(TBROK, "Write missing bytes expect %d got %ld", iosize, res);

	if (res2 != 0)
		tst_brk(TBROK, "Write error: %s", tst_strerrno(-res2));

	put_iocb(iocb);

	--busy;
	--tocopy;

	if (dstflags & O_DIRECT)
		SAFE_FSYNC(dstfd);

	if (!tst_remaining_runtime())
		tst_brk(TCONF, "Out of runtime!");
}

static void async_copy(io_context_t ctx, struct iocb *iocb, long res, long res2)
{
	int iosize = iocb->u.c.nbytes;
	char *buf = iocb->u.c.buf;
	off_t offset = iocb->u.c.offset;
	int w;

	if (res != iosize)
		tst_brk(TBROK, "Read missing bytes expect %d got %ld", iosize, res);

	if (res2 != 0)
		tst_brk(TBROK, "Read error: %s", tst_strerrno(-res2));

	io_prep_pwrite(iocb, dstfd, buf, iosize, offset);
	io_set_callback(iocb, async_write_done);

	w = io_submit(ctx, 1, &iocb);
	if (w < 0)
		tst_brk(TBROK, "io_submit error: %s", tst_strerrno(-w));
}

static void io_wait_run(io_context_t ctx, struct timespec *to)
{
	struct io_event events[aionum];
	struct io_event *ep;
	int n;

	n = io_getevents(ctx, 1, aionum, events, to);
	if (n < 0)
		tst_brk(TBROK, "io_getevents() failed: %s", tst_strerrno(-n));

	for (ep = events; n-- > 0; ep++) {
		io_callback_t cb = (io_callback_t) ep->data;
		struct iocb *iocb = ep->obj;

		cb(ctx, iocb, ep->res, ep->res2);
	}
}

static void async_run(io_context_t ctx, int fd, io_callback_t cb)
{
	long long offset = 0;
	int rc, i, n;
	int iosize;

	tocopy = filesize / aio_blksize;
	busy = 0;

	while (tocopy > 0) {
		n = MIN(aionum - busy, tocopy);

		if (n > 0) {
			struct iocb *ioq[n];

			for (i = 0; i < n; i++) {
				struct iocb *io = get_iocb();

				iosize = MIN(filesize - offset, aio_blksize);

				/* If we don't have any byte to write, exit */
				if (iosize <= 0)
					break;

				io_prep_pread(io, fd, io->u.c.buf, iosize, offset);
				io_set_callback(io, cb);

				ioq[i] = io;
				offset += iosize;
			}

			rc = io_submit(ctx, i, ioq);
			if (rc < 0)
				tst_brk(TBROK, "io_submit write error: %s", tst_strerrno(-rc));

			busy += n;
		}

		io_wait_run(ctx, 0);
	}
}

static void setup(void)
{
	int maxaio;
	long long leftover;

	if (tst_parse_int(str_aionum, &aionum, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of I/O '%s'", str_aionum);

	SAFE_FILE_SCANF("/proc/sys/fs/aio-max-nr", "%d", &maxaio);
	tst_res(TINFO, "Maximum AIO blocks: %d", maxaio);

	if (aionum > maxaio)
		tst_res(TCONF, "Number of async IO blocks passed the maximum (%d)", maxaio);

	if (tst_parse_filesize(str_aio_blksize, &aio_blksize, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid write blocks size '%s'", str_aio_blksize);

	if (str_oflag) {
		if (strncmp(str_oflag, "SYNC", 4) == 0) {
			dstflags |= O_SYNC;
		} else if (strncmp(str_oflag, "DIRECT", 6) == 0) {
			if (tst_fs_type(".") == TST_TMPFS_MAGIC)
				tst_brk(TCONF, "O_DIRECT not supported on tmpfs");

			srcflags |= O_DIRECT;
			dstflags |= O_DIRECT;
		}
	}

	alignment = tst_dev_block_size(".");

	if (dstflags & O_DIRECT && aio_blksize % alignment)
		tst_brk(TCONF, "Block size is not multiple of drive block size");

	if (tst_parse_filesize(str_filesize, &filesize, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid file size '%s'", str_filesize);

	leftover = filesize % aio_blksize;

	if (leftover > 0) {
		filesize = filesize - leftover + aio_blksize;

		tst_res(TINFO, "Rounded filesize to the next block size multiple: %llu", filesize);
	}

	tst_res(TINFO, "Fill %s with random data", srcname);

	srcfd = SAFE_OPEN(srcname, srcflags | O_RDWR | O_CREAT, 0666);
	fill_with_rand_data(srcfd, filesize);
	SAFE_CLOSE(srcfd);
}

static void cleanup(void)
{
	if (srcfd > 0)
		SAFE_CLOSE(srcfd);

	if (dstfd > 0)
		SAFE_CLOSE(dstfd);
}

static void run(void)
{
	const int buffsize = 4096;
	io_context_t myctx;
	struct stat st;
	char srcbuff[buffsize];
	char dstbuff[buffsize];
	int reads = 0;
	int i, r;

	srcfd = SAFE_OPEN(srcname, srcflags | O_RDWR | O_CREAT, 0666);
	dstfd = SAFE_OPEN(dstname, dstflags | O_WRONLY | O_CREAT, 0666);

	tst_res(TINFO, "Copy %s -> %s", srcname, dstname);

	memset(&myctx, 0, sizeof(myctx));
	io_queue_init(aionum, &myctx);

	async_init();
	async_run(myctx, srcfd, async_copy);

	io_destroy(myctx);
	SAFE_CLOSE(srcfd);
	SAFE_CLOSE(dstfd);

	tst_res(TINFO, "Comparing %s with %s", srcname, dstname);

	SAFE_STAT(dstname, &st);
	if (st.st_size != filesize) {
		tst_res(TFAIL, "Expected destination file size %lld but it's %ld", filesize, st.st_size);
		/* no need to compare files */
		return;
	}

	srcfd = SAFE_OPEN(srcname, srcflags | O_RDONLY, 0666);
	dstfd = SAFE_OPEN(dstname, srcflags | O_RDONLY, 0666);

	reads = howmany(filesize, buffsize);

	for (i = 0; i < reads; i++) {
		r = SAFE_READ(0, srcfd, srcbuff, buffsize);
		SAFE_READ(0, dstfd, dstbuff, buffsize);
		if (memcmp(srcbuff, dstbuff, r)) {
			tst_res(TFAIL, "Files are not identical");
			return;
		}
	}

	tst_res(TPASS, "Files are identical");

	SAFE_CLOSE(srcfd);
	SAFE_CLOSE(dstfd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.max_runtime = 1800,
	.needs_root = 1,
	.options = (struct tst_option[]) {
		{"b:", &str_aio_blksize, "Size of writing blocks (default 1K)"},
		{"s:", &str_filesize, "Size of file (default 10M)"},
		{"n:", &str_aionum, "Number of Async IO blocks (default 16)"},
		{"f:", &str_oflag, "Open flag: SYNC | DIRECT (default O_CREAT only)"},
		{},
	},
};
#else
TST_TEST_TCONF("test requires libaio and its development packages");
#endif
