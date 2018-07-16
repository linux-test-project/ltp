/*
 * Copyright (c) International Business Machines  Corp., 200i1
 * Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

/*
 * DESCRIPTION
 * 1) Call sysctl(2) with nlen set to 0, and expect ENOTDIR.
 * 2) Call sysctl(2) with nlen greater than CTL_MAXNAME, and expect ENOTDIR.
 * 3) Call sysctl(2) with the address of oldname outside the address space of
 *    the process, and expect EFAULT.
 * 4) Call sysctl(2) with the address of soldval outside the address space of
 *    the process, and expect EFAULT.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <linux/sysctl.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

static char osname[BUFSIZ];
static size_t length = BUFSIZ;

static struct tcase {
	int name[2];
	int nlen;
	void *oldval;
	size_t *oldlen;
	int exp_err;
} tcases[] = {
	{{CTL_KERN, KERN_OSREV}, 0, osname, &length, ENOTDIR},
	{{CTL_KERN, KERN_OSREV}, CTL_MAXNAME + 1, osname, &length, ENOTDIR},
	{{CTL_KERN, KERN_OSRELEASE}, 2, (void *) -1, &length, EFAULT},
	{{CTL_KERN, KERN_VERSION}, 2, osname, (void *) -1, EFAULT},
};

static void verify_sysctl(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct __sysctl_args args = {
		.name = tc->name,
		.nlen = tc->nlen,
		.oldval = tc->oldval,
		.oldlenp = tc->oldlen,
	};

	TEST(tst_syscall(__NR__sysctl, &args));
	if (TST_RET != -1) {
		tst_res(TFAIL, "sysctl(2) succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc->exp_err) {
		tst_res(TPASS | TTERRNO, "Got expected error");
	} else {
		tst_res(TFAIL | TTERRNO, "Got unexpected error, expected %s",
			tst_strerrno(tc->exp_err));
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_sysctl,
};
