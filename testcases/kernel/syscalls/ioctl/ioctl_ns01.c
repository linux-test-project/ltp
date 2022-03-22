// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Federico Bonfiglio fedebonfi95@gmail.com
 */

/*
 * Test ioctl_ns with NS_GET_PARENT request.
 *
 * Parent process tries to get parent of initial namespace, which should
 * fail with EPERM because it has no parent.
 *
 * Child process has a new pid namespace, which should make the call fail
 * with EPERM error.
 *
 */
#define _GNU_SOURCE

#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/ioctl_ns.h"
#include "lapi/namespaces_constants.h"

#define STACK_SIZE (1024 * 1024)

static char *child_stack;

static void setup(void)
{
	int exists = access("/proc/self/ns/pid", F_OK);

	if (exists < 0)
		tst_res(TCONF, "namespace not available");

	child_stack = ltp_alloc_stack(STACK_SIZE);
	if (!child_stack)
		tst_brk(TBROK|TERRNO, "stack alloc");
}

static void cleanup(void)
{
	free(child_stack);
}

static void test_ns_get_parent(void)
{
	int fd, parent_fd;

	fd = SAFE_OPEN("/proc/self/ns/pid", O_RDONLY);
	parent_fd = ioctl(fd, NS_GET_PARENT);
	if (parent_fd == -1) {
		if (errno == ENOTTY)
			tst_brk(TCONF, "ioctl(NS_GET_PARENT) not implemented");

		if (errno == EPERM)
			tst_res(TPASS, "NS_GET_PARENT fails with EPERM");
		else
			tst_res(TFAIL | TERRNO, "unexpected ioctl error");
	} else {
		tst_res(TFAIL, "call to ioctl succeded");
	}
	SAFE_CLOSE(fd);
}

static int child(void *arg LTP_ATTRIBUTE_UNUSED)
{
	test_ns_get_parent();
	return 0;
}

static void run(void)
{
	test_ns_get_parent();

	if (ltp_clone(CLONE_NEWPID | SIGCHLD, &child, 0,
		STACK_SIZE, child_stack) == -1)
		tst_brk(TBROK | TERRNO, "ltp_clone failed");
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_root = 1,
	.min_kver = "4.9",
	.setup = setup,
	.cleanup = cleanup,
};
