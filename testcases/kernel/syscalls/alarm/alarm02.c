// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Author: Billy Jean Horne
 * Copyright (c) Linux Test Project, 2009-2022
 */

/*\
 * [Description]
 *
 * Verify that alarm() returns:
 *
 * - zero when there was no previously scheduled alarm
 * - number of seconds remaining until any previously scheduled alarm
 */

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

	alarms_received = 0;

	TST_EXP_PASS(alarm(tc->sec), "alarm(%u)", tc->sec);

	TST_EXP_VAL(alarm(0), tc->sec);

	if (alarms_received == 1) {
		tst_res(TFAIL, "alarm(%u) delivered SIGALRM for seconds value %s",
				tc->sec, tc->str);
	}
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
