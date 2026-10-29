// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_KCONFIG_H__
#define TST_KCONFIG_H__

#include <stdbool.h>
#include <stddef.h>

/**
 * TST_KCONFIG_INIT() - Initialization helper macro for struct
 *     tst_kconfig_var. Requires <string.h> for strlen().
 * @confname: Config variable name string.
 */
#define TST_KCONFIG_INIT(confname) { \
	.id = confname, \
	.id_len = strlen(confname) \
}

/**
 * struct tst_kconfig_var - Kernel config variable lookup result.
 * @id: Config variable name (e.g. CONFIG_FOO).
 * @id_len: Length of @id string.
 * @choice: Result: 'm', 'y', 'v', 'n', or 0 (not found).
 * @val: Allocated value string when @choice is 'v'.
 */
struct tst_kconfig_var {
	char id[64];
	unsigned int id_len;
	char choice;
	char *val;
};

/**
 * tst_kconfig_read() - Read and parse kernel config.
 * @vars: An array of caller initialized tst_kconfig_var structures.
 * @vars_len: Length of the @vars array.
 *
 * The path to the kernel config is autodetected from known locations
 * and can be overridden with the KCONFIG_PATH environment variable.
 *
 * After a call to this function each choice is set as follows:
 *
 *  - 'm' - config option set to m
 *  - 'y' - config option set to y
 *  - 'v' - config option set to other value
 *  - 'n' - config option is not set
 *  - 0   - config option not found
 *
 * When choice is 'v' the val pointer holds a newly allocated string.
 */
void tst_kconfig_read(struct tst_kconfig_var vars[], size_t vars_len);

/**
 * tst_kconfig_check() - Check if required kernel config options are set.
 * @kconfigs: NULL-terminated array of config strings needed for the testrun.
 *
 * Config options can be passed as "CONFIG_FOO" (must be set) or
 * "CONFIG_FOO=bar" (value must match).
 *
 * Return: 0 if every config is satisfied, 1 if at least one is missing.
 */
int tst_kconfig_check(const char *const kconfigs[]);

/**
 * TST_KCMDLINE_INIT() - Initialization helper macro for struct tst_kcmdline_var.
 * @paraname: Kernel command-line parameter name.
 */
#define TST_KCMDLINE_INIT(paraname) { \
	.key = paraname, \
	.value = "", \
	.found = false \
}

/**
 * struct tst_kcmdline_var - Kernel command-line parameter storage.
 * @key: Parameter name.
 * @value: Parameter value buffer.
 * @found: Whether the parameter was found.
 */
struct tst_kcmdline_var {
	const char *key;
	char value[256];
	bool found;
};

/**
 * tst_kcmdline_parse() - Parse kernel command-line parameters from /proc/cmdline.
 * @params: Array of tst_kcmdline_var structures to fill.
 * @params_len: Length of the @params array.
 */
void tst_kcmdline_parse(struct tst_kcmdline_var params[], size_t params_len);

/*
 * tst_has_slow_kconfig() - Check if any performance-degrading kernel configs are enabled.
 *
 * This function iterates over the list of slow kernel configuration options
 * (`tst_slow_kconfigs`) and checks if any of them are enabled in the running kernel.
 *
 * Return:
 * - 1 if at least one slow kernel config is enabled.
 * - 0 if none of the slow kernel configs are enabled.
 */
int tst_has_slow_kconfig(void);

#endif	/* TST_KCONFIG_H__ */
