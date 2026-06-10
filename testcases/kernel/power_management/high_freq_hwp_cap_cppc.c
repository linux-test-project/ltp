// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Piotr Kubaj <piotr.kubaj@intel.com>
 */

/*\
 * Verify for all online logical CPUs that their highest performance value are
 * the same for HWP Capability MSR 0x771 and CPPC sysfs file.
 *
 * On HWP-capable x86 platforms the acpi_cppc/highest_perf sysfs attribute is
 * expected to reflect the same highest-performance value that firmware
 * programs into the HWP Capabilities MSR (0x771, bits 7:0). A mismatch
 * between the two interfaces indicates a kernel regression in how CPPC
 * values are exposed to userspace, and would break tools (e.g. cpupower,
 * intel_pstate tuning scripts) that rely on the sysfs interface to make
 * frequency-scaling decisions.
 *
 * The test needs root because reading /dev/cpu/N/msr needs CAP_SYS_RAWIO /
 * root.
 */

#include "tst_test.h"
#include "tst_safe_prw.h"

#if defined(__i386__) || defined(__x86_64__)
# include "lapi/cpuid.h"
#endif

#define MSR_HWP_CAPABILITIES	0x771
#define HIGHEST_PERF_MASK	0xFF

#define CPUID_LEAF_THERMAL	0x6
#define CPUID_HWP_BIT		(1 << 7)

static int nproc;
static int fd = -1;

static void setup(void)
{
#if defined(__i386__) || defined(__x86_64__)
	unsigned int eax, ebx, ecx, edx;

	__cpuid_count(CPUID_LEAF_THERMAL, 0, eax, ebx, ecx, edx);
	if (!(eax & CPUID_HWP_BIT))
		tst_brk(TCONF, "HWP not supported (MSR 0x771 unavailable)");
#endif

	if (access("/dev/cpu/0/msr", F_OK) == -1)
		tst_brk(TCONF | TERRNO, "msr driver not loaded");

	if (access("/sys/devices/system/cpu/cpu0/acpi_cppc/highest_perf", F_OK) == -1)
		tst_brk(TCONF | TERRNO, "CPPC sysfs not available");

	nproc = tst_ncpus_conf();
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static void run(void)
{
	bool status = true;
	char path[PATH_MAX];

	for (int i = 0; i < nproc; i++) {
		int online = 1;
		unsigned long long msr_highest_perf = 0, sysfs_highest_perf = 0;

		if (i) {
			snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/online", i);
			SAFE_FILE_SCANF(path, "%d", &online);
		}

		if (!online) {
			tst_res(TINFO, "CPU%d offline, skipping", i);
			continue;
		}

		snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/acpi_cppc/highest_perf", i);
		SAFE_FILE_SCANF(path, "%llu", &sysfs_highest_perf);
		tst_res(TDEBUG, "%s: %llu", path, sysfs_highest_perf);

		snprintf(path, sizeof(path), "/dev/cpu/%d/msr", i);
		fd = SAFE_OPEN(path, O_RDONLY);

		SAFE_PREAD(1, fd, &msr_highest_perf, sizeof(msr_highest_perf), MSR_HWP_CAPABILITIES);
		SAFE_CLOSE(fd);
		msr_highest_perf &= HIGHEST_PERF_MASK;
		tst_res(TDEBUG, "%s: %llu", path, msr_highest_perf);

		bool match = msr_highest_perf == sysfs_highest_perf;

		if (!match) {
			tst_res(TINFO, "cpu%d: sysfs=%llu MSR=%llu",
				i, sysfs_highest_perf, msr_highest_perf);
			status = false;
		}

		tst_res(TINFO, "cpu%d: %s", i, match ? "OK" : "MISMATCH");
	}

	if (status)
		tst_res(TPASS, "Sysfs and MSR values are equal");
	else
		tst_res(TFAIL, "Highest performance values differ between sysfs and MSR");
}

static struct tst_test test = {
	.needs_cpu_vendor = "GenuineIntel",
	.needs_kconfigs = (const char *const []) {
		"CONFIG_ACPI_CPPC_LIB",
		"CONFIG_X86_MSR",
		NULL
	},
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.supported_archs = (const char *const []) {
		"x86",
		"x86_64",
		NULL
	},
	.test_all = run
};
