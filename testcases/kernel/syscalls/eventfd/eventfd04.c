// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
 * Copyright (c) Linux Test Project, 2008-2022
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test whether writefd is set by select() when eventfd() counter value is
 * not the maximum value, then check if writefd is not set when eventfd()
 * counter value is maximum value.
 */

#include <stdlib.h>
#include <sys/eventfd.h>
#include "tst_test.h"

static void run(void)
{
	int fd;
	fd_set writefds;
	uint64_t val;
	uint64_t non_max = 10;
	uint64_t max = UINT64_MAX - 1;
	struct timeval timeout = { 0, 0 };

	fd = TST_EXP_FD(eventfd(0, EFD_NONBLOCK));

	FD_ZERO(&writefds);
	FD_SET(fd, &writefds);

	SAFE_WRITE(0, fd, &non_max, sizeof(non_max));
	TEST(select(fd + 1, NULL, &writefds, NULL, &timeout));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "select");

	TST_EXP_EQ_LI(FD_ISSET(fd, &writefds), 1);

	SAFE_READ(0, fd, &val, sizeof(val));
	SAFE_WRITE(0, fd, &max, sizeof(max));

	TEST(select(fd + 1, NULL, &writefds, NULL, &timeout));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "select");

	TST_EXP_EQ_LI(FD_ISSET(fd, &writefds), 0);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_EVENTFD",
		NULL
	},
};
