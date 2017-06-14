/*
 * Copyright (c) 2017 Fujitsu Ltd.
 *  Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a regression test for a crash caused by memcg function
 * reentrant on RHEL6.  When doing rmdir(), a pending signal can
 * interrupt the execution and lead to cgroup_clear_css_refs()
 * being entered repeatedly, this results in a BUG_ON().
 *
 * This bug was introduced by following RHEL6 patch on 2.6.32-488.el6:
 *
 *  [mm] memcg: fix race condition between memcg teardown and swapin
 *  Link: https://bugzilla.redhat.com/show_bug.cgi?id=1001197
 *  Patch: ftp://partners.redhat.com/1c5d859a/de6aafa8185ed8fd934f2debc72b79eb/kernel-individual-patch/rhel6/v2.6.32-to-kernel-2.6.32-488.el6.tar.bz2
 *         31675-mm-memcg-fix-race-condition-between-memcg-teardown-.patch
 *
 * This test can crash the buggy kernel on RHEL6.6GA, and the bug
 * was fixed by following patch on 2.6.32-536.el6:
 *
 *  [mm] memcg: fix crash in re-entrant cgroup_clear_css_refs()
 *  Link: https://bugzilla.redhat.com/show_bug.cgi?id=1168185
 *  Patch: ftp://partners.redhat.com/1c5d859a/de6aafa8185ed8fd934f2debc72b79eb/kernel-individual-patch/rhel6/v2.6.32-to-kernel-2.6.32-536.el6.tar.bz2
 *         35944-mm-memcg-fix-crash-in-re-entrant-cgroup_clear_css_r.patch
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
	.tid = "memcg_test_3",
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.min_kver = "2.6.24",
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test,
};
