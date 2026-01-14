// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2009 Cisco Systems, Inc.  All Rights Reserved.
 * Copyright (c) 2009 FUJITSU LIMITED.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2026
 *
 * Author: Liu Bo <liubo2009@cn.fujitsu.com>
 * Author: Ngie Cooper <yaneurabeya@gmail.com>
 */

#ifndef __LTP_SIGNAL_H
#define __LTP_SIGNAL_H

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include "config.h"

/*
 * For all but __mips__:
 *
 * _COMPAT_NSIG / _COMPAT_NSIG_BPW == 2.
 *
 * For __mips__:
 *
 * _COMPAT_NSIG / _COMPAT_NSIG_BPW == 4.
 *
 * See asm/compat.h under the kernel source for more details.
 *
 * Multiply that by a fudge factor of 4 and you have your SIGSETSIZE.
 */
#if defined __mips__
#define SIGSETSIZE 16
#else
#define SIGSETSIZE (_NSIG / 8)
#endif

#endif
