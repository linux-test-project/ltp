// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>
 */

#ifndef TST_FIPS_H__
#define TST_FIPS_H__

/*
 * Detect whether FIPS enabled
 * @return 0: FIPS not enabled, 1: FIPS enabled
 */
int tst_fips_enabled(void);

#endif /* TST_FIPS_H__ */
