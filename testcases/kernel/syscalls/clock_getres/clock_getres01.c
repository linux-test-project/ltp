/******************************************************************************
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd                *
 *   Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,               *
 *              Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,                 *
 *              Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>                  *
 *   LTP authors:                                                             *
 *              Manas Kumar Nayak maknayak@in.ibm.com>                        *
 *              Zeng Linggang <zenglg.jy@cn.fujitsu.com>                      *
 *              Cyril Hrubis <chrubis@suse.cz>                                *
 *                                                                            *
 * This program is free software;  you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  *
 * the GNU General Public License for more details.                           *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program;  if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           *
 *                                                                            *
 ******************************************************************************/
/*
 * Description: This tests the clock_getres() syscall
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/posix_clocks.h"

static struct timespec res;

static struct test_case {
	char *name;
	clockid_t clk_id;
	struct timespec *res;
	int ret;
	int err;
} tcase[] = {
	{"REALTIME", CLOCK_REALTIME, &res, 0, 0},
	{"MONOTONIC", CLOCK_MONOTONIC, &res, 0, 0},
	{"PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID, &res, 0, 0},
	{"THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID, &res, 0, 0},
	{"REALTIME", CLOCK_REALTIME, NULL, 0, 0},
	{"CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW, &res, 0, 0,},
	{"CLOCK_REALTIME_COARSE", CLOCK_REALTIME_COARSE, &res, 0, 0,},
	{"CLOCK_MONOTONIC_COARSE", CLOCK_MONOTONIC_COARSE, &res, 0, 0,},
	{"CLOCK_BOOTTIME", CLOCK_BOOTTIME, &res, 0, 0,},
	{"CLOCK_REALTIME_ALARM", CLOCK_REALTIME_ALARM, &res, 0, 0,},
	{"CLOCK_BOOTTIME_ALARM", CLOCK_BOOTTIME_ALARM, &res, 0, 0,},
	{"-1", -1, &res, -1, EINVAL},
};

static void do_test(unsigned int i)
{
	TEST(clock_getres(tcase[i].clk_id, tcase[i].res));

	if (TEST_RETURN != tcase[i].ret) {
		if (TEST_ERRNO == EINVAL) {
			tst_res(TCONF, "clock_getres(%s, ...) NO SUPPORTED", tcase[i].name);
			return;
		}

		tst_res(TFAIL | TTERRNO, "clock_getres(%s, ...) failed", tcase[i].name);
		return;
	}

	if (TEST_ERRNO != tcase[i].err) {
		tst_res(TFAIL,
			"clock_getres(%s, ...) failed unexpectedly: %s, expected: %s",
			tcase[i].name, tst_strerrno(TEST_ERRNO), tst_strerrno(tcase[i].err));
		return;
	}

	tst_res(TPASS, "clock_getres(%s, ...) succeeded", tcase[i].name);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(tcase),
};
