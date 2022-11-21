// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies, 2002. All Rights Reserved.
 * Author: Suresh Babu V. <suresh.babu@wipro.com>
 */

/*\
 * [Description]
 *
 * Verify that getrlimit(2) call will be successful for all possible resource
 * types.
 */

#include <sys/resource.h>
#include "tst_test.h"

static struct tcase {
	int res;
	char *res_str;
} tcases[] = {
	{RLIMIT_CPU,        "RLIMIT_CPU"},
	{RLIMIT_FSIZE,      "RLIMIT_FSIZE"},
	{RLIMIT_DATA,       "RLIMIT_DATA"},
	{RLIMIT_STACK,      "RLIMIT_STACK"},
	{RLIMIT_CORE,       "RLIMIT_CORE"},
	{RLIMIT_RSS,        "RLIMIT_RSS"},
	{RLIMIT_NPROC,      "RLIMIT_NPROC"},
	{RLIMIT_NOFILE,     "RLIMIT_NOFILE"},
	{RLIMIT_MEMLOCK,    "RLIMIT_MEMLOCK"},
	{RLIMIT_AS,         "RLIMIT_AS"},
	{RLIMIT_LOCKS,      "RLIMIT_LOCKS"},
	{RLIMIT_MSGQUEUE,   "RLIMIT_MSGQUEUE"},
#ifdef RLIMIT_NICE
	{RLIMIT_NICE,       "RLIMIT_NICE"},
#endif
#ifdef RLIMIT_RTPRIO
	{RLIMIT_RTPRIO,     "RLIMIT_RTPRIO"},
#endif
	{RLIMIT_SIGPENDING, "RLIMIT_SIGPENDING"},
#ifdef RLIMIT_RTTIME
	{RLIMIT_RTTIME,     "RLIMIT_RTTIME"},
#endif
};

static void verify_getrlimit(unsigned int i)
{
	struct rlimit rlim;
	struct tcase *tc = &tcases[i];

	TST_EXP_PASS(getrlimit(tc->res, &rlim),
				"getrlimit() test for %s",
				tc->res_str);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getrlimit,
};
