// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Michael Moese <mmoese@suse.de>
 */

/* Usage example
 *
 * ...
 * #include "tst_test.h"
 * #include "tst_taint.h"
 * ..
 * void setup(void)
 * {
 *	...
 *	tst_taint_init(TST_TAINT_W | TST_TAINT_D));
 *	...
 * }
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
 * The above code checks, if the kernel issued a warning (TST_TAINT_W)
 * or even died (TST_TAINT_D) during test execution.
 * If these are set after running a test case, we most likely
 * triggered a kernel bug.
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
#define TST_TAINT_X	(1 << 16) /* auxiliary taint, for distro's use */

/*
 * Initialize and prepare support for checking tainted kernel.
 *
 * supply the mask of TAINT-flags you want to check, for example
 * (TST_TAINT_W | TST_TAINT_D) when you want to check if the kernel issued
 * a warning or even reported it died.
 *
 * This function tests if the requested flags are supported on the
 * locally running kernel. In case the tainted-flags are already set by
 * the kernel, there is no reason to continue and TCONF is generated.
 *
 * The mask must not be zero.
 */
void tst_taint_init(unsigned int mask);


/*
 * check if the tainted flags handed to tst_taint_init() are still not set
 * during or after running the test.
 * Calling this function is only allowed after tst_taint_init() was called,
 * otherwise TBROK will be generated.
 *
 * returns 0 or a bitmask of the flags that currently tainted the kernel.
 */
unsigned int tst_taint_check(void);


#endif /* TST_TAINTED_H__ */
