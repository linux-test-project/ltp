// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2014-2019
 */

#ifndef LAPI_POSIX_TYPES_H__
#define LAPI_POSIX_TYPES_H__

#include <linux/posix_types.h>

#ifndef __kernel_long_t
# if defined(__x86_64__) && defined(__ILP32__)
typedef long long		__kernel_long_t;
typedef unsigned long long	__kernel_ulong_t;
# else
typedef long			__kernel_long_t;
typedef unsigned long		__kernel_ulong_t;
# endif
#endif

#endif /* LAPI_POSIX_TYPES_H__ */
