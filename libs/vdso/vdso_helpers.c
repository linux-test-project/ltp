// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar<viresh.kumar@linaro.org>
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

#include "tse_parse_vdso.h"
#include "config.h"

#ifdef HAVE_GETAUXVAL
# include <sys/auxv.h>
#endif /* HAVE_GETAUXVAL */

static unsigned long sysinfo_ehdr;

static void vdso_init(void)
{
#ifdef HAVE_GETAUXVAL
	if (sysinfo_ehdr)
		return;

	sysinfo_ehdr = getauxval(AT_SYSINFO_EHDR);
	if (!sysinfo_ehdr) {
		tst_res(TINFO, "Couldn't find AT_SYSINFO_EHDR");
		return;
	}

	vdso_init_from_sysinfo_ehdr(sysinfo_ehdr);
#else
	tst_res(TINFO, "getauxval() not supported");
#endif /* HAVE_GETAUXVAL */
}

void find_clock_gettime_vdso(gettime_t *ptr_vdso_gettime,
			     gettime_t *ptr_vdso_gettime64)
{
	/*
	 * Some vDSO exports its clock_gettime() implementation with a different
	 * name and version from other architectures, so we need to handle it as
	 * a special case.
	 */
#if defined(__powerpc__) || defined(__powerpc64__)
	const char *version = "LINUX_2.6.15";
	const char *name = "__kernel_clock_gettime";
#elif defined(__aarch64__)
	const char *version = "LINUX_2.6.39";
	const char *name = "__kernel_clock_gettime";
#elif defined(__s390__)
	const char *version = "LINUX_2.6.29";
	const char *name = "__kernel_clock_gettime";
#elif defined(__nds32__)
	const char *version = "LINUX_4";
	const char *name = "__vdso_clock_gettime";
#elif defined(__riscv)
	const char *version = "LINUX_4.15";
	const char *name = "__vdso_clock_gettime";
#else
	const char *version = "LINUX_2.6";
	const char *name = "__vdso_clock_gettime";
#endif

	const char *version64 = "LINUX_2.6";
	const char *name64 = "__vdso_clock_gettime64";

	vdso_init();

	*ptr_vdso_gettime = vdso_sym(version, name);
	if (!*ptr_vdso_gettime)
		tst_res(TINFO, "Couldn't find vdso_gettime()");

	*ptr_vdso_gettime64 = vdso_sym(version64, name64);
	if (!*ptr_vdso_gettime64)
		tst_res(TINFO, "Couldn't find vdso_gettime64()");
}
