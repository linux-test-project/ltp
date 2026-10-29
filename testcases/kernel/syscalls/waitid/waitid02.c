// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Manas Kumar Nayak maknayak@in.ibm.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Tests if waitid() returns EINVAL when passed invalid options flag value.
 */

#include <sys/wait.h>
#include "tst_test.h"

static siginfo_t *infop;

static void run(void)
{
	TST_EXP_FAIL(waitid(P_ALL, 0, infop, WNOHANG), EINVAL);
}

static struct tst_test test = {
	.test_all = run,
	.bufs = (struct tst_buffers[]) {
		{&infop, .size = sizeof(*infop)},
		{}
	}
};
