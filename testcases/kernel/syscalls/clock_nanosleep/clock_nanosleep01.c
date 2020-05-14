// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 *          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Copyright (c) 2016 Linux Test Project
 */

#include <limits.h>

#include "tst_safe_clocks.h"
#include "tst_sig_proc.h"
#include "tst_timer.h"

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

enum test_type {
	NORMAL,
	SEND_SIGINT,
};

#define TYPE_NAME(x) .ttype = x, .desc = #x

struct test_case {
	clockid_t clk_id;	   /* clock_* clock type parameter */
	int ttype;		   /* test type (enum) */
	const char *desc;	   /* test description (name) */
	int flags;		   /* clock_nanosleep flags parameter */
	long tv_sec;
	long tv_nsec;
	int exp_ret;
	int exp_err;
};

/*
 *   test status of errors on man page
 *   EINTR	      v (function was interrupted by a signal)
 *   EINVAL	     v (invalid tv_nsec, etc.)
 */

static struct test_case tcase[] = {
	{
		TYPE_NAME(NORMAL),
		.clk_id = CLOCK_REALTIME,
		.flags = 0,
		.tv_sec = 0,
		.tv_nsec = -1,
		.exp_ret = -1,
		.exp_err = EINVAL,
	},
	{
		TYPE_NAME(NORMAL),
		.clk_id = CLOCK_REALTIME,
		.flags = 0,
		.tv_sec = 0,
		.tv_nsec = 1000000000,
		.exp_ret = -1,
		.exp_err = EINVAL,
	},
	{
		TYPE_NAME(NORMAL),
		.clk_id = CLOCK_THREAD_CPUTIME_ID,
		.flags = 0,
		.tv_sec = 0,
		.tv_nsec = 500000000,
		.exp_ret = -1,
		.exp_err = ENOTSUP,
	},
	{
		TYPE_NAME(SEND_SIGINT),
		.clk_id = CLOCK_REALTIME,
		.flags = 0,
		.tv_sec = 10,
		.tv_nsec = 0,
		.exp_ret = -1,
		.exp_err = EINTR,
	},
};

static struct tst_ts *rq;
static struct tst_ts *rm;

static struct test_variants {
	int (*func)(clockid_t clock_id, int flags, void *request, void *remain);
	enum tst_ts_type type;
	char *desc;
} variants[] = {
	{ .func = libc_clock_nanosleep, .type = TST_LIBC_TIMESPEC, .desc = "vDSO or syscall with libc spec"},

#if (__NR_clock_nanosleep != __LTP__NR_INVALID_SYSCALL)
	{ .func = sys_clock_nanosleep, .type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_clock_nanosleep_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .func = sys_clock_nanosleep64, .type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

void setup(void)
{
	rq->type = variants[tst_variant].type;
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
	SAFE_SIGNAL(SIGINT, sighandler);
}

static void do_test(unsigned int i)
{
	struct test_variants *tv = &variants[tst_variant];
	struct test_case *tc = &tcase[i];
	pid_t pid = 0;

	memset(rm, 0, sizeof(*rm));
	rm->type = rq->type;

	tst_res(TINFO, "case %s", tc->desc);

	if (tc->ttype == SEND_SIGINT)
		pid = create_sig_proc(SIGINT, 40, 500000);

	tst_ts_set_sec(rq, tc->tv_sec);
	tst_ts_set_nsec(rq, tc->tv_nsec);

	TEST(tv->func(tc->clk_id, tc->flags, tst_ts_get(rq), tst_ts_get(rm)));

	if (tv->func == libc_clock_nanosleep) {
		/*
		 * The return value and error number are differently set for
		 * libc syscall as compared to kernel syscall.
		 */
		if (TST_RET) {
			TST_ERR = TST_RET;
			TST_RET = -1;
		}

		/*
		 * nsleep isn't implemented by kernelf or
		 * CLOCK_THREAD_CPUTIME_ID and it returns ENOTSUP, but libc
		 * changes that error value to EINVAL.
		 */
		if (tc->clk_id == CLOCK_THREAD_CPUTIME_ID)
			tc->exp_err = EINVAL;
	}

	if (pid) {
		SAFE_KILL(pid, SIGTERM);
		SAFE_WAIT(NULL);
	}

	if (tc->ttype == SEND_SIGINT) {
		long long expect_ms = tst_ts_to_ms(*rq);
		long long remain_ms = tst_ts_to_ms(*rm);

		if (tst_ts_valid(rm)) {
			tst_res(TFAIL | TTERRNO,
				"The clock_nanosleep() haven't updated"
				" timespec or it's not valid");
			return;
		}

		if (remain_ms > expect_ms) {
			tst_res(TFAIL| TTERRNO,
				"remaining time > requested time (%lld > %lld)",
				remain_ms, expect_ms);
			return;
		}

		tst_res(TPASS, "Timespec updated correctly");
	}

	if ((TST_RET != tc->exp_ret) || (TST_ERR != tc->exp_err)) {
		tst_res(TFAIL | TTERRNO, "returned %ld, expected %d,"
			" expected errno: %s (%d)", TST_RET,
			tc->exp_ret, tst_strerrno(tc->exp_err), tc->exp_err);
		return;
	}

	tst_res(TPASS | TERRNO, "clock_nanosleep() failed with");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.forks_child = 1,
	.bufs = (struct tst_buffers []) {
		{&rq, .size = sizeof(*rq)},
		{&rm, .size = sizeof(*rm)},
		{},
	}
};
