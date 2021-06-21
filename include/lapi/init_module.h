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

static inline int finit_module(int fd, const char *param_values, int flags)
{
	return tst_syscall(__NR_finit_module, fd, param_values, flags);
}

static inline void finit_module_supported_by_kernel(void)
{
       long ret;

       if ((tst_kvercmp(3, 8, 0)) < 0) {
               /* Check if the syscall is backported on an older kernel */
               ret = syscall(__NR_finit_module, 0, "", 0);
               if (ret == -1 && errno == ENOSYS)
                       tst_brk(TCONF, "Test not supported on kernel version < v3.8");
       }
}

#endif /* INIT_MODULE_H__ */
