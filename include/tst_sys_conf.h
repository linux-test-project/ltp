/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 Jan Stancek <jstancek@redhat.com>
 */

#ifndef TST_SYS_CONF_H__
#define TST_SYS_CONF_H__

struct tst_sys_conf {
	char path[PATH_MAX];
	char value[PATH_MAX];
	struct tst_sys_conf *next;
};

int tst_sys_conf_save_str(const char *path, const char *value);
int tst_sys_conf_save(const char *path);
void tst_sys_conf_restore(int verbose);
void tst_sys_conf_dump(void);

#endif
