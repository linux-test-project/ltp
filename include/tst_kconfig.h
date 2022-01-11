// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_KCONFIG_H__
#define TST_KCONFIG_H__

struct tst_kconfig_var {
	char id[64];
	unsigned int id_len;
	char choice;
	char *val;
};

/**
 *
 * Reads a kernel config, parses it and writes results into an array of
 * tst_kconfig_var structures.
 *
 * The path to the kernel config should be autodetected in most of the cases as
 * the code looks for know locations. It can be explicitely set/overrided with
 * the KCONFIG_PATH environment variable as well.
 *
 * The caller has to initialize the tst_kconfig_var structure. The id has to be
 * filled with config variable name such as 'CONFIG_FOO', the id_len should
 * hold the id string length and the choice and val has to be zeroed.
 *
 * After a call to this function each choice be set as follows:
 *
 *  'm' - config option set to m
 *  'y' - config option set to y
 *  'v' - config option set to other value
 *  'n' - config option is not set
 *   0  - config option not found
 *
 * In the case that match is set to 'v' the val pointer points to a newly
 * allocated string that holds the value.
 *
 * @param vars An array of caller initalized tst_kconfig_var structures.
 * @param vars_len Length of the vars array.
 */
void tst_kconfig_read(struct tst_kconfig_var vars[], size_t vars_len);

/**
 * Checks if required kernel configuration options are set in the kernel
 * config. Return 0 if every config is satisfied and return 1 if at least
 * one is missing.
 *
 * The config options can be passed in two different formats, either
 * "CONFIG_FOO" in which case the option has to be set in order to continue the
 * test or with an explicit value "CONFIG_FOO=bar" in which case the value has
 * to match.
 *
 * @param kconfigs NULL-terminated array of config strings needed for the testrun.
 */
int tst_kconfig_check(const char *const kconfigs[]);

#endif	/* TST_KCONFIG_H__ */
