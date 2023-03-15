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
 * Verify write operation for eventfd fail with:
 *
 * - EAGAIN when counter is zero on non blocking fd
 * - EINVAL when buffer size is less than 8 bytes, or if an attempt is made to
 *	write the value 0xffffffffffffffff
 */

#include <stdlib.h>
#include <sys/eventfd.h>
#include "tst_test.h"

static void run(void)
{
	int fd;
	uint64_t val = 12;
	uint64_t buf;
	uint32_t invalid;

	fd = TST_EXP_FD(eventfd(0, EFD_NONBLOCK));

	SAFE_WRITE(0, fd, &val, sizeof(val));
	SAFE_READ(0, fd, &buf, sizeof(buf));
	TST_EXP_EQ_LI(buf, val);

	val = UINT64_MAX - 1;
	SAFE_WRITE(0, fd, &val, sizeof(val));
	TST_EXP_FAIL(write(fd, &val, sizeof(val)), EAGAIN);
	TST_EXP_FAIL(write(fd, &invalid, sizeof(invalid)), EINVAL);

	val = 0xffffffffffffffffLL;
	TST_EXP_FAIL(write(fd, &val, sizeof(val)), EINVAL);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_EVENTFD",
		NULL
	},
};
