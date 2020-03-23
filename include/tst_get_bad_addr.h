/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef TST_GET_BAD_ADDR_H__
#define TST_GET_BAD_ADDR_H__

/* Functions from lib/tst_get_bad_addr.c */
void *tst_get_bad_addr(void (*cleanup_fn) (void));

#endif	/* TST_GET_BAD_ADDR_H__ */
