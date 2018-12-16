// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Author: Richard Logan
 *
 * Test Description:
 *  The process does a fork:
 *	1) By the value returned by child's alarm(0), check whether child
 *	   process cleared the previously specified alarm request or not.
 *	2) By the value returned by parent's alarm(0), check whether parent
 *	   process cleared the previously specified alarm request or not.
 */

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "tst_test.h"

static void verify_alarm(void)
{
	pid_t pid;

	TEST(alarm(100));

	pid = SAFE_FORK();
	if (pid == 0) {
		TEST(alarm(0));
		if (TST_RET != 0) {
			tst_res(TFAIL,
				"alarm(100), fork, alarm(0) child's "
				"alarm returned %ld", TST_RET);
		} else {
			tst_res(TPASS,
				"alarm(100), fork, alarm(0) child's "
				"alarm returned %ld", TST_RET);
		}
		exit(0);
	}

	TEST(alarm(0));
	if (TST_RET != 100) {
		tst_res(TFAIL,
			"alarm(100), fork, alarm(0) parent's "
			"alarm returned %ld", TST_RET);
	} else {
		tst_res(TPASS,
			"alarm(100), fork, alarm(0) parent's "
			"alarm returned %ld", TST_RET);
	}
}

static struct tst_test test = {
	.test_all = verify_alarm,
	.forks_child = 1,
};
