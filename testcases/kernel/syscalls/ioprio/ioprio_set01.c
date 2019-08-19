// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linus Walleij <linus.walleij@linaro.org>
 *
 * Description:
 * Basic ioprio_set() test. Gets the current process I/O priority and
 * bumps it up one notch, then down two notches and checks that the
 * new priority is reported back correctly.
 */
#include <sys/types.h>
#include <sys/syscall.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "ioprio.h"

static void run(void)
{
	int class, prio;

	/* Get the I/O priority for the current process */
	TEST(sys_ioprio_get(IOPRIO_WHO_PROCESS, 0));

	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "ioprio_get failed");

	class = IOPRIO_PRIO_CLASS(TST_RET);
	prio = IOPRIO_PRIO_LEVEL(TST_RET);
	tst_res(TINFO, "ioprio_get returned class %s prio %d",
		to_class_str[class], prio);

	/* Bump prio to what it was + 1 */
	class = IOPRIO_CLASS_BE;
	if (!prio_in_range(prio + 1)) {
		tst_res(TFAIL, "ioprio increase out of range (%d)", prio + 1);
		return;
	}
	prio = (prio + 1);

	TEST(sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
			    IOPRIO_PRIO_VALUE(class, prio)));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "ioprio_set failed");
	else
		ioprio_check_setting(class, prio, 1);

	/* Bump prio down two notches */
	if (!prio_in_range(prio - 2)) {
		tst_res(TFAIL, "ioprio increase out of range (%d)", prio - 2);
		return;
	}
	prio = (prio - 2);
	TEST(sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
			    IOPRIO_PRIO_VALUE(class, prio)));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "ioprio_set failed");
	else
		ioprio_check_setting(class, prio, 1);
}

static struct tst_test test = {
	.test_all = run,
};
