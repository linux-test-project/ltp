// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Federico Bonfiglio fedebonfi95@gmail.com
 */

/*
 * Test ioctl_ns with NS_GET_PARENT request.
 *
 * Child cloned with the CLONE_NEWPID flag is created in a new pid namespace.
 * That's checked by comparing its /proc/self/ns/pid symlink and the parent's
 * one. Also child thinks its pid is 1.
 *
 */
#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <sched.h>
#include "tst_test.h"
#include "lapi/ioctl_ns.h"
#include "lapi/namespaces_constants.h"

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];

static void setup(void)
{
	int exists = access("/proc/self/ns/pid", F_OK);

	if (exists < 0)
		tst_res(TCONF, "namespace not available");
}

static int child(void *arg)
{
	if (getpid() != 1)
		tst_res(TFAIL, "child should think its pid is 1");
	else
		tst_res(TPASS, "child thinks its pid is 1");
	TST_CHECKPOINT_WAIT(0);
	return 0;
}

static void run(void)
{
	pid_t pid = ltp_clone(CLONE_NEWPID, &child, 0,
		STACK_SIZE, child_stack);

	char child_namespace[20];
	int my_fd, child_fd, parent_fd;

	sprintf(child_namespace, "/proc/%i/ns/pid", pid);
	my_fd = SAFE_OPEN("/proc/self/ns/pid", O_RDONLY);
	child_fd = SAFE_OPEN(child_namespace, O_RDONLY);
	parent_fd = ioctl(child_fd, NS_GET_PARENT);

	if (parent_fd == -1) {
		TST_CHECKPOINT_WAKE(0);

		if (errno == ENOTTY) {
			tst_res(TCONF, "ioctl(NS_GET_PARENT) not implemented");
			return;
		}

		tst_brk(TBROK | TERRNO, "ioctl(NS_GET_PARENT) failed");
	}

	struct stat my_stat, child_stat, parent_stat;

	SAFE_FSTAT(my_fd, &my_stat);
	SAFE_FSTAT(child_fd, &child_stat);
	SAFE_FSTAT(parent_fd, &parent_stat);
	if (my_stat.st_ino != parent_stat.st_ino)
		tst_res(TFAIL, "parents have different inodes");
	else if (parent_stat.st_ino == child_stat.st_ino)
		tst_res(TFAIL, "child and parent have same inode");
	else
		tst_res(TPASS, "child and parent are consistent");
	SAFE_CLOSE(my_fd);
	SAFE_CLOSE(child_fd);
	SAFE_CLOSE(parent_fd);
	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.min_kver = "4.9",
	.setup = setup
};
