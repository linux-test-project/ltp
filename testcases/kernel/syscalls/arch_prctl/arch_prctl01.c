// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) UnionTech Software Technology Co.,Ltd., 2024
 * Author: Lu Fei <lufei@uniontech.com>
 */

/*\
 * [Description]
 *
 * Simple test on arch_prctl to set and get cpuid instruction of test thread.
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "lapi/syscalls.h"
#include "lapi/arch_prctl.h"
#include <stdlib.h>
#include <string.h>

static int arch_prctl_get(int code)
{
	return tst_syscall(__NR_arch_prctl, code, NULL);
}

static int arch_prctl_set(int code, unsigned long addr)
{
	return tst_syscall(__NR_arch_prctl, code, addr);
}

static bool tag;

static void setup(void)
{
	FILE *f;
	char flag_mid[] = " cpuid_fault ";
	char flag_end[] = " cpuid_fault\n";
	char *line = NULL;
	size_t len = 0;

	tag = false;

	f = SAFE_FOPEN("/proc/cpuinfo", "r");

	while (getline(&line, &len, f) != -1)
		if (strncmp(line, "flags", strlen("flags")) == 0 &&
				(strstr(line, flag_mid) != NULL ||
				 strstr(line, flag_end) != NULL)) {
			tag = true;
			break;
		}
}

static void run(unsigned int index)
{
	if (tag)
		TST_EXP_PASS(arch_prctl_set(ARCH_SET_CPUID, index));
	else
		TST_EXP_FAIL(arch_prctl_set(ARCH_SET_CPUID, index), ENODEV);

	// if cpu has cpuid_fault flag, ARCH_GET_CPUID returns what has been
	// set: index, otherwise, returns default status: 1
	int exp = tag ? index : 1;

	TEST(arch_prctl_get(ARCH_GET_CPUID));
	if (TST_RET == exp)
		tst_res(TPASS, "get cpuid succeed.");
	else
		tst_res(TFAIL, "get wrong cpuid status");
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.tcnt = 2,
	.min_kver = "4.12",
	.supported_archs = (const char *const []){"x86_64", "x86", NULL}
};
