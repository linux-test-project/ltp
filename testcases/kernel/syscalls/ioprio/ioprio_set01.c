// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linus Walleij <linus.walleij@linaro.org>
 *
 * Description:
 * Basic ioprio_set() test. Gets the current process I/O priority and
 * bumps it up one notch, then down two notches and checks that the
 * new priority is reported back correctly.
 */

#include "tst_test.h"
#include "ioprio.h"

static int orig_class;
static int orig_prio;

static void run(void)
{
	int class = orig_class, prio = orig_prio;

	/* Bump prio to what it was + 1 */
	class = IOPRIO_CLASS_BE;
	prio = prio + 1;
	if (!prio_in_range(prio)) {
		tst_res(TCONF, "ioprio increase out of range (%d)", prio);
		goto second;
	}

	TEST(sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
			    IOPRIO_PRIO_VALUE(class, prio)));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "ioprio_set failed");
	else
		ioprio_check_setting(class, prio, 1);

second:
	/* Bump prio down two notches */
	prio = prio - 2;
	if (!prio_in_range(prio)) {
		tst_res(TCONF, "ioprio decrease out of range (%d)", prio);
		return;
	}

	TEST(sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
			    IOPRIO_PRIO_VALUE(class, prio)));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "ioprio_set failed");
	else
		ioprio_check_setting(class, prio, 1);
}

static void setup(void)
{
	/* Get the I/O priority for the current process */
	TEST(sys_ioprio_get(IOPRIO_WHO_PROCESS, 0));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "ioprio_get failed");

	orig_class = IOPRIO_PRIO_CLASS(TST_RET);
	orig_prio = IOPRIO_PRIO_LEVEL(TST_RET);

	tst_res(TINFO, "ioprio_get returned class %s prio %d",
		to_class_str[orig_class], orig_prio);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};
