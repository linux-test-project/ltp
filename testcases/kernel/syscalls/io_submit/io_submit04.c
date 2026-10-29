// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Wei Gao <wegao@suse.com>
 */

/*\
 * Test RWF_NOWAIT support in io_submit(), verifying that an
 * asynchronous read operation on a blocking resource (empty pipe)
 * will cause -EAGAIN. This is done by checking that io_getevents()
 * :manpage:`io_getevents(2)` syscall returns immediately and
 * io_event.res is equal to -EAGAIN.
 */

#include "config.h"
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/aio_abi.h"

#define BUF_SIZE 100

static int fd[2] = {-1, -1};
static aio_context_t ctx;
static char *buf;
static iocb *cb;
static iocb **iocbs;

static void setup(void)
{
	if (tst_syscall(__NR_io_setup, 1, &ctx))
		tst_brk(TBROK | TERRNO, "io_setup failed");

	SAFE_PIPE(fd);

	cb->aio_fildes = fd[0];
	cb->aio_lio_opcode = IOCB_CMD_PREAD;
	cb->aio_buf = TST_PTR_TO_UINT(buf);
	cb->aio_offset = 0;
	cb->aio_nbytes = BUF_SIZE;
	cb->aio_rw_flags = RWF_NOWAIT;

	iocbs[0] = cb;
}

static void cleanup(void)
{
	if (fd[0] != -1)
		SAFE_CLOSE(fd[0]);

	if (fd[1] != -1)
		SAFE_CLOSE(fd[1]);

	if (ctx && tst_syscall(__NR_io_destroy, ctx))
		tst_brk(TBROK | TERRNO, "io_destroy() failed");
}

static void run(void)
{
	struct io_event evbuf;
	struct timespec timeout = { .tv_sec = 1 };
	long nr = 1;

	TEST(tst_syscall(__NR_io_submit, ctx, nr, iocbs));

	if (TST_RET == -1 && TST_ERR == EOPNOTSUPP) {
		tst_brk(TCONF, "RWF_NOWAIT not supported by kernel");
	} else if (TST_RET != nr) {
		tst_brk(TBROK | TTERRNO, "io_submit() returns %ld, expected %ld",
				TST_RET, nr);
	}

	TEST(tst_syscall(__NR_io_getevents, ctx, 1, 1, &evbuf, &timeout));

	if (TST_RET != 1) {
		tst_res(TFAIL | TTERRNO, "io_getevents() failed to get 1 event");
		return;
	}

	if (evbuf.res == -EAGAIN)
		tst_res(TPASS, "io_getevents() returned EAGAIN on read event");
	else
		tst_res(TFAIL, "io_getevents() returned with %s instead of EAGAIN",
			tst_strerrno(-evbuf.res));
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_AIO=y",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&buf, .size = BUF_SIZE},
		{&cb, .size = sizeof(iocb)},
		{&iocbs, .size = sizeof(iocb *)},
		{},
	}
};
