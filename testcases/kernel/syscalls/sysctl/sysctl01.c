// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Copyright (c) 2019 SUSE.  All Rights Reserved.
 */

/*
 * DESCRIPTION:
 * Testcase for testing the basic functionality of sysctl(2) system call.
 * This testcase attempts to read the kernel parameters by using
 * sysctl({CTL_KERN, KERN_* }, ...) and compares it with the known values.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <linux/version.h>
#include <sys/utsname.h>
#include <linux/unistd.h>
#include <linux/sysctl.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

static struct utsname buf;

static struct tcase {
	char *desc;
	int name[2];
	char *cmp_str;
} tcases[] = {
	{"KERN_OSTYPE", {CTL_KERN, KERN_OSTYPE}, buf.sysname},
	{"KERN_OSRELEASE", {CTL_KERN, KERN_OSRELEASE}, buf.release},
	{"KERN_VERSION", {CTL_KERN, KERN_VERSION}, buf.version},
};

static void verify_sysctl(unsigned int n)
{
	char osname[BUFSIZ];
	size_t length = BUFSIZ;
	struct tcase *tc = &tcases[n];

	memset(osname, 0, BUFSIZ);

	struct __sysctl_args args = {
		.name = tc->name,
		.nlen = ARRAY_SIZE(tc->name),
		.oldval = osname,
		.oldlenp = &length,
	};

	TEST(tst_syscall(__NR__sysctl, &args));
	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "sysctl() failed unexpectedly");
		return;
	}

	if (strcmp(osname, tc->cmp_str)) {
		tst_res(TFAIL, "Strings don't match %s : %s",
			osname, tc->cmp_str);
	} else {
		tst_res(TPASS, "Test for %s is correct", tc->desc);
	}
}

static void setup(void)
{
	/* get kernel name and information */
	if (uname(&buf) == -1)
		tst_brk(TBROK | TERRNO, "uname() failed");

	/* revert uname change in case of kGraft/livepatch */
	char *klp_tag;
	char *right_brace;

	klp_tag = strstr(buf.version, "/kGraft-");
	if (!klp_tag)
		klp_tag = strstr(buf.version, "/lp-");
	if (klp_tag) {
		right_brace = strchr(klp_tag, ')');
		if (right_brace)
			memmove(klp_tag, right_brace, strlen(right_brace)+1);
	}
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_sysctl,
};
