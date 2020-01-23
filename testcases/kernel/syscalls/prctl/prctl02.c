// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * 1) prctl() fails with EINVAL when an invalid value is given for option
 * 2) prctl() fails with EINVAL when option is PR_SET_PDEATHSIG & arg2 is
 * not zero or a valid signal number.
 * 3) prctl() fails with EINVAL when option is PR_SET_DUMPABLE & arg2 is
 * neither SUID_DUMP_DISABLE nor SUID_DUMP_USER.
 * 4) prctl() fails with EFAULT when arg2 is an invalid address.
 * 5) prctl() fails with EFAULT when option is PR_SET_SECCOMP & arg2 is
 * SECCOMP_MODE_FILTER & arg3 is an invalid address.
 * 6) prctl() fails with EACCES when option is PR_SET_SECCOMP & arg2 is
 * SECCOMP_MODE_FILTER & the process does not have the CAP_SYS_ADMIN
 * capability.
 * 7) prctl() fails with EINVAL when option is PR_SET_TIMING & arg2 is not
 * not PR_TIMING_STATISTICAL.
 * 8,9) prctl() fails with EINVAL when option is PR_SET_NO_NEW_PRIVS & arg2
 * is not equal to 1 or arg3 is nonzero.
 * 10) prctl() fails with EINVAL when options is PR_GET_NO_NEW_PRIVS & arg2,
 * arg3, arg4, or arg5 is nonzero.
 * 11) prctl() fails with EINVAL when options is PR_SET_THP_DISABLE & arg3,
 * arg4, arg5 is non-zero.
 * 12) prctl() fails with EINVAL when options is PR_GET_THP_DISABLE & arg2,
 * arg3, arg4, or arg5 is nonzero.
 * 13) prctl() fails with EINVAL when options is PR_CAP_AMBIENT & an unused
 * argument such as arg4 is nonzero.
 * 14) prctl() fails with EINVAL when option is PR_GET_SPECULATION_CTRL and
 * unused arguments is nonzero.
 * 15) prctl() fails with EPERM when option is PR_SET_SECUREBITS and the
 * caller does not have the CAP_SETPCAP capability.
 * 16) prctl() fails with EPERM when option is PR_CAPBSET_DROP and the caller
 * does not have the CAP_SETPCAP capability.
 */

#include <errno.h>
#include <signal.h>
#include <sys/prctl.h>
#include <linux/filter.h>
#include <linux/capability.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include "config.h"
#include "lapi/prctl.h"
#include "lapi/seccomp.h"
#include "lapi/syscalls.h"
#include "tst_test.h"
#include "tst_capability.h"

#define OPTION_INVALID 999
#define unsup_string "prctl() doesn't support this option, skip it"
static const struct sock_filter  strict_filter[] = {
	BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)
};

static const struct sock_fprog strict = {
	.len = (unsigned short)ARRAY_SIZE(strict_filter),
	.filter = (struct sock_filter *)strict_filter
};

static unsigned long strict_addr = (unsigned long)&strict;

static unsigned long bad_addr;
static unsigned long num_0;
static unsigned long num_1 = 1;
static unsigned long num_2 = 2;
static unsigned long num_invalid = ULONG_MAX;
static int seccomp_nsup;
static int nonewprivs_nsup;
static int thpdisable_nsup;
static int capambient_nsup;
static int speculationctrl_nsup;

static struct tcase {
	int option;
	unsigned long *arg2;
	unsigned long *arg3;
	int exp_errno;
	char *tname;
} tcases[] = {
	{OPTION_INVALID, &num_1, &num_0, EINVAL, "invalid option"},
	{PR_SET_PDEATHSIG, &num_invalid, &num_0, EINVAL, "PR_SET_PDEATHSIG"},
	{PR_SET_DUMPABLE, &num_2, &num_0, EINVAL, "PR_SET_DUMPABLE"},
	{PR_SET_NAME, &bad_addr, &num_0, EFAULT, "PR_SET_NAME"},
	{PR_SET_SECCOMP, &num_2, &bad_addr, EFAULT, "PR_SET_SECCOMP"},
	{PR_SET_SECCOMP, &num_2, &strict_addr, EACCES, "PR_SET_SECCOMP"},
	{PR_SET_TIMING, &num_1, &num_0, EINVAL, "PR_SET_TIMING"},
	{PR_SET_NO_NEW_PRIVS, &num_0, &num_0, EINVAL, "PR_SET_NO_NEW_PRIVS"},
	{PR_SET_NO_NEW_PRIVS, &num_1, &num_1, EINVAL, "PR_SET_NO_NEW_PRIVS"},
	{PR_GET_NO_NEW_PRIVS, &num_1, &num_0, EINVAL, "PR_GET_NO_NEW_PRIVS"},
	{PR_SET_THP_DISABLE, &num_0, &num_1, EINVAL, "PR_SET_THP_DISABLE"},
	{PR_GET_THP_DISABLE, &num_1, &num_1, EINVAL, "PR_GET_THP_DISABLE"},
	{PR_CAP_AMBIENT, &num_invalid, &num_0, EINVAL, "PR_CAP_AMBIENT"},
	{PR_GET_SPECULATION_CTRL, &num_0, &num_invalid, EINVAL, "PR_GET_SPECULATION_CTRL"},
	{PR_SET_SECUREBITS, &num_0, &num_0, EPERM, "PR_SET_SECUREBITS"},
	{PR_CAPBSET_DROP, &num_1, &num_0, EPERM, "PR_CAPBSET_DROP"},
};

static void verify_prctl(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	switch (tc->option) {
	case PR_SET_SECCOMP:
		if (seccomp_nsup) {
			tst_res(TCONF, "%s", unsup_string);
			return;
		}
	break;
	case PR_GET_NO_NEW_PRIVS:
	case PR_SET_NO_NEW_PRIVS:
		if (nonewprivs_nsup) {
			tst_res(TCONF, "%s", unsup_string);
			return;
		}
	break;
	case PR_SET_THP_DISABLE:
	case PR_GET_THP_DISABLE:
		if (thpdisable_nsup) {
			tst_res(TCONF, "%s", unsup_string);
			return;
		}
	break;
	case PR_CAP_AMBIENT:
		if (capambient_nsup) {
			tst_res(TCONF, "%s", unsup_string);
			return;
		}
	break;
	case PR_GET_SPECULATION_CTRL:
		if (speculationctrl_nsup) {
			tst_res(TCONF, "%s", unsup_string);
			return;
		}
	break;
	default:
	break;
	}

	TEST(prctl(tc->option, *tc->arg2, *tc->arg3, 0, 0));
	if (TST_RET == 0) {
		tst_res(TFAIL, "prctl() succeeded unexpectedly");
		return;
	}

	if (tc->exp_errno == TST_ERR) {
		tst_res(TPASS | TTERRNO, "prctl() failed as expected");
	} else {
		if (tc->option == PR_SET_SECCOMP && TST_ERR == EINVAL)
			tst_res(TCONF, "current system was not built with CONFIG_SECCOMP_FILTER.");
		else
			tst_res(TFAIL | TTERRNO, "prctl() failed unexpectedly, expected %s",
				tst_strerrno(tc->exp_errno));
	}
}

static void setup(void)
{
	bad_addr = (unsigned long)tst_get_bad_addr(NULL);

	TEST(prctl(PR_GET_SECCOMP));
	if (TST_ERR == EINVAL)
		seccomp_nsup = 1;

	TEST(prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0));
	if (TST_ERR == EINVAL)
		nonewprivs_nsup = 1;

	TEST(prctl(PR_GET_THP_DISABLE, 0, 0, 0, 0));
	if (TST_ERR == EINVAL)
		thpdisable_nsup = 1;

	TEST(prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0, 0));
	if (TST_ERR == EINVAL)
		capambient_nsup = 1;

	TEST(prctl(PR_GET_SPECULATION_CTRL, 0, 0, 0, 0));
	if (TST_ERR == EINVAL)
		speculationctrl_nsup = 1;
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_prctl,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		TST_CAP(TST_CAP_DROP, CAP_SETPCAP),
		{}
	},
};
