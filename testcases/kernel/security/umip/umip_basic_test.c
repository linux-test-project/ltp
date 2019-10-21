// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2018 Intel Corporation
 * Author: Neri, Ricardo <ricardo.neri@intel.com>
 *	 Pengfei, Xu   <pengfei.xu@intel.com>
 */

/*
 * This test will check if Intel umip(User-Mode Execution Prevention) is
 * working.
 *
 * Intel CPU of ICE lake or newer is required for the test
 * kconfig requirement:CONFIG_X86_INTEL_UMIP=y
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"

#define CPUINFO_FILE "/proc/cpuinfo"

#define GDT_LEN 10
#define IDT_LEN 10

#ifdef __x86_64__

static void asm_sgdt(void)
{
	unsigned char val[GDT_LEN];

	memset(val, 0, sizeof(val));
	tst_res(TINFO, "TEST sgdt, sgdt result save at [%p]", val);
	asm volatile("sgdt %0\n" : "=m" (val));
	exit(0);
}

static void asm_sidt(void)
{
	unsigned char val[IDT_LEN];

	memset(val, 0, sizeof(val));
	tst_res(TINFO, "TEST sidt, sidt result save at [%p]", val);
	asm volatile("sidt %0\n" : "=m" (val));
	exit(0);
}

static void asm_sldt(void)
{
	unsigned long val;

	tst_res(TINFO, "TEST sldt, sldt result save at [%p]", &val);
	asm volatile("sldt %0\n" : "=m" (val));
	exit(0);
}

static void asm_smsw(void)
{
	unsigned long val;

	tst_res(TINFO, "TEST smsw, smsw result save at [%p]", &val);
	asm volatile("smsw %0\n" : "=m" (val));
	exit(0);
}

static void asm_str(void)
{
	unsigned long val;

	tst_res(TINFO, "TEST str, str result save at [%p]", &val);
	asm volatile("str %0\n" : "=m" (val));
	exit(0);
}

static void verify_umip_instruction(unsigned int n)
{
	int status;
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		tst_no_corefile(0);

		switch (n) {
		case 0:
			asm_sgdt();
			break;
		case 1:
			asm_sidt();
			break;
		case 2:
			asm_sldt();
			break;
		case 3:
			asm_smsw();
			break;
		case 4:
			asm_str();
			break;
		default:
			tst_brk(TBROK, "Invalid tcase parameter: %d", n);
		}
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	switch (n) {
	case 0:
	case 1:
	case 3:
		/* after linux kernel v5.4 mainline, 64bit SGDT SIDT SMSW will return
		   dummy value and not trigger SIGSEGV due to kernel code change */
		if ((tst_kvercmp(5, 4, 0)) >= 0) {
			tst_res(TINFO, "Linux kernel version is v5.4 or after than v5.4");
			if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
				tst_res(TFAIL, "Got SIGSEGV");
				return;
			}
			tst_res(TPASS, "Didn't receive SIGSEGV, child exited with %s",
				tst_strstatus(status));
			return;
		} else
			tst_res(TINFO, "Linux kernel version is before than v5.4");
	}

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "Got SIGSEGV");
		return;
	}
	tst_res(TFAIL, "Didn't receive SIGSEGV, child exited with %s",
		tst_strstatus(status));
}

static void setup(void)
{
	FILE *fp;
	char buf[2048];

	fp = SAFE_FOPEN(CPUINFO_FILE, "r");
	while (!feof(fp)) {
		if (fgets(buf, sizeof(buf), fp) == NULL) {
			SAFE_FCLOSE(fp);
			tst_brk(TCONF, "cpuinfo show: cpu does not support umip");
		}

		if (!strstr(buf, "flags"))
			continue;

		if (strstr(buf, "umip")) {
			tst_res(TINFO, "cpuinfo contains umip, CPU supports umip");
			break;
		} else
			continue;
	}

	SAFE_FCLOSE(fp);
}

static struct tst_test test = {
	.min_kver = "4.1",
	.setup = setup,
	.tcnt = 5,
	.forks_child = 1,
	.test = verify_umip_instruction,
	.needs_kconfigs = (const char *[]){
		"CONFIG_X86_INTEL_UMIP=y",
		NULL
	},
	.needs_root = 1,
};

#else

TST_TEST_TCONF("Tests needs x86_64 CPU");

#endif /* __x86_64__ */
