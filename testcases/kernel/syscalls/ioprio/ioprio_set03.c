// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linus Walleij <linus.walleij@linaro.org>
 * Copyright (c) 2023 Linux Test Project
 */

/*\
 * Negative ioprio_set() test. Test some non-working priorities to make
 * sure they don't work.
 */

#include "tst_test.h"
#include "ioprio.h"

static void run(void)
{
	int class;

	class = IOPRIO_CLASS_BE;

	/*
	 * Test to fail with prio 8, first set prio 4 so we know what it
	 * should still be after failure, i.e. we check that the priority
	 * didn't change as a side effect of setting an invalid priority.
	 */
	sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
		       IOPRIO_PRIO_VALUE(class, 4));
	TEST(sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
			    IOPRIO_PRIO_VALUE(class, IOPRIO_PRIO_NUM)));
	if (TST_RET == -1) {
		ioprio_check_setting(class, 4, 1);
		if (errno == EINVAL)
			tst_res(TPASS | TTERRNO, "returned correct error for wrong prio");
		else
			tst_res(TFAIL, "ioprio_set returns wrong errno %d",
				TST_ERR);
	} else {
		tst_res(TFAIL, "ioprio_set IOPRIO_CLASS_BE prio 8 should not work");
	}

	/* Any other prio than 0 should not work with NONE */
	class = IOPRIO_CLASS_NONE;
	TEST(sys_ioprio_set(IOPRIO_WHO_PROCESS, 0,
			    IOPRIO_PRIO_VALUE(class, 4)));
	if (TST_RET == -1) {
		tst_res(TINFO, "tested illegal priority with class %s",
			to_class_str[class]);
		if (errno == EINVAL)
			tst_res(TPASS | TTERRNO, "returned correct error for wrong prio");
		else
			tst_res(TFAIL, "ioprio_set returns wrong errno %d",
				TST_ERR);
	} else {
		tst_res(TFAIL, "ioprio_set IOPRIO_CLASS_NONE should fail");
	}
}

static struct tst_test test = {
	.test_all = run,
};
