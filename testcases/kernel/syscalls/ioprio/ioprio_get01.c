// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linus Walleij <linus.walleij@linaro.org>
 *
 * Description:
 * Basic ioprio_get() test. Gets the current process I/O priority and
 * checks that the values are sane.
 */

#include "tst_test.h"
#include "ioprio.h"

static void run(void)
{
	int prio, class;

	/* Get the I/O priority for the current process */
	TEST(sys_ioprio_get(IOPRIO_WHO_PROCESS, 0));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "ioprio_get failed");
		return;
	}

	class = IOPRIO_PRIO_CLASS(TST_RET);
	prio = IOPRIO_PRIO_LEVEL(TST_RET);

	if (!prio_in_range(prio)) {
		tst_res(TFAIL, "ioprio out of range (%d)", prio);
		return;
	}

	if (!class_in_range(class)) {
		tst_res(TFAIL, "ioprio class of range (%d)", class);
		return;
	}

	tst_res(TPASS, "ioprio_get returned class %s prio %d",
		to_class_str[class], prio);
}

static struct tst_test test = {
	.test_all = run,
};
