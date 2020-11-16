// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Linux Test Project
 */

#ifndef LAPI_FUTEX_H__
#define LAPI_FUTEX_H__

#include <stdint.h>

typedef volatile uint32_t futex_t;

#if !defined(SYS_futex) && defined(SYS_futex_time64)
#define SYS_futex SYS_futex_time64
#endif

#endif /* LAPI_FUTEX_H__ */
