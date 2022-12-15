// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * DESCRIPTION
 * 1) Call sysctl(2) as a root user, and attempt to write data
 *    to the kernel_table[]. Since the table does not have write
 *    permissions even for the root, it should fail EPERM.
 * 2) Call sysctl(2) as a non-root user, and attempt to write data
 *    to the kernel_table[]. Since the table does not have write
 *    permission for the regular user, it should fail with EPERM.
 *
 * NOTE: There is a documentation bug in 2.6.33-rc1 where unfortunately
 * the behavior of sysctl(2) isn't properly documented, as discussed
 * in detail in the following thread:
 * http://sourceforge.net/mailarchive/message.php?msg_name=4B7BA24F.2010705%40linux.vnet.ibm.com.
 *
 * The documentation bug is filed as:
 * https://bugzilla.kernel.org/show_bug.cgi?id=15446 . If you want the
 * message removed, please ask your fellow kernel maintainer to fix their
 * documentation.
 *
 * Thanks!
 * -Ngie
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <linux/sysctl.h>
#include <pwd.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

static int exp_eno;

static void verify_sysctl(void)
{
	char *osname = "Linux";
	int name[] = {CTL_KERN, KERN_OSTYPE};
	struct __sysctl_args args = {
		.name = name,
		.nlen = ARRAY_SIZE(name),
		.newval = osname,
		.newlen = sizeof(osname),
	};

	TEST(tst_syscall(__NR__sysctl, &args));
	if (TST_RET != -1) {
		tst_res(TFAIL, "sysctl(2) succeeded unexpectedly");
		return;
	}

	if (TST_ERR == exp_eno) {
		tst_res(TPASS | TTERRNO, "Got expected error");
	} else {
		tst_res(TFAIL | TTERRNO, "Got unexpected error, expected %s",
			tst_strerrno(exp_eno));
	}
}

static void setup(void)
{
	/* Look above this warning. */
	tst_res(TINFO,
		 "this test's results are based on potentially undocumented behavior in the kernel. read the NOTE in the source file for more details");
	exp_eno = EACCES;
}

static void do_test(void)
{
	pid_t pid;
	struct passwd *ltpuser;

	pid = SAFE_FORK();
	if (!pid) {
		ltpuser = SAFE_GETPWNAM("nobody");
		SAFE_SETUID(ltpuser->pw_uid);
		verify_sysctl();
	} else {
		verify_sysctl();
		tst_reap_children();
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.test_all = do_test,
};
