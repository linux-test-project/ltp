// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Federico Bonfiglio <fedebonfi95@gmail.com>
 * Copyright (c) Linux Test Project, 2019-2022
 */

/*\
 * Test ioctl_ns with NS_GET_USERNS request.
 *
 * Owning user namespace of process calling ioctl is out of scope,
 * which should make the call fail with EPERM.
 */

#define _GNU_SOURCE

#include <errno.h>
#include "tst_test.h"
#include "lapi/ioctl_ns.h"

static void setup(void)
{
	int exists = access("/proc/self/ns/user", F_OK);

	if (exists < 0)
		tst_res(TCONF, "namespace not available");
}

static void run(void)
{
	int fd, parent_fd;

	fd = SAFE_OPEN("/proc/self/ns/user", O_RDONLY);
	parent_fd = ioctl(fd, NS_GET_USERNS);
	if (parent_fd == -1) {
		if (errno == ENOTTY)
			tst_brk(TCONF, "ioctl(NS_GET_USERNS) not implemented");

		if (errno == EPERM)
			tst_res(TPASS, "NS_GET_USERNS fails with EPERM");
		else
			tst_res(TFAIL | TERRNO, "unexpected ioctl error");
	} else {
		tst_res(TFAIL, "call to ioctl succeded");
	}
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.min_kver = "4.9",
	.setup = setup
};
