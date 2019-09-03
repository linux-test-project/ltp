// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Authors: Jinhui huang <huangjh.jy@cn.fujitsu.com>
 */

#ifndef __LAPI_UINPUT_H_
#define __LAPI_UINPUT_H_

#include <linux/uinput.h>

#ifndef UI_GET_SYSNAME
#define UI_GET_SYSNAME(len)	_IOC(_IOC_READ, UINPUT_IOCTL_BASE, 44, len)
#endif

#endif /* __LAPI_UINPUT_H_ */
