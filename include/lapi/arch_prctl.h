// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Cyril Hrubis <chrubis@suse.cz>
 */
#ifndef LAPI_ARCH_PRCTL_H__
#define LAPI_ARCH_PRCTL_H__

#include "config.h"

#ifdef HAVE_ASM_PRCTL_H
# include <asm/prctl.h>
#endif

#ifndef ARCH_GET_CPUID
# define ARCH_GET_CPUID 0x1011
#endif

#ifndef ARCH_SET_CPUID
# define ARCH_SET_CPUID 0x1012
#endif

#endif /* LAPI_ARCH_PRCTL_H__ */
