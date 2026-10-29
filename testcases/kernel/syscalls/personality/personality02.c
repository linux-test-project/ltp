// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test checks if select() timeout is not updated when personality with
 * STICKY_TIMEOUTS is used.
 */

#include "tst_test.h"
#include "lapi/personality.h"
#include <sys/select.h>

#define USEC 10

static void run(void)
{
	struct timeval tv = { .tv_sec = 0, .tv_usec = USEC };
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(1, &rfds);

	SAFE_PERSONALITY(PER_LINUX | STICKY_TIMEOUTS);

	TEST(select(2, &rfds, NULL, NULL, &tv));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "select() error");

	SAFE_PERSONALITY(PER_LINUX);

	TST_EXP_EQ_LI(tv.tv_usec, USEC);
}

static struct tst_test test = {
	.test_all = run,
};
