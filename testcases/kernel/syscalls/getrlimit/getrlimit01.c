// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies, 2002. All Rights Reserved.
 * Author: Suresh Babu V. <suresh.babu@wipro.com>
 */

/*\
 * Verify that getrlimit(2) call will be successful for all possible resource
 * types.
 */

#include <sys/resource.h>
#include "tst_test.h"

#define RES(x) .res = x, .res_str = #x
static struct tcase {
	int res;
	char *res_str;
} tcases[] = {
	{RES(RLIMIT_CPU)},
	{RES(RLIMIT_FSIZE)},
	{RES(RLIMIT_DATA)},
	{RES(RLIMIT_STACK)},
	{RES(RLIMIT_CORE)},
	{RES(RLIMIT_RSS)},
	{RES(RLIMIT_NPROC)},
	{RES(RLIMIT_NOFILE)},
	{RES(RLIMIT_MEMLOCK)},
	{RES(RLIMIT_AS)},
	{RES(RLIMIT_LOCKS)},
	{RES(RLIMIT_MSGQUEUE)},
#ifdef RLIMIT_NICE
	{RES(RLIMIT_NICE)},
#endif
#ifdef RLIMIT_RTPRIO
	{RES(RLIMIT_RTPRIO)},
#endif
	{RES(RLIMIT_SIGPENDING)},
#ifdef RLIMIT_RTTIME
	{RES(RLIMIT_RTTIME)},
#endif
};

static void verify_getrlimit(unsigned int i)
{
	struct rlimit rlim;
	struct tcase *tc = &tcases[i];

	TST_EXP_PASS(getrlimit(tc->res, &rlim), "getrlimit(%s, &rlim)",
		     tc->res_str);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getrlimit,
};
