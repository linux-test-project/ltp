/*
 * Copyright (C) 2013 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * errno tests for setns(2) - reassociate thread with a namespace
 */
#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>
#include <sched.h>
#include <pwd.h>
#include <string.h>
#include "config.h"
#include "test.h"
#include "lapi/syscalls.h"
#include "safe_macros.h"

char *TCID = "setns01";

#if defined(__NR_setns)
#include "setns.h"

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

static void setup(void);
static void cleanup(void);
static void setup0(struct testcase_t *, int);
static void setup1(struct testcase_t *, int);
static void setup2(struct testcase_t *, int);
static void setup3(struct testcase_t *, int);
static void setup4(struct testcase_t *, int);
static void cleanup1(struct testcase_t *);
static void cleanup4(struct testcase_t *);

struct testcase_t tdat[] = {
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
		.cleanup = cleanup1
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

static int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);
static const char nobody_uid[] = "nobody";
static struct passwd *ltpuser;

static void setup0(struct testcase_t *t, int i)
{
	t->ns_type = ns_types[i];
}

static void setup1(struct testcase_t *t, int i)
{
	t->ns_type = ns_types[i];
	t->fd = open("dummy", O_RDWR|O_CREAT, 0600);
	if (t->fd == -1)
		tst_brkm(TFAIL|TERRNO, cleanup, "setup1:open failed");
	unlink("dummy");
}

static void cleanup1(struct testcase_t *t)
{
	close(t->fd);
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
	SAFE_SETEUID(NULL, ltpuser->pw_uid);

	t->fd = ns_fds[i];
	t->ns_type = ns_types[i];
}

static void cleanup4(struct testcase_t *t)
{
	SAFE_SETEUID(NULL, 0);
}

static void test_setns(struct testcase_t *t)
{
	int ret, i;

	for (i = 0; i < ns_total; i++) {
		if (t->setup)
			t->setup(t, i);

		if (t->skip) {
			tst_resm(TINFO, "skip %s", tdat->msg);
			continue;
		}

		tst_resm(TINFO, "setns(%d, 0x%x)", t->fd, t->ns_type);
		ret = syscall(__NR_setns, t->fd, t->ns_type);
		if (ret == t->exp_ret) {
			if (ret == -1 && errno == t->exp_errno)
				tst_resm(TPASS, "%s exp_errno=%d", t->msg,
						t->exp_errno);
			else
				tst_resm(TFAIL|TERRNO, "%s exp_errno=%d",
					t->msg, t->exp_errno);
		} else {
			tst_resm(TFAIL, "%s ret=%d expected=%d", t->msg,
					ret, t->exp_ret);
		}

		if (t->cleanup)
			t->cleanup(t);
	}
}

int main(int argc, char *argv[])
{
	int lc, testno;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (testno = 0; testno < TST_TOTAL; testno++)
			test_setns(&tdat[testno]);
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	/* runtime check if syscall is supported */
	ltp_syscall(__NR_setns, -1, 0);

	init_available_ns();
	if (ns_total == 0)
		tst_brkm(TCONF, NULL, "no ns types/proc entries");

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK | TERRNO, NULL, "getpwnam failed");


	tst_tmpdir();
	TEST_PAUSE;
}

static void cleanup(void)
{
	close_ns_fds();
	tst_rmdir();
}
#else
int main(int argc, char *argv[])
{
	tst_brkm(TCONF, NULL, "__NR_setns is not defined on your system.");

}
#endif
