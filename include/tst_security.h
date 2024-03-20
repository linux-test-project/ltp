/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) Linux Test Project, 2020-2024
 */

#ifndef TST_SECURITY_H__
#define TST_SECURITY_H__

/*
 * Detect whether FIPS enabled
 * @return 0: FIPS not enabled, 1: FIPS enabled
 */
int tst_fips_enabled(void);

int tst_lockdown_enabled(void);
int tst_secureboot_enabled(void);

#endif /* TST_SECURITY_H__ */
