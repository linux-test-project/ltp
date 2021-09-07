// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Author: Xie Ziyao <ziyaoxie@outlook.com>
 */

#ifndef TST_BITMAP_H__
#define TST_BITMAP_H__

/*
 * Check whether the n-th bit of val is set
 * @return 0: the n-th bit of val is 0, 1: the n-th bit of val is 1
 */
#define TST_IS_BIT_SET(val, n) (((val) & (1<<(n))) >> (n))

#endif /* TST_BITMAP_H__ */
