// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Piotr Kubaj <piotr.kubaj@intel.com>
 */

/*\
 * Validate presence and permissions of RFIM attributes.
 * The kernel exposes the dvfs, fivr and dlvr attribute groups
 * independently, gated on per-device feature bits, so the test checks
 * whichever of the three groups the hardware presents.
 *
 * Permissions are verified by opening each node: read-only attributes must
 * accept `O_RDONLY` and reject a write open, while read-write ones must accept
 * `O_RDWR`. For sysfs nodes the mode bits alone are not authoritative - the
 * kernel can still refuse the open in the file's own syscall handler - so
 * :manpage:`open(2)` is what actually exercises access.
 *
 * The test requires root because the read-write RFIM attributes are root-owned
 * and created mode 0644, so only the owner holds the write bit. An
 * unprivileged user could read them but would get `EACCES` when opening them
 * `O_RDWR`, which the read-write check would misreport as a failure rather
 * than reflecting the actual access policy. The read-only attributes are mode
 * 0444 and world-readable.
 */

#include "tst_test.h"

#define RFIM_ROOT "/sys/bus/pci/devices/0000:00:04.0"

enum rfim_group {
	GROUP_DVFS,
	GROUP_FIVR,
	GROUP_DLVR,
};

static const char * const group_name[] = {
	[GROUP_DVFS] = "dvfs",
	[GROUP_FIVR] = "fivr",
	[GROUP_DLVR] = "dlvr",
};

static bool have_group[ARRAY_SIZE(group_name)];

static struct tcase {
	const char *path;
	const bool write;
	enum rfim_group group;
} tcases[] = {
	{ RFIM_ROOT "/fivr/vco_ref_code_lo", 1, GROUP_FIVR },
	{ RFIM_ROOT "/fivr/vco_ref_code_hi", 1, GROUP_FIVR },
	{ RFIM_ROOT "/fivr/spread_spectrum_pct", 1, GROUP_FIVR },
	{ RFIM_ROOT "/fivr/spread_spectrum_clk_enable", 1, GROUP_FIVR },
	{ RFIM_ROOT "/fivr/rfi_vco_ref_code", 1, GROUP_FIVR },
	{ RFIM_ROOT "/fivr/fivr_fffc_rev", 1, GROUP_FIVR },

	{ RFIM_ROOT "/dlvr/dlvr_freq_select", 1, GROUP_DLVR },
	{ RFIM_ROOT "/dlvr/dlvr_rfim_enable", 1, GROUP_DLVR },
	{ RFIM_ROOT "/dlvr/dlvr_spread_spectrum_pct", 1, GROUP_DLVR },
	{ RFIM_ROOT "/dlvr/dlvr_control_mode", 1, GROUP_DLVR },
	{ RFIM_ROOT "/dlvr/dlvr_control_lock", 1, GROUP_DLVR },
	{ RFIM_ROOT "/dlvr/dlvr_hardware_rev", 0, GROUP_DLVR },
	{ RFIM_ROOT "/dlvr/dlvr_freq_mhz", 0, GROUP_DLVR },
	{ RFIM_ROOT "/dlvr/dlvr_pll_busy", 0, GROUP_DLVR },

	{ RFIM_ROOT "/dvfs/rfi_restriction_run_busy", 1, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/rfi_restriction_err_code", 1, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/rfi_restriction_data_rate_base", 1, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/rfi_restriction_data_rate", 1, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/rfi_restriction", 1, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/rfi_disable", 1, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/ddr_data_rate_point_0", 0, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/ddr_data_rate_point_1", 0, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/ddr_data_rate_point_2", 0, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/ddr_data_rate_point_3", 0, GROUP_DVFS },
	{ RFIM_ROOT "/dvfs/ddr_data_rate", 0, GROUP_DVFS },
};

static bool has_group(const char *name)
{
	char path[PATH_MAX];
	struct stat stats;

	snprintf(path, sizeof(path), "%s/%s", RFIM_ROOT, name);

	if (stat(path, &stats)) {
		if (errno == ENOENT)
			return false;
		tst_brk(TBROK | TERRNO, "stat(%s)", path);
	}

	if (!S_ISDIR(stats.st_mode))
		tst_brk(TBROK, "%s exists but is not a directory", path);

	return true;
}

static void setup(void)
{
	bool any = false;

	for (unsigned int i = 0; i < ARRAY_SIZE(group_name); i++) {
		have_group[i] = has_group(group_name[i]);
		any |= have_group[i];
	}

	if (!any)
		tst_brk(TCONF, "No RFIM attribute groups present under %s", RFIM_ROOT);
}

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	if (!have_group[tc->group]) {
		tst_res(TCONF, "%s: %s group not present", tc->path,
			group_name[tc->group]);
		return;
	}

	if (tc->write) {
		int fd = TST_EXP_FD(open(tc->path, O_RDWR));

		if (fd != -1)
			SAFE_CLOSE(fd);
	} else {
		int fd = TST_EXP_FD(open(tc->path, O_RDONLY));

		if (fd != -1)
			SAFE_CLOSE(fd);
		TST_EXP_FAIL2(open(tc->path, O_WRONLY), EACCES, "%s should reject writes", tc->path);
	}
}

static struct tst_test test = {
	.min_kver = "6.4",
	.needs_cpu_vendor = "GenuineIntel",
	.needs_kconfigs = (const char *const []) {
		"CONFIG_INT340X_THERMAL",
		NULL
	},
	.needs_root = 1,
	.supported_archs = (const char *const []) {
		"x86",
		"x86_64",
		NULL
	},
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run
};
