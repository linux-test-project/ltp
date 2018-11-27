// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_KCONFIG_H__
#define TST_KCONFIG_H__

struct tst_kconfig_res {
	char match;
	char *value;
};

/**
 * Reads a kernel config and parses it for values defined in kconfigs array.
 *
 * The path to the kernel config should be autodetected in most of the cases as
 * the code looks for know locations. It can be explicitely set/overrided with
 * the KCONFIG_PATH environment variable as well.
 *
 * The kcofings array is expected to contain strings in a format "CONFIG_FOO"
 * or "CONFIG_FOO=bar". The result array has to be suitably sized to fit the
 * results.
 *
 * @param kconfigs array of config strings to look for
 * @param results array to store results to
 * @param cnt size of the arrays
 *
 * The match in the tst_kconfig_res structure is set as follows:
 *
 *  'm' - config option set to m
 *  'y' - config option set to y
 *  'v' - config option set to other value
 *  'n' - config option is not set
 *   0  - config option not found
 *
 * In the case that match is set to 'v' the value points to a newly allocated
 * string that holds the value.
 */
void tst_kconfig_read(const char *const kconfigs[],
		      struct tst_kconfig_res results[], size_t cnt);

/**
 * Checks if required kernel configuration options are set in the kernel
 * config and exits the test with TCONF if at least one is missing.
 *
 * The config options can be passed in two different formats, either
 * "CONFIG_FOO" in which case the option has to be set in order to continue the
 * test or with an explicit value "CONFIG_FOO=bar" in which case the value has
 * to match.
 *
 * @param kconfigs NULL-terminated array of config strings needed for the testrun.
 */
void tst_kconfig_check(const char *const kconfigs[]);

#endif	/* TST_KCONFIG_H__ */
