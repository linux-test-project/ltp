/*
 * Copyright (c) 2009 Cisco Systems, Inc.  All Rights Reserved.
 * Copyright (c) 2009 FUJITSU LIMITED.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Author: Liu Bo <liubo2009@cn.fujitsu.com>
 * Author: Ngie Cooper <yaneurabeya@gmail.com>
 *
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
