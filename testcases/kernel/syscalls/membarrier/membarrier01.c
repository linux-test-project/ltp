// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */
/*
 * Basic tests for membarrier(2) syscall. Tests below are responsible for
 * testing the membarrier(2) interface only, without checking if the barrier
 * was successful or not. Check test_case structure for each test description.
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syscall.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/membarrier.h"

struct test_case {
	char testname[80];
	int command;		/* membarrier cmd                            */
	int needregister;	/* membarrier cmd needs register cmd	     */
	int flags;		/* flags for given membarrier cmd	     */
	long exp_ret;		/* expected return code for given cmd        */
	int exp_errno;		/* expected errno for given cmd failure      */
	int enabled;		/* enabled, despite results from CMD_QUERY   */
	int always;		/* CMD_QUERY should always enable this test  */
	int force;		/* force if CMD_QUERY reports not enabled    */
	int force_exp_errno;	/* expected errno after forced cmd           */
	int change_exp_errno;	/* previous kernels forced errno result      */
	int change_kernver[3];	/* kernel version having diff expected errno */
};

struct test_case tc[] = {
	{
	 /*
	  * case 00) invalid cmd
	  *     - enabled by default
	  *     - should always fail with EINVAL
	  */
	 .testname = "cmd_fail",
	 .command = -1,
	 .exp_ret = -1,
	 .exp_errno = EINVAL,
	 .enabled = 1,
	 },
	{
	 /*
	  * case 01) invalid flags
	  *     - enabled by default
	  *     - should always fail with EINVAL
	  */
	 .testname = "cmd_flags_fail",
	 .command = MEMBARRIER_CMD_QUERY,
	 .flags = 1,
	 .exp_ret = -1,
	 .exp_errno = EINVAL,
	 .enabled = 1,
	 },
	{
	 /*
	  * case 02) global barrier
	  *     - should ALWAYS be enabled by CMD_QUERY
	  *     - should always succeed
	  */
	 .testname = "cmd_global_success",
	 .command = MEMBARRIER_CMD_GLOBAL,
	 .flags = 0,
	 .exp_ret = 0,
	 .always = 1,
	 },
	 /*
	  * commit 22e4ebb975 (v4.14-rc1) added cases 03, 04 and 05 features:
	  */
	{
	 /*
	  * case 03) private expedited barrier with no registrations
	  *     - should fail with errno=EPERM due to no registrations
	  *     - or be skipped if unsupported by running kernel
	  */
	 .testname = "cmd_private_expedited_fail",
	 .command = MEMBARRIER_CMD_PRIVATE_EXPEDITED,
	 .flags = 0,
	 .exp_ret = -1,
	 .exp_errno = EPERM,
	 },
	{
	 /*
	  * case 04) register private expedited
	  *     - should succeed when supported by running kernel
	  *     - or fail with errno=EINVAL if unsupported and forced
	  */
	 .testname = "cmd_private_expedited_register_success",
	 .command = MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED,
	 .flags = 0,
	 .exp_ret = 0,
	 .force = 1,
	 .force_exp_errno = EINVAL,
	 },
	{
	 /*
	  * case 05) private expedited barrier with registration
	  *     - should succeed due to existing registration
	  *     - or fail with errno=EINVAL if unsupported and forced
	  *     - NOTE: commit 70216e18e5 (v4.16-rc1) changed behavior:
	  *     -       (a) if unsupported, and forced, < 4.16 , errno is EINVAL
	  *     -       (b) if unsupported, and forced, >= 4.16, errno is EPERM
	  */
	 .testname = "cmd_private_expedited_success",
	 .needregister = MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED,
	 .command = MEMBARRIER_CMD_PRIVATE_EXPEDITED,
	 .flags = 0,
	 .exp_ret = 0,
	 .force = 1,
	 .force_exp_errno = EPERM,
	 .change_exp_errno = EINVAL,
	 .change_kernver = { 4, 16, 0 },
	 },
	 /*
	  * commit 70216e18e5 (v4.16-rc1) added cases 06, 07 and 08 features:
	  */
	{
	 /*
	  * case 06) private expedited sync core barrier with no registrations
	  *     - should fail with errno=EPERM due to no registrations
	  *     - or be skipped if unsupported by running kernel
	  */
	 .testname = "cmd_private_expedited_sync_core_fail",
	 .command = MEMBARRIER_CMD_PRIVATE_EXPEDITED_SYNC_CORE,
	 .flags = 0,
	 .exp_ret = -1,
	 .exp_errno = EPERM,
	 },
	{
	 /*
	  * case 07) register private expedited sync core
	  *     - should succeed when supported by running kernel
	  *     - or fail with errno=EINVAL if unsupported and forced
	  */
	 .testname = "cmd_private_expedited_sync_core_register_success",
	 .command = MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED_SYNC_CORE,
	 .flags = 0,
	 .exp_ret = 0,
	 .force = 1,
	 .force_exp_errno = EINVAL,
	 },
	{
	 /*
	  * case 08) private expedited sync core barrier with registration
	  *     - should succeed due to existing registration
	  *     - or fail with errno=EINVAL if unsupported and forced
	  */
	 .testname = "cmd_private_expedited_sync_core_success",
	 .needregister = MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED_SYNC_CORE,
	 .command = MEMBARRIER_CMD_PRIVATE_EXPEDITED_SYNC_CORE,
	 .flags = 0,
	 .exp_ret = 0,
	 .force = 1,
	 .force_exp_errno = EINVAL,
	 },
	 /*
	  * commit c5f58bd58f4 (v4.16-rc1) added cases 09, 10 and 11 features:
	  */
	{
	 /*
	  * case 09) global expedited barrier with no registrations
	  *     - should never fail due to no registrations
	  *     - or be skipped if unsupported by running kernel
	  */
	 .testname = "cmd_global_expedited_success",
	 .command = MEMBARRIER_CMD_GLOBAL_EXPEDITED,
	 .flags = 0,
	 .exp_ret = 0,
	 },
	{
	 /*
	  * case 10) register global expedited
	  *     - should succeed when supported by running kernel
	  *     - or fail with errno=EINVAL if unsupported and forced
	  */
	 .testname = "cmd_global_expedited_register_success",
	 .command = MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED,
	 .flags = 0,
	 .exp_ret = 0,
	 .force = 1,
	 .force_exp_errno = EINVAL,
	 },
	{
	 /*
	  * case 11) global expedited barrier with registration
	  *     - should also succeed with registrations
	  *     - or fail with errno=EINVAL if unsupported and forced
	  */
	 .testname = "cmd_global_expedited_success",
	 .needregister = MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED,
	 .command = MEMBARRIER_CMD_GLOBAL_EXPEDITED,
	 .flags = 0,
	 .exp_ret = 0,
	 .force = 1,
	 .force_exp_errno = EINVAL,
	 },
};

#define passed_ok(_test)						       \
	do {								       \
		tst_res(TPASS, "membarrier(2): %s passed", _test.testname);    \
		return;							       \
	} while (0)

#define passed_unexpec(_test)						       \
	do {								       \
		tst_res(TFAIL, "membarrier(2): %s passed unexpectedly. "       \
			"ret = %ld with errno %d were expected. (force: %d)",  \
			_test.testname, _test.exp_ret, _test.exp_errno,        \
			_test.force);					       \
		return;							       \
	} while (0)

#define failed_ok(_test)						       \
	do {								       \
		tst_res(TPASS, "membarrier(2): %s failed as expected",	       \
			_test.testname);				       \
		return;							       \
	} while (0)

#define failed_ok_unsupported(_test)					       \
	do {								       \
		tst_res(TPASS, "membarrier(2): %s failed as expected "	       \
			"(unsupported)", _test.testname);		       \
		return;							       \
	} while (0)

#define failed_not_ok(_test, _gotret, _goterr)				       \
	do {								       \
		tst_res(TFAIL, "membarrier(2): %s failed. "		       \
			"ret = %ld when expected was %ld. "		       \
			"errno = %d when expected was %d. (force: %d)",        \
			_test.testname, _gotret, _test.exp_ret, _goterr,       \
			_test.exp_errno, _test.force);			       \
		return;							       \
	} while (0)

#define failed_unexpec(_test, _gotret, _goterr) 			       \
	do {								       \
		tst_res(TFAIL, "membarrier(2): %s failed unexpectedly. "       \
			"Got ret = %ld with errno %d. (force: %d)",	       \
			_test.testname, _gotret, _goterr, _test.force);	       \
		return;							       \
	} while (0)

#define skipped(_test)							       \
	do {								       \
		tst_res(TPASS, "membarrier(2): %s skipped (unsupported)",      \
			_test.testname);				       \
		return;							       \
	} while (0)

#define skipped_fail(_test)						       \
	do {								       \
		tst_res(TFAIL, "membarrier(2): %s reported as not supported",  \
			_test.testname);				       \
		return;							       \
	} while (0)

static int sys_membarrier(int cmd, int flags)
{
	return tst_syscall(__NR_membarrier, cmd, flags);
}

static void verify_membarrier(unsigned int i)
{
	int ret;

	/* not enabled and not enforced: test is skipped */

	if (!tc[i].enabled && !tc[i].force) {

		if (tc[i].always == 0)
			skipped(tc[i]);

		skipped_fail(tc[i]);
	}

	/* iterations: registration needed for some cases */

	if (tc[i].needregister && tc[i].enabled) {
		ret = sys_membarrier(tc[i].needregister, 0);
		if (ret < 0) {
			tst_brk(TBROK, "membarrier(2): %s could not register",
					tc[i].testname);
		}
	}

	TEST(sys_membarrier(tc[i].command, tc[i].flags));

	/* enabled and not enforced: regular expected results only */

	if (tc[i].enabled && !tc[i].force) {

		if (TST_RET >= 0 && tc[i].exp_ret == TST_RET)
			passed_ok(tc[i]);

		if (TST_RET < 0) {
			if (tc[i].exp_ret == TST_RET)
				failed_ok(tc[i]);
			else
				failed_not_ok(tc[i], TST_RET, TST_ERR);
		}
	}

	/* not enabled and enforced: failure and expected errors */

	if (!tc[i].enabled && tc[i].force) {

		if (TST_RET >= 0)
			passed_unexpec(tc[i]);

		if (TST_RET < 0) {
			if (tc[i].force_exp_errno == TST_ERR)
				failed_ok_unsupported(tc[i]);
			else
				failed_unexpec(tc[i], TST_RET, TST_ERR);
		}
	}

	/* enabled and enforced: tricky */

	if (tc[i].enabled && tc[i].force) {

		if (TST_RET >= 0) {
			if (tc[i].exp_ret == TST_RET)
				passed_ok(tc[i]);
			else
				passed_unexpec(tc[i]);
		}

		if (TST_RET < 0) {

			if (tc[i].exp_ret == TST_RET) {

				if (tc[i].exp_errno == TST_ERR)
					failed_ok(tc[i]);
				else
					failed_unexpec(tc[i], TST_RET, TST_ERR);
			}

			/* unknown on force failure if enabled and forced */
			failed_unexpec(tc[i], TST_RET, TST_ERR);
		}
	}
}

static void wrap_verify_membarrier(unsigned int i)
{
	pid_t pid;

	/*
	 * The Linux kernel does not provide a way to unregister the process
	 * (mm->membarrier_state) intent of being affected by the membarrier(2)
	 * system call, thus the need of having a wrapper to fork() a child.
	 */

	pid = SAFE_FORK();

	if (pid)
		SAFE_WAITPID(pid, NULL, 0);
	else
		verify_membarrier(i);
}

static void setup(void)
{
	size_t i;
	int ret;

	ret = sys_membarrier(MEMBARRIER_CMD_QUERY, 0);
	if (ret < 0) {
		if (errno == ENOSYS)
			tst_brk(TBROK, "membarrier(2): not supported");
	}

	for (i = 0; i < ARRAY_SIZE(tc); i++) {
		if ((tc[i].command > 0) && (ret & tc[i].command))
			tc[i].enabled = 1;

		/* forcing unsupported command might have different errno */

		if (tc[i].change_exp_errno && tc[i].enabled == 0) {
			if (tst_kvercmp(tc[i].change_kernver[0],
					tc[i].change_kernver[1],
					tc[i].change_kernver[2]) < 0)
				tc[i].force_exp_errno = tc[i].change_exp_errno;
		}
	}
}

static struct tst_test test = {
	.setup = setup,
	.test = wrap_verify_membarrier,
	.tcnt = ARRAY_SIZE(tc),
	.min_kver = "4.3.0",	/* commit: 5b25b13ab0 (sys_membarrier(): ...) */
	.forks_child = 1,
};
