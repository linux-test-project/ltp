// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef LAPI_PRCTL_H__
# define LAPI_PRCTL_H__

#include <sys/prctl.h>

#ifndef PR_SET_CHILD_SUBREAPER
# define PR_SET_CHILD_SUBREAPER	36
# define PR_GET_CHILD_SUBREAPER	37
#endif

#endif /* LAPI_PRCTL_H__ */
