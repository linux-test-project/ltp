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

#endif
