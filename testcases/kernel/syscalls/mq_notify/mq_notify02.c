// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 *   Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that mq_notify() fails with EINVAL when invalid input
 * arguments are given.
 */

#include <mqueue.h>
#include "tst_test.h"

static struct test_case_t {
	struct sigevent sevp;
	int exp_errno;
} tcase[] = {
	{{.sigev_notify = -1}, EINVAL},
	{{.sigev_notify = SIGEV_SIGNAL, .sigev_signo = _NSIG + 1}, EINVAL},
};

static void run(unsigned int i)
{
	struct test_case_t *test = &tcase[i];

	TST_EXP_FAIL(mq_notify(0, &(test->sevp)), test->exp_errno);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = run,
};
