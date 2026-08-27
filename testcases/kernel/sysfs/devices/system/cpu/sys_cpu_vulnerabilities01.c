// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the CPU vulnerability reports exported under
 * /sys/devices/system/cpu/vulnerabilities/.
 *
 * Each file describes the status of one hardware vulnerability. The kernel
 * always prints a human readable status that starts with one of a few known
 * prefixes. The test verifies that every vulnerability file:
 *
 * - is non-empty
 * - starts with one of the known status prefixes (Not affected, Vulnerable,
 *   Mitigation:, Unknown, Processor vulnerable, KVM:)
 *
 * KVM: is used by itlb_multihit_show_state() in arch/x86/kernel/cpu/bugs.c
 * (guarded by CONFIG_KVM_INTEL, i.e. present on most x86_64 distribution
 * kernels), e.g. ``KVM: Mitigation: VMX disabled`` or ``KVM: Vulnerable``,
 * for hosts where the vulnerability only matters to KVM guests, not the
 * host itself.
 *
 * The test skips with TCONF when the vulnerabilities directory is not present,
 * which is the case on architectures that do not report them.
 */

#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static const char *const known_prefixes[] = {
	"Not affected",
	"Vulnerable",
	"Mitigation:",
	"Unknown",
	"Processor vulnerable",
	"KVM:",
};

static void check_vuln(const char *name)
{
	char status[256] = "";
	unsigned int i;

	TST_SYSFS_READ_STR(status, sizeof(status), PATH_SYS_CPU_VULN "/%s", name);

	if (status[0] == '\0') {
		tst_res(TFAIL, "%s/%s is empty", PATH_SYS_CPU_VULN, name);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(known_prefixes); i++) {
		if (!strncmp(status, known_prefixes[i],
			     strlen(known_prefixes[i]))) {
			tst_res(TPASS, "%s: '%s'", name, status);
			return;
		}
	}

	tst_res(TFAIL, "%s has unexpected status '%s'", name, status);
}

static void do_test(void)
{
	DIR *d;
	struct dirent *ent;

	if (access(PATH_SYS_CPU_VULN, F_OK)) {
		tst_res(TCONF, PATH_SYS_CPU_VULN " is not available");
		return;
	}

	d = SAFE_OPENDIR(PATH_SYS_CPU_VULN);

	while ((ent = SAFE_READDIR(d))) {
		if (ent->d_name[0] == '.')
			continue;

		check_vuln(ent->d_name);
	}

	SAFE_CLOSEDIR(d);
}

static struct tst_test test = {
	.test_all = do_test,
};
