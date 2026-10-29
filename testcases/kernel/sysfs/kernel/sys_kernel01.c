// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for assorted scalar attributes exported directly under
 * /sys/kernel/.
 *
 * The test verifies that:
 *
 * - address_bits is either 32 or 64
 * - cpu_byteorder is ``little`` or ``big`` and matches the host byte order
 * - rcu_expedited and rcu_normal are booleans (0 or 1)
 *
 * All checks skip gracefully with TCONF when a particular attribute is not
 * present, since these files were added over several kernel releases.
 */

#include <endian.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static const char *const address_bits_allowed[] = {
	"32", "64", NULL
};

static void run(void)
{
	TST_SYSFS_ASSERT_ONEOF(address_bits_allowed, PATH_SYS_KERNEL "/address_bits");

#if __BYTE_ORDER == __LITTLE_ENDIAN
	TST_SYSFS_EXP_EQ_STR("little", PATH_SYS_KERNEL "/cpu_byteorder");
#elif __BYTE_ORDER == __BIG_ENDIAN
	TST_SYSFS_EXP_EQ_STR("big", PATH_SYS_KERNEL "/cpu_byteorder");
#endif

	TST_SYSFS_ASSERT_BOOL(PATH_SYS_KERNEL "/rcu_expedited");
	TST_SYSFS_ASSERT_BOOL(PATH_SYS_KERNEL "/rcu_normal");
}

static struct tst_test test = {
	.test_all = run,
};
