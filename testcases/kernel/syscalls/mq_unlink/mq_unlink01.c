// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 *          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Copyright (c) 2016 Linux Test Project
 */

#include <errno.h>
#include <pwd.h>
#include <mqueue.h>

#include "tst_test.h"
#include "tst_safe_posix_ipc.h"

#define QUEUE_NAME	"/test_mqueue"

static uid_t euid;
static struct passwd *pw;

struct test_case {
	int as_nobody;
	char *qname;
	int ret;
	int err;
};

static struct test_case tcase[] = {
	{
		.qname = QUEUE_NAME,
		.ret = 0,
		.err = 0,
	},
	{
		.as_nobody = 1,
		.qname = QUEUE_NAME,
		.ret = -1,
		.err = EACCES,
	},
	{
		.qname = "/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaa",
		.ret = -1,
		.err = ENOENT,
	},
	{
		.qname = "/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaa",
		.ret = -1,
		.err = ENAMETOOLONG,
	},
};

void setup(void)
{
	euid = geteuid();
	pw = SAFE_GETPWNAM("nobody");
}

static void do_test(unsigned int i)
{
	struct test_case *tc = &tcase[i];
	mqd_t fd;

	tst_res(TINFO, "queue name %s", tc->qname);

	/*
	 * When test ended with SIGTERM etc, mq descriptor is left remains.
	 * So we delete it first.
	 */
	mq_unlink(QUEUE_NAME);

	/* prepare */
	fd = SAFE_MQ_OPEN(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRWXU, NULL);

	if (tc->as_nobody && seteuid(pw->pw_uid)) {
		tst_res(TFAIL | TERRNO, "seteuid failed");
		goto EXIT;
	}

	/* test */
	TEST(mq_unlink(tc->qname));
	if (TST_ERR != tc->err || TST_RET != tc->ret) {
		tst_res(TFAIL | TTERRNO, "mq_unlink returned %ld, expected %d,"
			" expected errno %s (%d)", TST_RET,
			tc->ret, tst_strerrno(tc->err), tc->err);
	} else {
		tst_res(TPASS | TTERRNO, "mq_unlink returned %ld", TST_RET);
	}

EXIT:
	/* cleanup */
	if (tc->as_nobody && seteuid(euid) == -1)
		tst_res(TWARN | TERRNO, "seteuid back to %d failed", euid);

	if (fd > 0 && close(fd))
		tst_res(TWARN | TERRNO, "close(fd) failed");

	mq_unlink(QUEUE_NAME);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.needs_root = 1,
	.setup = setup,
};
