/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 Jan Stancek <jstancek@redhat.com>
 */

#ifndef TST_SYS_CONF_H__
#define TST_SYS_CONF_H__

/**
 * TST_SR_TCONF_MISSING - End test with :c:enum:`TCONF <tst_res_flags>` if the
 * file does not exist.
 */
#define TST_SR_TCONF_MISSING 0x0

/**
 * TST_SR_TBROK_MISSING - End test with :c:enum:`TBROK <tst_res_flags>` if the
 * file does not exist.
 */
#define TST_SR_TBROK_MISSING 0x1

/**
 * TST_SR_SKIP_MISSING - Continue without saving the file if it does not exist.
 */
#define TST_SR_SKIP_MISSING 0x2

/**
 * TST_SR_TCONF_RO - End test with :c:enum:`TCONF <tst_res_flags>` if the file
 * is read-only.
 */
#define TST_SR_TCONF_RO 0x0

/**
 * TST_SR_TBROK_RO - End test with :c:enum:`TBROK <tst_res_flags>` if the file
 * is read-only.
 */
#define TST_SR_TBROK_RO 0x4

/**
 * TST_SR_SKIP_RO - Continue without saving the file if it is read-only.
 */
#define TST_SR_SKIP_RO 0x8

/**
 * TST_SR_IGNORE_ERR - Ignore all errors during reading and writing the file.
 */
#define TST_SR_IGNORE_ERR 0x10

/**
 * TST_SR_TCONF - Equivalent to :ref:`TST_SR_TCONF_MISSING` |
 * :ref:`TST_SR_TCONF_RO`.
 */
#define TST_SR_TCONF (TST_SR_TCONF_MISSING | TST_SR_TCONF_RO)

/**
 * TST_SR_TBROK - Equivalent to :ref:`TST_SR_TBROK_MISSING` |
 * :ref:`TST_SR_TBROK_RO`.
 */
#define TST_SR_TBROK (TST_SR_TBROK_MISSING | TST_SR_TBROK_RO)

/**
 * TST_SR_SKIP - Equivalent to :ref:`TST_SR_SKIP_MISSING` |
 * :ref:`TST_SR_SKIP_RO`.
 */
#define TST_SR_SKIP (TST_SR_SKIP_MISSING | TST_SR_SKIP_RO)

/**
 * struct tst_path_val - Saving and restoring /proc|sys values.
 *
 * @path: A file in /proc|sys.
 * @val: If non-NULL string it will be saved to path.
 * @flags: :ref:`TST_SR_* <TST_SR_TCONF_MISSING>` flags to modify the behavior.
 */
struct tst_path_val {
	const char *path;
	const char *val;
	unsigned int flags;
};

void tst_sys_conf_save_str(const char *path, const char *value);
int tst_sys_conf_save(const struct tst_path_val *conf);
void tst_sys_conf_restore(int verbose);
void tst_sys_conf_dump(void);

int tst_read_bool_sys_param(const char *filename);

/**
 * TST_SYS_CONF_LONG_SET() - Writes a long int into a sys or proc file.
 *
 * @path: A path to a sysfs or a procfs file.
 * @val: A long int value to be written to the file.
 * @check: If non-zero the library reads the file back and checks that the
 *         value is the one we have written there. If not the library calls
 *         tst_brk(TBROK, ...).
 *
 * Sets a sysfs or procfs file and optionally checks that it was set correctly.
 */
#define TST_SYS_CONF_LONG_SET(path, val, check) \
	tst_sys_conf_long_set_(__FILE__, __LINE__, path, val, check)

void tst_sys_conf_long_set_(const char *file, const int lineno,
			    const char *path, long val, int check);


/**
 * TST_SYS_CONF_LONG_GET() - Reads a long int from sys or proc file.
 *
 * @path: A path to a sysfs or a procfs file.
 * return: A value read from the file converted into a long.
 *
 * Gets a sysfs or procfs file value and converts it to long.
 */
#define TST_SYS_CONF_LONG_GET(path) \
	tst_sys_conf_long_get_(__FILE__, __LINE__, path)

long tst_sys_conf_long_get_(const char *file, const int lineno,
			    const char *path);

#endif
