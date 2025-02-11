// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
 * Copyright (c) Linux Test Project, 2008-2022
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test whether eventfd() counter update in child is reflected in the parent.
 */

#include <stdlib.h>
#include <sys/eventfd.h>
#include "tst_test.h"

static void run(void)
{
	int fd;
	uint64_t val;
	uint64_t to_parent = 0xdeadbeef;

	fd = TST_EXP_FD(eventfd(0, EFD_NONBLOCK));

	if (!SAFE_FORK()) {
		SAFE_WRITE(0, fd, &to_parent, sizeof(to_parent));
		exit(0);
	}

	tst_reap_children();

	SAFE_READ(0, fd, &val, sizeof(val));
	TST_EXP_EQ_LI(val, to_parent);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_EVENTFD",
		NULL
	},
};
