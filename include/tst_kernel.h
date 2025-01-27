/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2018-2024
 */

#ifndef TST_KERNEL_H__
#define TST_KERNEL_H__

#include <stdbool.h>

/**
 * tst_kernel_bits() - Detect if running on 32bit or 64bit kernel.
 *
 * Return: 32 if the test process is running on 32bit kernel and 64 if on 64bit
 * kernel.
 */
int tst_kernel_bits(void);

/**
 * tst_is_compat_mode() - Detect if running in compat mode.
 *
 * Detect if the test is 32bit binary executed on a 64bit kernel,
 * i.e. we are testing the kernel compat layer.
 *
 * Return: non-zero if the test process is running in compat mode.
 */
int tst_is_compat_mode(void);

/**
 * tst_abi_bits() - Detect if compiled for required kernel ABI.
 *
 * @abi: kernel ABI bits (32 or 64).
 *
 * Return: true if compiled for required ABI or false otherwise.
 */
bool tst_abi_bits(int abi);

/**
 * tst_check_builtin_driver() - Check if the kernel module is built-in.
 *
 * @driver: the name of the driver.
 *
 * Return: 0 if builtin driver or -1 when driver is missing or config file not
 * available. On Android *always* 0 (always expect the driver is available).
 */
int tst_check_builtin_driver(const char *driver);

/**
 * tst_check_driver() - Check support for the kernel module.
 *
 * Check support for the kernel module (both built-in and loadable).
 *
 * @driver: the name of the driver.
 *
 * Return: 0 if the kernel has the driver, -1 when driver is missing or config
 * file not available. On Android *always* 0 (always expect the driver is
 * available).
 */
int tst_check_driver(const char *driver);

/**
 * tst_check_preempt_rt() - Check if the running kernel is RT.
 *
 * Check support for the kernel module (both built-in and loadable).
 *
 * Return: -1 if the kernel is RT, 0 otherwise.
 */
int tst_check_preempt_rt(void);

#endif	/* TST_KERNEL_H__ */
