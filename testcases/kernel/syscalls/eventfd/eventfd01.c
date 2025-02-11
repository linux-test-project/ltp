// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
 * Copyright (c) Linux Test Project, 2008-2022
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify read operation for eventfd fail with:
 *
 * - EAGAIN when counter is zero on non blocking fd
 * - EINVAL when buffer size is less than 8 bytes
 */

#include <stdlib.h>
#include <sys/eventfd.h>
#include "tst_test.h"

#define EVENT_COUNT 10

static void run(void)
{
	int fd;
	uint64_t val;
	uint32_t invalid;

	fd = TST_EXP_FD(eventfd(EVENT_COUNT, EFD_NONBLOCK));

	SAFE_READ(0, fd, &val, sizeof(val));
	TST_EXP_EQ_LI(val, EVENT_COUNT);

	TST_EXP_FAIL(read(fd, &val, sizeof(val)), EAGAIN);
	TST_EXP_FAIL(read(fd, &invalid, sizeof(invalid)), EINVAL);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_EVENTFD",
		NULL
	},
};
