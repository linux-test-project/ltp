// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 FUJITSU LIMITED. All rights reserved.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

/*
 * This is a regression test for a crash caused by memcg function
 * reentrant on buggy kernel.  When doing rmdir(), a pending signal can
 * interrupt the execution and lead to cgroup_clear_css_refs()
 * being entered repeatedly, this results in a BUG_ON().
 */

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mount.h>
#include "tst_test.h"

#define MNTPOINT	"memcg"
#define SUBDIR	"memcg/testdir"

static int mount_flag;
static volatile int sigcounter;

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
	sigcounter++;
}

static void do_child(void)
{
	while (1)
		SAFE_KILL(getppid(), SIGUSR1);

	exit(0);
}

static void do_test(void)
{
	pid_t cpid;

	SAFE_SIGNAL(SIGUSR1, sighandler);

	cpid = SAFE_FORK();
	if (cpid == 0)
		do_child();

	while (sigcounter < 50000) {
		if (access(SUBDIR, F_OK))
			SAFE_MKDIR(SUBDIR, 0644);
		rmdir(SUBDIR);
	}

	SAFE_KILL(cpid, SIGKILL);
	SAFE_WAIT(NULL);

	tst_res(TPASS, "Bug not reproduced");
}

static void setup(void)
{
	int ret;

	SAFE_MKDIR(MNTPOINT, 0644);

	ret = mount("memcg", MNTPOINT, "cgroup", 0, "memory");
	if (ret) {
		if (errno == ENOENT)
			tst_brk(TCONF | TERRNO, "memcg not supported");

		tst_brk(TCONF | TERRNO, "mounting memcg failed");
	}
	mount_flag = 1;
}

static void cleanup(void)
{
	if (!access(SUBDIR, F_OK))
		SAFE_RMDIR(SUBDIR);

	if (mount_flag)
		tst_umount(MNTPOINT);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.min_kver = "2.6.24",
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test,
};
