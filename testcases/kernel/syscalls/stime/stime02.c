// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * Test Description:
 *   Verify that the system call stime() fails to set the system's idea
 *   of data and time if invoked by "non-root" user.
 *
 * Expected Result:
 *  stime() should fail with return value -1 and set errno to EPERM.
 *
 * History
 *	07/2001 John George
 *		-Ported
 */

#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>

#include "tst_test.h"
#include "stime_var.h"

static time_t new_time;

static void run(void)
{
	TEST(do_stime(&new_time));
	if (TST_RET != -1) {
		tst_res(TFAIL,
			"stime() returned %ld, expected -1 EPERM", TST_RET);
		return;
	}

	if (TST_ERR == EPERM) {
		tst_res(TPASS | TTERRNO, "stime(2) fails, Caller not root");
		return;
	}

	tst_res(TFAIL | TTERRNO,
		"stime(2) fails, Caller not root, expected errno:%d", EPERM);
}

static void setup(void)
{
	time_t curr_time;
	struct passwd *ltpuser;

	stime_info();

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETUID(ltpuser->pw_uid);

	if ((curr_time = time(NULL)) < 0)
		tst_brk(TBROK | TERRNO, "time() failed");

	new_time = curr_time + 10;
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.test_variants = TEST_VARIANTS,
};
