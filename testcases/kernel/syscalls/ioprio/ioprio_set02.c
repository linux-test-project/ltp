// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linus Walleij <linus.walleij@linaro.org>
 * Copyright (c) 2023 Linux Test Project
 */

/*\
 * Extended ioprio_set() test.
 * Tests to set all 8 priority levels for best effort priority, then
 * switches to test all 8 priority levels for idle priority.
 */

#include "tst_test.h"
#include "ioprio.h"

static void run(void)
{
	int class, prio;
	int fail_in_loop;

	/* Bump to best effort scheduling, all 8 priorities */
	class = IOPRIO_CLASS_BE;

	fail_in_loop = 0;
	for (prio = 0; prio < IOPRIO_PRIO_NUM; prio++) {
		TEST(sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
				    IOPRIO_PRIO_VALUE(class, prio)));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO, "ioprio_set IOPRIO_CLASS_BE prio %d failed", prio);
			fail_in_loop = 1;
		}
	}
	if (!fail_in_loop)
		tst_res(TPASS, "tested all prios in class %s",
			 to_class_str[class]);

	/* Bump down to idle scheduling */
	class = IOPRIO_CLASS_IDLE;

	fail_in_loop = 0;
	for (prio = 0; prio < IOPRIO_PRIO_NUM; prio++) {
		TEST(sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
				    IOPRIO_PRIO_VALUE(class, prio)));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO, "ioprio_set IOPRIO_CLASS_IDLE prio %d failed", prio);
			fail_in_loop = 1;
		}
	}
	if (!fail_in_loop)
		tst_res(TPASS, "tested all prios in class %s",
			 to_class_str[class]);

	/* Test NONE scheduling */
	class = IOPRIO_CLASS_NONE;
	TEST(sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
			    IOPRIO_PRIO_VALUE(class, 0)));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "ioprio_set IOPRIO_CLASS_NONE failed");
	else
		ioprio_check_setting(class, 0, 1);
}

static struct tst_test test = {
	.test_all = run,
};
