// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>
 */

#ifndef LAPI_UINPUT_H__
#define LAPI_UINPUT_H__

#include <linux/uinput.h>

#ifndef UI_GET_SYSNAME
# define UI_GET_SYSNAME(len)	_IOC(_IOC_READ, UINPUT_IOCTL_BASE, 44, len)
#endif

#endif	/* LAPI_UINPUT_H__ */
