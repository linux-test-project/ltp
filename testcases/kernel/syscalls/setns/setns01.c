// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2014-2020
 *
 * errno tests for setns(2) - reassociate thread with a namespace
 */
#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sched.h>
#include <pwd.h>
#include <string.h>
#include "config.h"
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "setns.h"

static const char nobody_uid[] = "nobody";
static struct passwd *ltpuser;
static int regular_fd;

struct testcase_t {
	const char *msg;
	int fd;
	int ns_type;
	int exp_ret;
	int exp_errno;
	int skip;
	void (*setup) (struct testcase_t *, int i);
	void (*cleanup) (struct testcase_t *);
};

static void setup0(struct testcase_t *t, int i)
{
	t->ns_type = ns_types[i];
}

static void setup1(struct testcase_t *t, int i)
{
	t->ns_type = ns_types[i];
	t->fd = regular_fd;
}

static void setup2(struct testcase_t *t, int i)
{
	t->fd = ns_fds[i];
}

static void setup3(struct testcase_t *t, int i)
{
	if (ns_total < 2) {
		t->skip = 1;
		return;
	}

	t->fd = ns_fds[i];
	t->ns_type = ns_types[(i+1) % ns_total];
}

static void setup4(struct testcase_t *t, int i)
{
	SAFE_SETEUID(ltpuser->pw_uid);

	t->fd = ns_fds[i];
	t->ns_type = ns_types[i];
}

static void cleanup4(LTP_ATTRIBUTE_UNUSED struct testcase_t *t)
{
	SAFE_SETEUID(0);
}

static struct testcase_t tcases[] = {
	{
		.msg = "invalid fd",
		.fd = -1,
		.exp_ret = -1,
		.exp_errno = EBADF,
		.setup = setup0,
	},
	{
		.msg = "regular file fd",
		.exp_ret = -1,
		.exp_errno = EINVAL,
		.setup = setup1,
	},
	{
		.msg = "invalid ns_type",
		.ns_type = -1,
		.exp_ret = -1,
		.exp_errno = EINVAL,
		.setup = setup2,
	},
	{
		.msg = "mismatch ns_type/fd",
		.exp_ret = -1,
		.exp_errno = EINVAL,
		.setup = setup3,
	},
	{
		.msg = "without CAP_SYS_ADMIN",
		.exp_ret = -1,
		.exp_errno = EPERM,
		.setup = setup4,
		.cleanup = cleanup4,
	}
};

static void test_setns(unsigned int testno)
{
	int ret, i;
	struct testcase_t *t = &tcases[testno];

	for (i = 0; i < ns_total; i++) {
		if (t->setup)
			t->setup(t, i);

		if (t->skip) {
			tst_res(TINFO, "skip %s", t->msg);
			continue;
		}

		tst_res(TINFO, "setns(%d, 0x%x)", t->fd, t->ns_type);
		ret = tst_syscall(__NR_setns, t->fd, t->ns_type);
		if (ret == t->exp_ret) {
			if (ret == -1 && errno == t->exp_errno) {
				tst_res(TPASS, "%s exp_errno=%d (%s)",
					t->msg,	t->exp_errno,
					tst_strerrno(t->exp_errno));
			} else {
				tst_res(TFAIL|TERRNO, "%s exp_errno=%d (%s)",
					t->msg, t->exp_errno,
					tst_strerrno(t->exp_errno));
			}
		} else {
			tst_res(TFAIL, "%s ret=%d expected=%d",
				t->msg,	ret, t->exp_ret);
		}

		if (t->cleanup)
			t->cleanup(t);
	}
}

static void setup(void)
{
	/* runtime check if syscall is supported */
	tst_syscall(__NR_setns, -1, 0);

	init_available_ns();
	if (ns_total == 0)
		tst_brk(TCONF, "no ns types/proc entries");

	ltpuser = SAFE_GETPWNAM(nobody_uid);
	regular_fd = SAFE_OPEN("dummy", O_RDWR|O_CREAT, 0600);
	SAFE_UNLINK("dummy");
}

static void cleanup(void)
{
	close_ns_fds();
	if (regular_fd)
		SAFE_CLOSE(regular_fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = test_setns,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
};

