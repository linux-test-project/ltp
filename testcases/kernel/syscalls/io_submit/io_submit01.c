// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Ported from Crackerjack to LTP by Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2011-2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Test io_submit() invoked via libaio:
 *
 * - io_submit fails and returns -EINVAL if ctx is invalid.
 * - io_submit fails and returns -EINVAL if nr is invalid.
 * - io_submit fails and returns -EFAULT if iocbpp pointer is invalid.
 * - io_submit fails and returns -EBADF if fd is invalid.
 * - io_submit succeeds and returns the number of iocbs submitted.
 * - io_submit succeeds and returns 0 if nr is zero.
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LIBAIO
#include <libaio.h>

static io_context_t ctx;
static io_context_t invalid_ctx;

static struct iocb iocb;
static struct iocb *iocbs[] = {&iocb};

static struct iocb inv_fd_iocb;
static struct iocb *inv_fd_iocbs[] = {&inv_fd_iocb};

static int rdonly_fd;
static struct iocb rdonly_fd_iocb;
static struct iocb *rdonly_fd_iocbs[] = {&rdonly_fd_iocb};

static int wronly_fd;
static struct iocb wronly_fd_iocb;
static struct iocb *wronly_fd_iocbs[] = {&wronly_fd_iocb};

static struct iocb zero_buf_iocb;
static struct iocb *zero_buf_iocbs[] = {&zero_buf_iocb};

static struct iocb *zero_iocbs[1];

static char buf[100];

static struct tcase {
	io_context_t *ctx;
	long nr;
	struct iocb **iocbs;
	int exp_errno;
	const char *desc;
} tcases[] = {
	/* Invalid ctx */
	{&invalid_ctx, 1, iocbs, -EINVAL, "invalid ctx"},
	/* Invalid nr */
	{&ctx, -1, iocbs, -EINVAL, "invalid nr"},
	/* Invalid pointer */
	{&ctx, 1, (void*)-1, -EFAULT, "invalid iocbpp pointer"},
	{&ctx, 1, zero_iocbs, -EFAULT, "NULL iocb pointers"},
	/* Invalid fd */
	{&ctx, 1, inv_fd_iocbs, -EBADF, "invalid fd"},
	{&ctx, 1, rdonly_fd_iocbs, -EBADF, "readonly fd for write"},
	{&ctx, 1, wronly_fd_iocbs, -EBADF, "writeonly fd for read"},
	/* No-op but should work fine */
	{&ctx, 1, zero_buf_iocbs, 1, "zero buf size"},
	{&ctx, 0, NULL, 0, "zero nr"},
};

static void setup(void)
{
	TEST(io_setup(1, &ctx));
	if (TST_RET == -ENOSYS)
		tst_brk(TCONF | TRERRNO, "io_setup(): AIO not supported by kernel");
	else if (TST_RET)
		tst_brk(TBROK | TRERRNO, "io_setup() failed");

	io_prep_pread(&inv_fd_iocb, -1, buf, sizeof(buf), 0);

	rdonly_fd = SAFE_OPEN("rdonly_file", O_RDONLY | O_CREAT, 0777);
	io_prep_pwrite(&rdonly_fd_iocb, rdonly_fd, buf, sizeof(buf), 0);

	io_prep_pread(&zero_buf_iocb, rdonly_fd, buf, 0, 0);

	wronly_fd = SAFE_OPEN("wronly_file", O_WRONLY | O_CREAT, 0777);
	io_prep_pread(&wronly_fd_iocb, wronly_fd, buf, sizeof(buf), 0);
}

static void cleanup(void)
{
	if (rdonly_fd > 0)
		SAFE_CLOSE(rdonly_fd);

	if (wronly_fd > 0)
		SAFE_CLOSE(wronly_fd);
}

static const char *errno_name(int err)
{
	if (err <= 0)
		return tst_strerrno(-err);

	return "SUCCESS";
}

static void verify_io_submit(unsigned int n)
{
	struct tcase *t = &tcases[n];
	struct io_event evbuf;
	struct timespec timeout = { .tv_sec = 1 };
	int i, ret;

	ret = io_submit(*t->ctx, t->nr, t->iocbs);

	if (ret == t->exp_errno) {
		tst_res(TPASS, "io_submit() with %s failed with %s",
			t->desc, errno_name(t->exp_errno));

		for (i = 0; i < ret; i++)
			io_getevents(*t->ctx, 1, 1, &evbuf, &timeout);

		return;
	}

	tst_res(TFAIL, "io_submit() returned %i(%s), expected %s(%i)",
		ret, ret < 0 ? tst_strerrno(-ret) : "SUCCESS",
		errno_name(t->exp_errno), t->exp_errno);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_io_submit,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
};

#else
	TST_TEST_TCONF("test requires libaio and it's development packages");
#endif
