// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 *          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Copyright (c) 2016 Linux Test Project
 */

#include <errno.h>
#include <mqueue.h>
#include <pwd.h>

#include "tst_test.h"
#include "tst_safe_file_ops.h"
#include "tst_safe_posix_ipc.h"

#define QUEUE_NAME	"/test_mqueue"
#define QUEUE_INIT	"/init_mqueue"

static uid_t euid;
static struct passwd *pw;
static char *qname;
static struct rlimit rlim;

static mqd_t fd, fd2;
static mqd_t fd3 = -1;
static int max_queues;

struct test_case {
	const char *desc;
	char *qname;
	int oflag;
	struct mq_attr *rq;
	int ret;
	int err;
	void (*setup)(void);
	void (*cleanup)(void);
};

#define PROC_MAX_QUEUES "/proc/sys/fs/mqueue/queues_max"

static void create_queue(void);
static void unlink_queue(void);
static void set_rlimit(void);
static void restore_rlimit(void);
static void set_max_queues(void);
static void restore_max_queues(void);

static struct test_case tcase[] = {
	{
		.desc = "NORMAL",
		.qname = QUEUE_NAME,
		.oflag = O_CREAT,
		.rq = &(struct mq_attr){.mq_maxmsg = 20, .mq_msgsize = 16384},
		.ret = 0,
		.err = 0,
	},
	{
		.desc = "NORMAL",
		.qname = QUEUE_NAME,
		.oflag = O_CREAT,
		.ret = 0,
		.err = 0,
	},
	{
		.desc = "NORMAL",
		.qname = "/caaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaa",
		.oflag = O_CREAT,
		.ret = 0,
		.err = 0,
	},
	{
		.desc = "NORMAL",
		.qname = "/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaa",
		.oflag = O_CREAT,
		.ret = -1,
		.err = ENAMETOOLONG,
	},

	{
		.desc = "NORMAL",
		.qname = "",
		.oflag = O_CREAT,
		.ret = -1,
		.err = EINVAL,
	},
	{
		.desc = "NORMAL",
		.qname = QUEUE_NAME,
		.ret = -1,
		.err = EACCES,
		.setup = create_queue,
		.cleanup = unlink_queue,
	},
	{
		.desc = "NORMAL",
		.qname = QUEUE_NAME,
		.oflag = O_CREAT | O_EXCL,
		.ret = -1,
		.err = EEXIST,
		.setup = create_queue,
		.cleanup = unlink_queue,
	},
	{
		.desc = "NO_FILE",
		.qname = QUEUE_NAME,
		.oflag = O_CREAT,
		.ret = -1,
		.err = EMFILE,
		.setup = set_rlimit,
		.cleanup = restore_rlimit,
	},
	{
		.desc = "NORMAL",
		.qname = "/notexist",
		.oflag = 0,
		.ret = -1,
		.err = ENOENT,
	},
	{
		.desc = "NO_SPACE",
		.qname = QUEUE_NAME,
		.oflag = O_CREAT,
		.ret = -1,
		.err = ENOSPC,
		.setup = set_max_queues,
		.cleanup = restore_max_queues,
	}
};

static void create_queue(void)
{
	fd2 = SAFE_MQ_OPEN(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRWXU, NULL);

	SAFE_SETEUID(pw->pw_uid);
}

static void unlink_queue(void)
{
	SAFE_SETEUID(euid);
	if (fd2 > 0)
		SAFE_CLOSE(fd2);

	if (mq_unlink(QUEUE_NAME))
		tst_brk(TBROK | TERRNO, "mq_close(" QUEUE_NAME ") failed");
}


static void set_max_queues(void)
{
	SAFE_FILE_SCANF(PROC_MAX_QUEUES, "%d", &max_queues);
	SAFE_FILE_PRINTF(PROC_MAX_QUEUES, "%d", 1);

	SAFE_SETEUID(pw->pw_uid);
}

static void restore_max_queues(void)
{
	SAFE_SETEUID(euid);

	SAFE_FILE_PRINTF(PROC_MAX_QUEUES, "%d", max_queues);
}

static void set_rlimit(void)
{
	if (rlim.rlim_cur > 0) {
		struct rlimit r;
		r.rlim_cur = 0;
		r.rlim_max = rlim.rlim_max;
		SAFE_SETRLIMIT(RLIMIT_NOFILE, &r);
	}
}

static void restore_rlimit(void)
{
	SAFE_SETRLIMIT(RLIMIT_NOFILE, &rlim);
}

static void setup(void)
{
	euid = geteuid();
	pw = SAFE_GETPWNAM("nobody");
	SAFE_GETRLIMIT(RLIMIT_NOFILE, &rlim);

	fd3 = SAFE_MQ_OPEN(QUEUE_INIT, O_CREAT | O_EXCL | O_RDWR, S_IRWXU, NULL);
}

static void cleanup(void)
{
	if (fd > 0)
		mq_close(fd);

	if (fd2 > 0)
		mq_close(fd2);

	if (fd3 > 0 && mq_close(fd3))
		tst_res(TWARN | TERRNO, "mq_close(%s) failed", QUEUE_INIT);

	if (mq_unlink(QUEUE_INIT))
		tst_res(TWARN | TERRNO, "mq_unlink(%s) failed", QUEUE_INIT);

	mq_unlink(qname);
}

static void do_test(unsigned int i)
{
	struct test_case *tc = &tcase[i];
	struct mq_attr oldattr;

	qname = tc->qname;
	fd = fd2 = -1;

	tst_res(TINFO, "queue name \"%s\"", qname);

	if (tc->setup)
		tc->setup();

	TEST(fd = mq_open(qname, tc->oflag, S_IRWXU, tc->rq));

	if (fd > 0 && tc->rq) {
		if (mq_getattr(fd, &oldattr) < 0) {
			tst_res(TFAIL | TERRNO, "mq_getattr failed");
			goto CLEANUP;
		}

		if (oldattr.mq_maxmsg != tc->rq->mq_maxmsg
			|| oldattr.mq_msgsize != tc->rq->mq_msgsize) {
			tst_res(TFAIL, "wrong mq_attr: "
				"mq_maxmsg expected %ld return %ld, "
				"mq_msgsize expected %ld return %ld",
				tc->rq->mq_maxmsg, oldattr.mq_maxmsg, tc->rq->mq_msgsize,
				oldattr.mq_msgsize);
			goto CLEANUP;
		}
	}

	if (tc->ret == 0) {
		if (TST_RET < 0) {
			tst_res(TFAIL | TTERRNO, "%s wrong return code: %ld",
				tc->desc, TST_RET);
		} else {
			tst_res(TPASS | TTERRNO, "%s returned: %ld",
				tc->desc, TST_RET);
		}

		goto CLEANUP;
	}

	if (TST_ERR != tc->err) {
		tst_res(TFAIL | TTERRNO, "%s expected errno: %d",
			tc->desc, TST_ERR);
		goto CLEANUP;
	}

	if (TST_RET != tc->ret) {
		tst_res(TFAIL | TTERRNO, "%s wrong return code: %ld",
			tc->desc, TST_RET);
	} else {
		tst_res(TPASS | TTERRNO, "%s returned: %ld",
			tc->desc, TST_RET);
	}

CLEANUP:
	if (tc->cleanup)
		tc->cleanup();

	if (TST_RET != -1) {
		if (fd > 0)
			SAFE_CLOSE(fd);
		mq_unlink(qname);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
};
