// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_INIT_MODULE_H__
#define LAPI_INIT_MODULE_H__

#include "config.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

static inline int init_module(void *module_image, unsigned long len,
			      const char *param_values)
{
	return tst_syscall(__NR_init_module, module_image, len, param_values);
}

static inline int finit_module(int fd, const char *param_values, int flags)
{
	return tst_syscall(__NR_finit_module, fd, param_values, flags);
}

#endif /* LAPI_INIT_MODULE_H__ */
