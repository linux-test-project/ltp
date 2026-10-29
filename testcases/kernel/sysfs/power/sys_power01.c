// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the power management attributes exported under
 * /sys/power/.
 *
 * The test verifies that:
 *
 * - pm_async, pm_debug_messages, pm_print_times and sync_on_suspend are
 *   booleans
 * - pm_freeze_timeout and wakeup_count are non-negative
 * - disk is a bracketed-choice file selecting one of platform, shutdown,
 *   reboot, suspend, test_resume or disabled. disk_show() in
 *   kernel/power/hibernate.c reports ``[disabled]`` as the sole token whenever
 *   hibernation_available() is false, e.g. because of the ``nohibernate`` boot
 *   parameter, kernel lockdown (common on Secure Boot enabled distributions)
 *   restricting LOCKDOWN_HIBERNATION, secretmem being in use, or CXL memory
 *   being active
 * - mem_sleep is a bracketed-choice file selecting one of s2idle, shallow or
 *   deep
 * - state lists only known sleep states (freeze, standby, mem, disk)
 *
 * Unlike disk and mem_sleep, state never marks a ``current`` value with
 * brackets, it merely enumerates the sleep states the kernel supports, so it
 * is validated as a plain token set rather than a bracketed choice.
 *
 * All checks skip gracefully with TCONF when a particular attribute is not
 * present.
 */

#include <limits.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static const char *const disk_allowed[] = {
	"platform", "shutdown", "reboot", "suspend", "test_resume",
	"disabled", NULL
};

static const char *const mem_sleep_allowed[] = {
	"s2idle", "shallow", "deep", NULL
};

static const char *const state_allowed[] = {
	"freeze", "standby", "mem", "disk", NULL
};

static void do_test(void)
{
	TST_SYSFS_ASSERT_BOOL(PATH_SYS_POWER "/pm_async");
	TST_SYSFS_ASSERT_BOOL(PATH_SYS_POWER "/pm_debug_messages");
	TST_SYSFS_ASSERT_BOOL(PATH_SYS_POWER "/pm_print_times");
	TST_SYSFS_ASSERT_BOOL(PATH_SYS_POWER "/sync_on_suspend");

	TST_SYSFS_ASSERT_RANGELL(0, LONG_MAX, PATH_SYS_POWER "/pm_freeze_timeout");
	TST_SYSFS_ASSERT_RANGELL(0, LONG_MAX, PATH_SYS_POWER "/wakeup_count");

	TST_SYSFS_ASSERT_CHOICE(disk_allowed, NULL, 0, PATH_SYS_POWER "/disk");
	TST_SYSFS_ASSERT_CHOICE(mem_sleep_allowed, NULL, 0, PATH_SYS_POWER "/mem_sleep");

	TST_SYSFS_ASSERT_TOKENS(state_allowed, NULL, PATH_SYS_POWER "/state");
}

static struct tst_test test = {
	.test_all = do_test,
};
