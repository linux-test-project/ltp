// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Michael Moese <mmoese@suse.de>
 */

/* Usage example
 *
 * ...
 * #include "tst_test.h"
 * ..
 * static struct tst_test test = {
 *	...
 *	.taint_check = TST_TAINT_W | TST_TAINT_D,
 *	...
 * };
 *
 * void run(void)
 * {
 *	...
 *	. test code here
 *	...
 *	if (tst_taint_check() != 0)
 *		tst_res(TFAIL, "kernel has issues");
 *	else
 *		tst_res(TPASS, "kernel seems to be fine");
 * }
 *
 *
 *
 * The above code checks whether the kernel issued a warning (TST_TAINT_W)
 * or even died (TST_TAINT_D) during test execution.
 * If these are set after running a test case, we most likely
 * triggered a kernel bug.
 *
 * You do not need to use tst_taint_check() explicitly because it'll be called
 * automatically at the end of testing by the LTP library if
 * tst_test.taint_check in non-zero.
 */

#ifndef TST_TAINTED_H__
#define TST_TAINTED_H__

/*
 * This are all 17 flags that are present in kernel 4.15
 * see kernel/panic.c in kernel sources
 *
 * Not all of them are valid in all kernel versions.
 */
#define TST_TAINT_G     (1 <<  0) /* a module with non-GPL license loaded */
#define TST_TAINT_F     (1 <<  1) /* a module was force-loaded */
#define TST_TAINT_S     (1 <<  2) /* SMP with Non-SMP kernel */
#define TST_TAINT_R     (1 <<  3) /* module force unloaded */
#define TST_TAINT_M     (1 <<  4) /* machine check error occurred */
#define TST_TAINT_B     (1 <<  5) /* page-release function found bad page */
#define TST_TAINT_U     (1 <<  6) /* user requested taint flag */
#define TST_TAINT_D     (1 <<  7) /* kernel died recently - OOPS or BUG */
#define TST_TAINT_A     (1 <<  8) /* ACPI table has been overwritten */
#define TST_TAINT_W     (1 <<  9) /* a warning has been issued by kernel */
#define TST_TAINT_C     (1 << 10) /* driver from drivers/staging was loaded */
#define TST_TAINT_I     (1 << 11) /* working around BIOS/Firmware bug */
#define TST_TAINT_O     (1 << 12) /* out of tree module loaded */
#define TST_TAINT_E     (1 << 13) /* unsigned module was loaded */
#define TST_TAINT_L     (1 << 14) /* A soft lock-up has previously occurred */
#define TST_TAINT_K     (1 << 15) /* kernel has been live-patched */
#define TST_TAINT_X     (1 << 16) /* auxiliary taint, for distro's use */
#define TST_TAINT_T     (1 << 17) /* kernel was built with the struct randomization plugin */
#define TST_TAINT_N     (1 << 18) /* an in-kernel test has been run */

/**
 * tst_taint_init() - Set up kernel taint checking.
 * @mask: Bitmask of TST_TAINT_* flags to monitor.
 *
 * Called automatically by the LTP library during test setup when
 * tst_test.taint_check is non-zero. Aborts with TBROK if any
 * requested flags are already set.
 */
void tst_taint_init(unsigned int mask);


/**
 * tst_taint_check() - Check if monitored taint flags have been set.
 *
 * May only be called after tst_taint_init(). Also called automatically
 * at the end of testing when tst_test.taint_check is non-zero.
 *
 * Return: 0 if clean, or a bitmask of newly set taint flags.
 */
unsigned int tst_taint_check(void);


#endif /* TST_TAINTED_H__ */
