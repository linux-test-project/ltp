// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Test PR_GET_TIMERSLACK and PR_SET_TIMERSLACK of prctl(2).
 *
 * - Each thread has two associated timer slack values: a "default"
 *   value, and a "current" value. PR_SET_TIMERSLACK sets the "current"
 *   timer slack value for the calling thread.
 *
 * - When a new thread is created, the two timer slack values are made
 *   the same as the "current" value of the creating thread.
 *
 * - The maximum timer slack value is ULONG_MAX. On 32bit machines, it
 *   is a valid value(about 4s). On 64bit machines, it is about 500 years
 *   and no person will set this over 4s.  prctl return value is int, so
 *   we test themaximum value is INT_MAX.
 *
 * - we also check current value via /proc/self/timerslack_ns if it is
 *   supported.
 */

#include <sys/prctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include "lapi/syscalls.h"
#include "lapi/prctl.h"
#include "tst_test.h"

#define PROC_TIMERSLACK_PATH "/proc/self/timerslack_ns"

static void check_reset_timerslack(char *message);
static void check_get_timerslack(char *message, unsigned long value);
static void check_inherit_timerslack(char *message, unsigned long value);
static unsigned long origin_value;

static struct tcase {
	void (*func_check)();
	unsigned long setvalue;
	unsigned long expvalue;
	char message[50];
} tcases[] = {
	{check_reset_timerslack, 0, 50000, "Reset"},
	{check_get_timerslack, 1, 1, "Min"},
	{check_get_timerslack, 70000, 70000, "Middle"},
	{check_get_timerslack, INT_MAX, INT_MAX, "Max"},
	{check_inherit_timerslack, 70000, 70000, "Child process"},
};

static int proc_flag = 1;

static void check_reset_timerslack(char *message)
{
	check_get_timerslack(message, origin_value);
}

static void check_get_timerslack(char *message, unsigned long value)
{
	TEST(prctl(PR_GET_TIMERSLACK));
	if ((unsigned long)TST_RET == value)
		tst_res(TPASS, "%s prctl(PR_GET_TIMERSLACK) got %lu expectedly",
				message, value);
	else
		tst_res(TFAIL, "%s prctl(PR_GET_TIMERSLACK) expected %lu got %lu",
				message, value, TST_RET);

	if (proc_flag)
		TST_ASSERT_INT(PROC_TIMERSLACK_PATH, value);
}

static void check_inherit_timerslack(char *message, unsigned long value)
{
	int pid;
	unsigned long current_value;
	unsigned long default_value;

	pid = SAFE_FORK();
	if (pid == 0) {
		current_value = prctl(PR_GET_TIMERSLACK);
		prctl(PR_SET_TIMERSLACK, 0);
		default_value = prctl(PR_GET_TIMERSLACK);
		if (current_value == value && default_value == value)
			tst_res(TPASS,
				"%s two timer slack values are made the same as the current value(%lu) of the creating thread.",
				message, value);
		else
			tst_res(TFAIL,
				"%s current_value is %lu, default value is %lu, the parent current value is %lu",
				message, current_value, default_value, value);
	}

}

static void verify_prctl(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(prctl(PR_SET_TIMERSLACK, tc->setvalue));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "prctl(PR_SET_TIMERSLACK, %lu) failed",
					  tc->setvalue);
		return;
	}

	tst_res(TPASS, "prctl(PR_SET_TIMERSLACK, %lu) succeed", tc->setvalue);
	tc->func_check(tc->message, tc->expvalue);
}

static void setup(void)
{
	if (access(PROC_TIMERSLACK_PATH, F_OK) == -1) {
		tst_res(TCONF, "proc doesn't support timerslack_ns interface");
		proc_flag = 0;
	}

	TEST(prctl(PR_GET_TIMERSLACK));
	origin_value = TST_RET;
	tst_res(TINFO, "current timerslack value is %lu", origin_value);
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_prctl,
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
};
