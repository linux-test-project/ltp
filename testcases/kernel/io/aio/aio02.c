// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2003
 * Copyright (c) Linux Test Project, 2004-2019
 *
 *  AUTHORS
 *   Kai Zhao (ltcd3@cn.ibm.com)
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LIBAIO
#include <errno.h>
#include <stdlib.h>
#include <libaio.h>

#define AIO_MAXIO 32
#define AIO_BLKSIZE (64*1024)

static int wait_count;

#define DESC_FLAGS_OPR(x, y) .desc = (x == IO_CMD_PWRITE ? "WRITE: " #y: "READ : " #y), \
	.flags = y, .operation = x

struct testcase {
	const char *desc;
	int flags;
	int operation;
} testcases[] = {
	{
		DESC_FLAGS_OPR(IO_CMD_PWRITE, O_WRONLY | O_TRUNC | O_DIRECT | O_LARGEFILE | O_CREAT),
	},
	{
		DESC_FLAGS_OPR(IO_CMD_PREAD, O_RDONLY | O_DIRECT | O_LARGEFILE),
	},
	{
		DESC_FLAGS_OPR(IO_CMD_PWRITE, O_RDWR | O_TRUNC),
	},
	{
		DESC_FLAGS_OPR(IO_CMD_PREAD, O_RDWR),
	},
	{
		DESC_FLAGS_OPR(IO_CMD_PWRITE, O_WRONLY | O_TRUNC),
	},
	{
		DESC_FLAGS_OPR(IO_CMD_PREAD, O_RDONLY),
	},
};

/*
 * Fatal error handler
 */
static void io_error(const char *func, int rc)
{
	if (rc == -ENOSYS)
		tst_brk(TCONF, "AIO not in this kernel\n");
	else if (rc < 0)
		tst_brk(TFAIL, "%s: %s\n", func, strerror(-rc));
	else
		tst_brk(TFAIL, "%s: error %d\n", func, rc);
}

/*
 * write work done
 */
static void work_done(io_context_t ctx, struct iocb *iocb, long res, long res2)
{
	(void) ctx;  // silence compiler warning (-Wunused)

	if (res2 != 0)
		io_error("aio write", res2);

	if (res != (long)iocb->u.c.nbytes)
		tst_brk(TFAIL, "write missed bytes expect %lu got %ld\n",
			iocb->u.c.nbytes, res);

	wait_count--;
}

/*
 * io_wait_run() - wait for an io_event and then call the callback.
 */
static int io_wait_run(io_context_t ctx, struct timespec *to)
{
	struct io_event events[AIO_MAXIO];
	struct io_event *ep;
	int ret, n;

	/*
	 * get up to aio_maxio events at a time.
	 */
	ret = n = io_getevents(ctx, 1, AIO_MAXIO, events, to);

	/*
	 * Call the callback functions for each event.
	 */
	for (ep = events; n-- > 0; ep++) {
		io_callback_t cb = (io_callback_t) ep->data;
		struct iocb *iocb = ep->obj;
		cb(ctx, iocb, ep->res, ep->res2);
	}
	return ret;
}

static int io_tio(char *pathname, int flag, int operation)
{
	int res, fd = 0, i = 0;
	void *bufptr = NULL;
	off_t offset = 0;
	struct timespec timeout;
	struct stat fi_stat;
	size_t alignment;

	io_context_t myctx;
	struct iocb iocb_array[AIO_MAXIO];
	struct iocb *iocbps[AIO_MAXIO];

	fd = SAFE_OPEN(pathname, flag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	/* determine the alignment from the blksize of the underlying device */
	SAFE_FSTAT(fd, &fi_stat);
	alignment = fi_stat.st_blksize;

	res = io_queue_init(AIO_MAXIO, &myctx);

	for (i = 0; i < AIO_MAXIO; i++) {

		switch (operation) {
		case IO_CMD_PWRITE:
			if (posix_memalign(&bufptr, alignment, AIO_BLKSIZE)) {
				tst_brk(TBROK | TERRNO, "posix_memalign failed");
				return -1;
			}
			memset(bufptr, 0, AIO_BLKSIZE);

			io_prep_pwrite(&iocb_array[i], fd, bufptr,
					   AIO_BLKSIZE, offset);
			io_set_callback(&iocb_array[i], work_done);
			iocbps[i] = &iocb_array[i];
			offset += AIO_BLKSIZE;

			break;
		case IO_CMD_PREAD:
			if (posix_memalign(&bufptr, alignment, AIO_BLKSIZE)) {
				tst_brk(TBROK | TERRNO, "posix_memalign failed");
				return -1;
			}
			memset(bufptr, 0, AIO_BLKSIZE);

			io_prep_pread(&iocb_array[i], fd, bufptr,
					  AIO_BLKSIZE, offset);
			io_set_callback(&iocb_array[i], work_done);
			iocbps[i] = &iocb_array[i];
			offset += AIO_BLKSIZE;
			break;
		default:
			tst_res(TFAIL, "Command failed; opcode returned: %d\n", operation);
			return -1;
			break;
		}
	}

	do {
		res = io_submit(myctx, AIO_MAXIO, iocbps);
	} while (res == -EAGAIN);

	if (res < 0)
		io_error("io_submit tio", res);

	/*
	 * We have submitted all the i/o requests. Wait for them to complete and
	 * call the callbacks.
	 */
	wait_count = AIO_MAXIO;

	timeout.tv_sec = 30;
	timeout.tv_nsec = 0;

	switch (operation) {
	case IO_CMD_PREAD:
	case IO_CMD_PWRITE:
		{
			while (wait_count) {
				res = io_wait_run(myctx, &timeout);
				if (res < 0)
					io_error("io_wait_run", res);
			}
		}
		break;
	}

	SAFE_CLOSE(fd);

	for (i = 0; i < AIO_MAXIO; i++)
		if (iocb_array[i].u.c.buf != NULL)
			free(iocb_array[i].u.c.buf);

	io_queue_release(myctx);

	return 0;
}

static void test_io(unsigned int n)
{
	int status, new_flags;
	struct testcase *tc = testcases + n;

	new_flags = tc->flags;

	if ((tst_fs_type(".") == TST_TMPFS_MAGIC) && (tc->flags & O_DIRECT)) {
		tst_res(TINFO, "Drop O_DIRECT flag for tmpfs");
		new_flags &= ~O_DIRECT;
	}

	status = io_tio("file", new_flags, tc->operation);
	if (status)
		tst_res(TFAIL, "%s, status = %d", tc->desc, status);
	else
		tst_res(TPASS, "%s", tc->desc);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test = test_io,
	.tcnt = ARRAY_SIZE(testcases),
};

#else
TST_TEST_TCONF("test requires libaio and its development packages");
#endif
