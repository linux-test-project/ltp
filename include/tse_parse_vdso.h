// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef TSE_PARSE_VDSO_H__
#define TSE_PARSE_VDSO_H__

#include <stdint.h>

/*
 * To use this vDSO parser, first call one of the vdso_init_* functions.
 * If you've already parsed auxv, then pass the value of AT_SYSINFO_EHDR
 * to vdso_init_from_sysinfo_ehdr.  Otherwise pass auxv to vdso_init_from_auxv.
 * Then call vdso_sym for each symbol you want.  For example, to look up
 * gettimeofday on x86_64, use:
 *
 *     <some pointer> = vdso_sym("LINUX_2.6", "gettimeofday");
 * or
 *     <some pointer> = vdso_sym("LINUX_2.6", "__vdso_gettimeofday");
 *
 * vdso_sym will return 0 if the symbol doesn't exist or if the init function
 * failed or was not called.  vdso_sym is a little slow, so its return value
 * should be cached.
 *
 * vdso_sym is threadsafe; the init functions are not.
 *
 * These are the prototypes:
 */

#include <time.h>

extern void vdso_init_from_auxv(void *auxv);
extern void vdso_init_from_sysinfo_ehdr(uintptr_t base);
extern void *vdso_sym(const char *version, const char *name);

typedef int (*gettime_t)(clockid_t clk_id, void *ts);
void find_clock_gettime_vdso(gettime_t *ptr_vdso_gettime,
			     gettime_t *ptr_vdso_gettime64);
#endif /* TSE_PARSE_VDSO_H__ */
