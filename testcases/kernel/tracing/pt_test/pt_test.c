// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2018 Intel Corporation
 * Author: Ammy Yi (ammy.yi@intel.com)
 */

/*
 * This test will check if Intel PT(Intel Processer Trace) is working.
 *
 * Intel CPU of 5th-generation Core (Broadwell) or newer is required for the test.
 *
 * kconfig requirement: CONFIG_PERF_EVENTS
 */


#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "config.h"

#ifdef HAVE_STRUCT_PERF_EVENT_MMAP_PAGE_AUX_HEAD
# include <linux/perf_event.h>

#define PAGESIZE 4096
#define INTEL_PT_MEMSIZE (17*PAGESIZE)

#define BIT(nr)                 (1UL << (nr))

#define INTEL_PT_PATH "/sys/devices/intel_pt"
#define INTEL_PT_PMU_TYPE "/sys/devices/intel_pt/type"
#define INTEL_PT_FORMAT_TSC "/sys/devices/intel_pt/format/tsc"
#define INTEL_PT_FORMAT_NRT "/sys/devices/intel_pt/format/noretcomp"

//Intel PT event handle
int fde = -1;
//map head and size
uint64_t **bufm;
long buhsz;
static char *str_mode;
static char *str_exclude_info;
static char *str_branch_flag;
int mode = 1;

static uint64_t **create_map(int fde, long bufsize, int flag)
{
	uint64_t **buf_ev;
	int pro_flag;
	struct perf_event_mmap_page *pc;

	buf_ev = SAFE_MALLOC(2*sizeof(uint64_t *));
	buf_ev[0] = NULL;
	buf_ev[1] = NULL;
	if (flag == 1) {
		tst_res(TINFO, "Memory will be r/w for full trace mode");
		pro_flag = PROT_READ | PROT_WRITE;
	} else {
		tst_res(TINFO, "Memory will be r only for snapshot mode");
		pro_flag = PROT_READ;
	}
	buf_ev[0] = SAFE_MMAP(NULL, INTEL_PT_MEMSIZE, PROT_READ | PROT_WRITE,
							MAP_SHARED, fde, 0);

	tst_res(TINFO, "Open Intel PT event failed");
	pc = (struct perf_event_mmap_page *)buf_ev[0];
	pc->aux_offset = INTEL_PT_MEMSIZE;
	pc->aux_size = bufsize;
	buf_ev[1] = SAFE_MMAP(NULL, bufsize, pro_flag,
					MAP_SHARED, fde, INTEL_PT_MEMSIZE);
	return buf_ev;
}


int intel_pt_pmu_value(char *dir)
{
	char *value;
	int val = 0;
	char delims[] = ":";

	SAFE_FILE_SCANF(dir, "%m[^\n]", &value);
	if (strstr(value, delims) == NULL) {
		val = atoi(value);
	} else {
		strsep(&value, delims);
		val = atoi(value);
	}
	return val;
}

static void del_map(uint64_t **buf_ev, long bufsize)
{
	if (buf_ev) {
		if (buf_ev[0])
			munmap(buf_ev[0], INTEL_PT_MEMSIZE);
		if (buf_ev[1])
			munmap(buf_ev[1], bufsize);
	}

	free(buf_ev);
}

static void intel_pt_trace_check(void)
{
	uint64_t aux_head = 0;
	struct perf_event_mmap_page *pmp;
	/* enable tracing */
	SAFE_IOCTL(fde, PERF_EVENT_IOC_RESET);
	SAFE_IOCTL(fde, PERF_EVENT_IOC_ENABLE);

	/* stop tracing */
	SAFE_IOCTL(fde, PERF_EVENT_IOC_DISABLE);

	/* check if there is some trace generated */
	pmp = (struct perf_event_mmap_page *)bufm[0];
	aux_head = *(volatile uint64_t *)&pmp->aux_head;
	if (aux_head == 0) {
		tst_res(TFAIL, "There is no trace");
		return;
	}

	tst_res(TPASS, "perf trace test passed");
}

static int is_affected_by_erratum_BDM106(void)
{
	int family = -1, model = -1;

	if (FILE_LINES_SCANF("/proc/cpuinfo", "cpu family%*s%d", &family)
		|| family != 6)
		return 0;

	if (!FILE_LINES_SCANF("/proc/cpuinfo", "model%*s%d", &model)) {
		tst_res(TINFO, "Intel FAM6 model %d", model);

		switch (model) {
		case 0x3D: /* INTEL_FAM6_BROADWELL */
		/* fallthrough */
		case 0x47: /* INTEL_FAM6_BROADWELL_G */
		/* fallthrough */
		case 0x4F: /* INTEL_FAM6_BROADWELL_X */
		/* fallthrough */
		case 0x56: /* INTEL_FAM6_BROADWELL_D */
			return 1;
		}
	}

	return 0;
}

static void setup(void)
{
	struct perf_event_attr attr = {};

	buhsz = 2 * PAGESIZE;

	if (access(INTEL_PT_PATH, F_OK)) {
		tst_brk(TCONF,
			"Requires Intel Core 5th+ generation (Broadwell and newer) and CONFIG_PERF_EVENTS enabled");
	}

	/* set attr for Intel PT trace */
	attr.type	= intel_pt_pmu_value(INTEL_PT_PMU_TYPE);
	attr.read_format = PERF_FORMAT_ID | PERF_FORMAT_TOTAL_TIME_RUNNING |
				PERF_FORMAT_TOTAL_TIME_ENABLED;
	attr.disabled	= 1;
	attr.config	= BIT(intel_pt_pmu_value(INTEL_PT_FORMAT_TSC)) |
				BIT(intel_pt_pmu_value(INTEL_PT_FORMAT_NRT));
	attr.size	= sizeof(struct perf_event_attr);
	attr.mmap			= 1;
	if (str_branch_flag) {
		if (is_affected_by_erratum_BDM106()) {
			tst_brk(TCONF, "erratum BDM106 disallows not "
				"setting BRANCH_EN on this CPU");
		}

		tst_res(TINFO, "Intel PT will disable branch trace");
		attr.config |= 1;
	}

	attr.exclude_kernel	= 0;
	attr.exclude_user	= 0;
	if (str_exclude_info) {
		if (!strcmp(str_exclude_info, "user")) {
			tst_res(TINFO, "Intel PT will exclude user trace");
			attr.exclude_user = 1;
		} else if (!strcmp(str_exclude_info, "kernel")) {
			tst_res(TINFO, "Intel PT will exclude kernel trace");
			attr.exclude_kernel = 1;
		} else {
			tst_brk(TBROK, "Invalid -e '%s'", str_exclude_info);
		}
	}

	/* only get trace for own pid */
	fde = tst_syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
	if (fde < 0) {
		tst_res(TINFO, "Open Intel PT event failed");
		tst_res(TFAIL, "perf trace full mode failed");
		return;
	}
	bufm = NULL;
	if (str_mode)
		mode = 0;

	bufm = create_map(fde, buhsz, mode);
}

static void cleanup(void)
{
	if (fde != -1)
		close(fde);

	del_map(bufm, buhsz);
}

static struct tst_option options[] = {
	{"m", &str_mode, "-m different mode, default is full mode"},
	{"e:", &str_exclude_info, "-e exclude info, user or kernel"},
	{"b", &str_branch_flag, "-b if disable branch trace"},
	{NULL, NULL, NULL}
};


static struct tst_test test = {
	.test_all = intel_pt_trace_check,
	.options = options,
	.min_kver = "4.1",
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
};

#else
TST_TEST_TCONF("Missing aux_* fields in struct perf_event_mmap_page");
#endif /* HAVE_STRUCT_PERF_EVENT_MMAP_PAGE_AUX_HEAD */
