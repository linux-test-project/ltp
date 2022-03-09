/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 Jan Stancek <jstancek@redhat.com>
 */

#ifndef TST_SYS_CONF_H__
#define TST_SYS_CONF_H__

struct tst_path_val {
        const char *path;
        const char *val;
};

int tst_sys_conf_save_str(const char *path, const char *value);
int tst_sys_conf_save(const char *path);
void tst_sys_conf_set(const char *path, const char *value);
void tst_sys_conf_restore(int verbose);
void tst_sys_conf_dump(void);

#endif
