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
 * Test whether readfd is set by select() when eventfd() counter value is
 * non-zero, then check if readfd is not set when eventfd() counter value is
 * zero.
 */

#include <stdlib.h>
#include <sys/eventfd.h>
#include "tst_test.h"

static void run(void)
{
	int fd;
	uint64_t val;
	fd_set readfds;
	uint64_t non_zero = 10;
	struct timeval timeout = { 0, 0 };

	fd = TST_EXP_FD(eventfd(0, EFD_NONBLOCK));

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	SAFE_WRITE(0, fd, &non_zero, sizeof(non_zero));
	TEST(select(fd + 1, &readfds, NULL, NULL, &timeout));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "select");

	TST_EXP_EQ_LI(FD_ISSET(fd, &readfds), 1);

	SAFE_READ(0, fd, &val, sizeof(val));
	TEST(select(fd + 1, &readfds, NULL, NULL, &timeout));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "select");

	TST_EXP_EQ_LI(FD_ISSET(fd, &readfds), 0);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_EVENTFD",
		NULL
	},
};
