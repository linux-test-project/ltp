/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 *          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <limits.h>

#include "linux_syscall_numbers.h"
#include "tst_sig_proc.h"
#include "tst_timer.h"
#include "tst_test.h"

#define MAX_MSEC_DIFF   20

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
	struct timespec rq;
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
		.clk_id = CLOCK_REALTIME,
		TYPE_NAME(NORMAL),
		.flags = 0,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = 500000000},
		.exp_ret = 0,
		.exp_err = 0,
	},
	{
		.clk_id = CLOCK_MONOTONIC,
		TYPE_NAME(NORMAL),
		.flags = 0,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = 500000000},
		.exp_ret = 0,
		.exp_err = 0,
	},
	{
		TYPE_NAME(NORMAL),
		.clk_id = CLOCK_REALTIME,
		.flags = 0,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = -1},
		.exp_ret = EINVAL,
		.exp_err = 0,
	},
	{
		TYPE_NAME(NORMAL),
		.clk_id = CLOCK_REALTIME,
		.flags = 0,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = 1000000000},
		.exp_ret = EINVAL,
		.exp_err = 0,
	},
	{
		TYPE_NAME(NORMAL),
		.clk_id = CLOCK_THREAD_CPUTIME_ID,
		.flags = 0,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = 500000000},
		.exp_ret = EINVAL,
		.exp_err = 0,
	},
	{
		TYPE_NAME(SEND_SIGINT),
		.clk_id = CLOCK_REALTIME,
		.flags = 0,
		.rq = (struct timespec) {.tv_sec = 10, .tv_nsec = 0},
		.exp_ret = EINTR,
		.exp_err = 0,
	},
};

void setup(void)
{
	SAFE_SIGNAL(SIGINT, sighandler);
	tst_timer_check(CLOCK_MONOTONIC);
}

static void do_test(unsigned int i)
{
	struct test_case *tc = &tcase[i];
	struct timespec rm = {0};
	long long elapsed_ms, expect_ms, remain_ms = 0;
	pid_t pid = 0;

	tst_res(TINFO, "case %s", tc->desc);

	/* setup */
	if (tc->ttype == SEND_SIGINT)
		pid = create_sig_proc(SIGINT, 40, 500000);

	/* test */
	tst_timer_start(CLOCK_MONOTONIC);
	TEST(clock_nanosleep(tc->clk_id, tc->flags, &tc->rq, &rm));
	tst_timer_stop();
	elapsed_ms = tst_timer_elapsed_ms();
	expect_ms = tst_timespec_to_ms(tc->rq);

	if (tc->ttype == SEND_SIGINT) {
		tst_res(TINFO, "remain time: %lds %ldns", rm.tv_sec, rm.tv_nsec);
		remain_ms = tst_timespec_to_ms(rm);
	}

	/* cleanup */
	if (pid) {
		SAFE_KILL(pid, SIGTERM);
		SAFE_WAIT(NULL);
	}

	/* result check */
	if (!TEST_RETURN && (elapsed_ms < expect_ms - MAX_MSEC_DIFF
		|| elapsed_ms > expect_ms + MAX_MSEC_DIFF)) {

		tst_res(TFAIL| TTERRNO, "The clock_nanosleep() haven't slept correctly,"
			" measured %lldms, expected %lldms +- %d",
			elapsed_ms, expect_ms, MAX_MSEC_DIFF);
		return;
	}

	if (tc->ttype == SEND_SIGINT && !rm.tv_sec && !rm.tv_nsec) {
		tst_res(TFAIL | TTERRNO, "The clock_nanosleep() haven't updated"
			" timestamp with remaining time");
		return;
	}

	if (tc->ttype == SEND_SIGINT && remain_ms > expect_ms) {
		tst_res(TFAIL| TTERRNO, "remaining time > requested time (%lld > %lld)",
			remain_ms, expect_ms);
		return;
	}

	if (TEST_RETURN != tc->exp_ret) {
		tst_res(TFAIL | TTERRNO, "returned %ld, expected %d,"
			" expected errno: %s (%d)", TEST_RETURN,
			tc->exp_ret, tst_strerrno(tc->exp_err), tc->exp_err);
		return;
	}

	tst_res(TPASS, "returned %s (%ld)",
		tst_strerrno(TEST_RETURN), TEST_RETURN);
}

static struct tst_test test = {
	.tid = "clock_nanosleep01",
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.setup = setup,
	.forks_child = 1,
};
