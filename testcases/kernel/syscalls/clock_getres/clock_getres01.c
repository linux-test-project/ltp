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

#include <sys/syscall.h>
#include <sys/types.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "config.h"
#include "include_j_h.h"

#include "test.h"
#include "usctest.h"
#include "lapi/posix_clocks.h"

#define NORMAL		1
#define NULL_POINTER	0

static struct test_case {
	char *name;
	clockid_t clk_id;
	int ttype;
	int ret;
	int err;
} tcase[] = {
	{"REALTIME", CLOCK_REALTIME, NORMAL, 0, 0},
	{"MONOTONIC", CLOCK_MONOTONIC, NORMAL, 0, 0},
	{"PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID, NORMAL, 0, 0},
	{"THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID, NORMAL, 0, 0},
	{"REALTIME", CLOCK_REALTIME, NULL_POINTER, 0, 0},
	{"CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW, NORMAL, 0, 0,},
	{"CLOCK_REALTIME_COARSE", CLOCK_REALTIME_COARSE, NORMAL, 0, 0,},
	{"CLOCK_MONOTONIC_COARSE", CLOCK_MONOTONIC_COARSE, NORMAL, 0, 0,},
	{"CLOCK_BOOTTIME", CLOCK_BOOTTIME, NORMAL, 0, 0,},
	{"CLOCK_REALTIME_ALARM", CLOCK_REALTIME_ALARM, NORMAL, 0, 0,},
	{"CLOCK_BOOTTIME_ALARM", CLOCK_BOOTTIME_ALARM, NORMAL, 0, 0,},
	{"-1", -1, NORMAL, -1, EINVAL},
};

static void setup(void);
static void cleanup(void);

char *TCID = "clock_getres01";
int TST_TOTAL = ARRAY_SIZE(tcase);

int main(int ac, char **av)
{
	int i;
	int lc;
	struct timespec res;
	const char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)))
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			if (tcase[i].ttype == NULL_POINTER)
				TEST(clock_getres(tcase[i].clk_id, NULL));
			else
				TEST(clock_getres(tcase[i].clk_id, &res));

			if (TEST_RETURN != tcase[i].ret) {
				if (TEST_ERRNO != EINVAL) {
					tst_resm(TFAIL,
						 "clock_getres %s failed",
						 tcase[i].name);
				} else {
					tst_resm(TCONF,
						 "clock_getres %s NO SUPPORTED",
						 tcase[i].name);
				}
			} else {
				if (TEST_ERRNO != tcase[i].err) {
					tst_resm(TFAIL,
						 "clock_getres %s failed with "
						 "unexpect errno: %d",
						 tcase[i].name, TEST_ERRNO);
				} else {
					tst_resm(TPASS,
						 "clock_getres %s succeeded",
						 tcase[i].name);
				}
			}

		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;
}
