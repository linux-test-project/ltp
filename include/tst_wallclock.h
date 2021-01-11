// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */


#ifndef TST_WALLCLK_H__
#define TST_WALLCLK_H__

void tst_wallclock_save(void);

void tst_wallclock_restore(void);

void tst_rtc_clock_save(const char *rtc_dev);

void tst_rtc_clock_restore(const char *rtc_dev);

#endif	/* TST_WALLCLK_H__ */
