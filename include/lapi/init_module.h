// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef INIT_MODULE_H__
#define INIT_MODULE_H__

#include "config.h"
#include "lapi/syscalls.h"
#include "tst_test.h"

static inline int init_module(void *module_image, unsigned long len,
			      const char *param_values)
{
	return tst_syscall(__NR_init_module, module_image, len, param_values);
}
#endif /* INIT_MODULE_H__ */
