// SPDX-License-Identifier: GPL-2.0-or-later
/*

  Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>

 */
/*

  Basic test for timens_offsets error handling.

  After a call to unshare(CLONE_NEWTIME) a new timer namespace is created, the
  process that has called the unshare() can adjust offsets for CLOCK_MONOTONIC
  and CLOCK_BOOTTIME for its children by writing to the '/proc/self/timens_offsets'.

 */

#define _GNU_SOURCE
#include "lapi/posix_clocks.h"
#include "tst_test.h"
#include "lapi/sched.h"

static struct tcase {
	const char *desc;
	const char *offsets;
	int exp_err;
} tcases[] = {
	{"Obvious garbage", "not an offset", EINVAL},
	{"Missing nanoseconds", "1 10", EINVAL},
	{"Negative nanoseconds", "1 10 -10", EINVAL},
	{"Nanoseconds > 1s", "1 10 1000000001", EINVAL},
	{"Unsupported CLOCK_REALTIME", "0 10 0", EINVAL},
	{"Mess on the second line", "1 10 0\na", EINVAL},
	{"Overflow kernel 64bit ns timer", "1 9223372036 0", ERANGE},
	{"Overflow kernel 64bit ns timer", "1 -9223372036 0", ERANGE},
};

static void verify_ns_clock(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fd, ret;

	SAFE_UNSHARE(CLONE_NEWTIME);

	fd = SAFE_OPEN("/proc/self/timens_offsets", O_WRONLY);
	ret = write(fd, tc->offsets, strlen(tc->offsets));

	if (ret != -1) {
		tst_res(TFAIL, "%s returned %i", tc->desc, ret);
		return;
	}

	if (errno != tc->exp_err) {
		tst_res(TFAIL | TERRNO, "%s should fail with %s, got:",
			tc->desc, tst_strerrno(tc->exp_err));
		return;
	}

	tst_res(TPASS | TERRNO, "%s", tc->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_ns_clock,
	.needs_root = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TIME_NS=y",
		NULL
	}
};
