// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 * Author: William Roske
 * Co-pilot: Dave Fenner
 */

/*
 * Testcase to test the basic functionality of setregid(2) systemm call.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

static gid_t gid, egid;	    /* current real and effective group id */
static gid_t neg_one = -1;

static struct tcase {
	gid_t *arg1;
	gid_t *arg2;
	const char *msg;
} tcases[] = {
	{&neg_one, &neg_one, "Dont change either real or effective gid" },
	{&neg_one, &egid,    "Change effective to effective gid" },
	{&gid,     &neg_one, "Change real to real gid" },
	{&neg_one, &gid,     "Change effective to real gid" },
	{&gid,     &gid,     "Try to change real to current real" }
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(SETREGID(*tc->arg1, *tc->arg2));

	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "%s", tc->msg);
	else
		tst_res(TPASS, "%s", tc->msg);
}

static void setup(void)
{
	gid = getgid();
	GID16_CHECK(gid, setregid);

	egid = getegid();
	GID16_CHECK(egid, setregid);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
};
