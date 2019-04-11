// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Federico Bonfiglio fedebonfi95@gmail.com
 */

/*
 * Test ioctl_ns with NS_GET_* request for file descriptors
 * that aren't namespaces.
 *
 * Calling ioctl with test directory's file descriptor
 * should make the call fail with ENOTTY.
 *
 */

#define _GNU_SOURCE

#include <errno.h>
#include "tst_test.h"
#include "lapi/ioctl_ns.h"

static int requests[] = {NS_GET_PARENT, NS_GET_USERNS,
	NS_GET_OWNER_UID, NS_GET_NSTYPE};

static void test_request(unsigned int n)
{
	int request = requests[n];
	int fd, ns_fd;

	fd = SAFE_OPEN(".", O_RDONLY);
	ns_fd = ioctl(fd, request);
	if (ns_fd == -1) {
		if (errno == ENOTTY)
			tst_res(TPASS, "request failed with ENOTTY");
		else
			tst_res(TFAIL | TERRNO, "unexpected ioctl error");
	} else {
		tst_res(TFAIL, "request success for invalid fd");
		SAFE_CLOSE(ns_fd);
	}
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(requests),
	.test = test_request,
	.needs_tmpdir = 1,
	.min_kver = "4.11"
};
