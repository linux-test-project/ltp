// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef STIME_VAR__
#define STIME_VAR__

#include <sys/time.h>
#include "config.h"
#include "lapi/syscalls.h"

#define TEST_VARIANTS 3

static int do_stime(time_t *ntime)
{
	switch (tst_variant) {
	case 0:
#ifndef HAVE_STIME
		tst_brk(TCONF, "libc stime() is not implemented");
#else
		return stime(ntime);
#endif
	break;
	case 1:
		return tst_syscall(__NR_stime, ntime);
	case 2: {
		struct timeval tv;

		tv.tv_sec = *ntime;
		tv.tv_usec = 0;

		return tst_syscall(__NR_settimeofday, &tv, (struct timezone *) 0);
	}
	}

	return -1;
}

static void stime_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing libc stime()");
	break;
	case 1:
		tst_res(TINFO, "Testing SYS_stime syscall");
	break;
	case 2:
		tst_res(TINFO, "Testing SYS_settimeofday syscall");
	break;
	}
}

#endif /* STIME_VAR__ */
