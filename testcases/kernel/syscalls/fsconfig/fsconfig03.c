// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 Alejandro Guerrero <aguerrero@qualys.com>
 * Copyright (c) 2023 Wei Gao <wegao@suse.com>
 */

/*\
 * [Description]
 *
 * Test for CVE-2022-0185.
 *
 * References links:
 *
 * - https://www.openwall.com/lists/oss-security/2022/01/25/14
 * - https://github.com/Crusaders-of-Rust/CVE-2022-0185
 *
 */

#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

static int fd = -1;

static void setup(void)
{
	fsopen_supported_by_kernel();
}

static void run(void)
{
	char *val = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
	long pagesize;

	TEST(fd = fsopen(tst_device->fs_type, 0));
	if (fd == -1)
		tst_brk(TBROK | TTERRNO, "fsopen() failed");

	pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize == -1)
		tst_brk(TBROK, "sysconf(_SC_PAGESIZE) failed");

	for (size_t i = 0; i < 5000; i++) {
		/* use same logic in kernel legacy_parse_param function */
		const size_t len = i * (strlen(val) + 2) + (strlen(val) + 1) + 2;

		TEST(fsconfig(fd, FSCONFIG_SET_STRING, "\x00", val, 0));

		/* Legacy fsconfig() just copies arguments to buffer */
		if (!TST_RET && len <= (size_t)pagesize)
			continue;

		if (!TST_RET) {
			tst_res(TFAIL, "fsconfig() passed unexpectedly");
		} else if (TST_RET != -1) {
			tst_brk(TBROK | TTERRNO,
				"Invalid fsconfig() return value %ld", TST_RET);
		} else if (TST_ERR != EINVAL) {
			tst_res(TFAIL | TTERRNO,
				"fsconfig() failed with unexpected error");
		}
	}

	if (fd != -1)
		SAFE_CLOSE(fd);

	if (tst_taint_check())
		tst_res(TFAIL, "kernel has issues on %s",
			tst_device->fs_type);
	else
		tst_res(TPASS, "kernel seems to be fine on %s",
			tst_device->fs_type);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.skip_filesystems = (const char *const []){"fuse", NULL},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "722d94847de29"},
		{"CVE", "2022-0185"},
		{}
	}
};
