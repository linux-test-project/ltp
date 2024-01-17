/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_KERNEL_H__
#define TST_KERNEL_H__

/*
 * Returns 32 if we are running on 32bit kernel and 64 if on 64bit kernel.
 */
int tst_kernel_bits(void);

/*
 * Returns non-zero if the test process is running in compat mode.
 */
int tst_is_compat_mode(void);

/*
 * Checks if the kernel module is built-in.
 *
 * @param driver The name of the driver.
 * @return Returns 0 if builtin driver
 * -1 when driver is missing or config file not available.
 * On Android *always* 0 (always expect the driver is available).
 */
int tst_check_builtin_driver(const char *driver);

/*
 * Checks support for the kernel module (both built-in and loadable).
 *
 * @param driver The name of the driver.
 * @return Returns 0 if the kernel has the driver,
 * -1 when driver is missing or config file not available.
 * On Android *always* 0 (always expect the driver is available).
 */
int tst_check_driver(const char *driver);

#endif	/* TST_KERNEL_H__ */
