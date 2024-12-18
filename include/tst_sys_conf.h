/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 Jan Stancek <jstancek@redhat.com>
 */

#ifndef TST_SYS_CONF_H__
#define TST_SYS_CONF_H__

#define TST_SR_TCONF_MISSING 0x0
#define TST_SR_TBROK_MISSING 0x1
#define TST_SR_SKIP_MISSING 0x2
#define TST_SR_TCONF_RO 0x0
#define TST_SR_TBROK_RO 0x4
#define TST_SR_SKIP_RO 0x8
#define TST_SR_IGNORE_ERR 0x10

#define TST_SR_TCONF (TST_SR_TCONF_MISSING | TST_SR_TCONF_RO)
#define TST_SR_TBROK (TST_SR_TBROK_MISSING | TST_SR_TBROK_RO)
#define TST_SR_SKIP (TST_SR_SKIP_MISSING | TST_SR_SKIP_RO)

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
