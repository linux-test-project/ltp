// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Ported from Crackerjack to LTP by Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2011-2017 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Test io_submit invoked via syscall(2):
 *
 * 1. io_submit() returns the number of iocbs submitted.
 * 2. io_submit() returns 0 if nr is zero.
 */

#include <linux/aio_abi.h>

#include "config.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

#define TEST_FILE "test_file"
#define MODE 0777

static int fd;
static char buf[100];

static aio_context_t ctx;
static struct iocb iocb;
static struct iocb *iocbs[] = {&iocb};

static struct tcase {
	aio_context_t *ctx;
	long nr;
	struct iocb **iocbs;
	const char *desc;
} tc[] = {
	{&ctx, 1, iocbs, "returns the number of iocbs submitted"},
	{&ctx, 0, NULL, "returns 0 if nr is zero"},
};

static inline void io_prep_option(struct iocb *cb, int fd, void *buf,
			size_t count, long long offset, unsigned opcode)
{
	memset(cb, 0, sizeof(*cb));
	cb->aio_fildes = fd;
	cb->aio_lio_opcode = opcode;
	cb->aio_buf = (uint64_t)buf;
	cb->aio_offset = offset;
	cb->aio_nbytes = count;
}

static void setup(void)
{
	TST_EXP_PASS_SILENT(tst_syscall(__NR_io_setup, 1, &ctx));
	fd = SAFE_OPEN(TEST_FILE, O_RDONLY | O_CREAT, MODE);
	io_prep_option(&iocb, fd, buf, 0, 0, IOCB_CMD_PREAD);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (tst_syscall(__NR_io_destroy, ctx))
		tst_brk(TBROK | TERRNO, "io_destroy() failed");
}

static void run(unsigned int i)
{
	struct io_event evbuf;
	struct timespec timeout = { .tv_sec = 1 };
	long j;

	TEST(tst_syscall(__NR_io_submit, *tc[i].ctx, tc[i].nr, tc[i].iocbs));

	if (TST_RET == tc[i].nr)
		tst_res(TPASS, "io_submit() %s", tc[i].desc);
	else
		tst_res(TFAIL | TTERRNO, "io_submit() returns %ld, expected %ld", TST_RET, tc[i].nr);

	for (j = 0; j < TST_RET; j++) {
		tst_syscall(__NR_io_getevents, *tc[i].ctx, 1, 1, &evbuf,
			&timeout);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.needs_tmpdir = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_AIO=y",
		NULL
	},
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
};
