// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Author: Billy Jean Horne
 *
 * Test Description:
 *  1) alarm() return UINT_MAX if seconds is UINT_MAX.
 *  2) alarm() return UINT_MAX/2 if seconds is UINT_MAX/2.
 *  3) alarm() return UINT_MAX/4 if seconds is UINT_MAX/4.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/signal.h>
#include <limits.h>

#include "tst_test.h"

static volatile int alarms_received;

static struct tcase {
	char *str;
	unsigned int sec;
} tcases[] = {
	{"INT_MAX", INT_MAX},
	{"UINT_MAX/2", UINT_MAX/2},
	{"UINT_MAX/4", UINT_MAX/4},
};

static void verify_alarm(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	unsigned int ret;

	alarms_received = 0;

	ret = alarm(tc->sec);
	if (ret != 0) {
		tst_res(TFAIL,
			"alarm(%u) returned %ld, when 0 was ",
			tc->sec, TST_RET);
		return;
	}

	TEST(alarm(0));
	if (alarms_received == 1) {
		tst_res(TFAIL,
			"alarm(%u) signal was received for value %s",
			tc->sec, tc->str);
			return;
	}

	if (tc->sec != TST_RET) {
		tst_res(TFAIL,
			"alarm(%u) returned %ld as unexpected",
			tc->sec, TST_RET);
			return;
	}

	tst_res(TPASS,
		"alarm(%u) returned %ld as expected "
		"for value %s",
		tc->sec, TST_RET, tc->str);
}

static void sighandler(int sig)
{
	if (sig == SIGALRM)
		alarms_received++;
}

static void setup(void)
{
	SAFE_SIGNAL(SIGALRM, sighandler);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_alarm,
	.setup = setup,
};
