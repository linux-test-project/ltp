// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Federico Bonfiglio fedebonfi95@gmail.com
 */

#ifndef IOCTL_NS_H__
#define IOCTL_NS_H__

#include <asm-generic/ioctl.h>

#ifndef NSIO
#define NSIO	0xb7
#endif
#ifndef NS_GET_PARENT
#define NS_GET_PARENT		_IO(NSIO, 0x2)
#endif
#ifndef NS_GET_OWNER_UID
#define NS_GET_OWNER_UID	_IO(NSIO, 0x4)
#endif
#ifndef NS_GET_USERNS
#define NS_GET_USERNS		_IO(NSIO, 0x1)
#endif
#ifndef NS_GET_NSTYPE
#define NS_GET_NSTYPE		_IO(NSIO, 0x3)
#endif


#endif /* IOCTL_NS_H__ */
