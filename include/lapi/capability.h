/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
 */

#ifndef LAPI_CAPABILITY_H__
#define LAPI_CAPABILITY_H__

#include "config.h"

#ifdef HAVE_SYS_CAPABILITY_H
# include <sys/capability.h>
/**
 * Some old libcap-devel(1.96~2.16) define _LINUX_TYPES_H in
 * sys/capability.h that makes ltp-lib cann't include linux/types.h
 * essentially. Here undefine it if include such old header-file.
 */
# ifndef HAVE_NEWER_LIBCAP
#  undef _LINUX_TYPES_H
# endif
#endif

#ifndef CAP_NET_RAW
# define CAP_NET_RAW          13
#endif

#ifndef CAP_IPC_LOCK
# define CAP_IPC_LOCK         14
#endif

#ifndef CAP_SYS_CHROOT
# define CAP_SYS_CHROOT       18
#endif

#ifndef CAP_SYS_ADMIN
# define CAP_SYS_ADMIN        21
#endif

#ifndef CAP_SYS_NICE
# define CAP_SYS_NICE         23
#endif

#ifndef CAP_SYS_TIME
# define CAP_SYS_TIME         25
#endif

#ifndef CAP_AUDIT_READ
# define CAP_AUDIT_READ       37
#endif

#ifndef CAP_SYS_RESOURCE
# define CAP_SYS_RESOURCE     24
#endif

#ifndef CAP_BPF
# define CAP_BPF              39
#endif

#ifndef CAP_TO_INDEX
# define CAP_TO_INDEX(x)     ((x) >> 5)
#endif

#ifndef CAP_TO_MASK
# define CAP_TO_MASK(x)      (1 << ((x) & 31))
#endif

#endif /* LAPI_CAPABILITY_H__ */
