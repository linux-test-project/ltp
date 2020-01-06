// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author(s): Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef KSM_HELPER_H
#define KSM_HELPER_H

#define PATH_KSM	"/sys/kernel/mm/ksm/"

void wait_ksmd_full_scan(void);

#endif /* KSM_HELPER_H */
