// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2009-2025
 * Copyright (c) International Business Machines Corp., 2007
 * Created by <rsalveti@linux.vnet.ibm.com>
 */

/*\
 * Test checks whether :man2:`swapon` system call returns EPERM when the maximum
 * number of swap files are already in use.
 *
 * NOTE: test does not try to calculate MAX_SWAPFILES from the internal
 * kernel implementation, instead make sure at least 15 swaps were created
 * before the maximum of swaps was reached.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/swap.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "libswap.h"

/*
 * MAX_SWAPFILES from the internal kernel implementation is currently <23, 29>,
 * depending on kernel configuration (see man swapon(2)). Chose small enough
 * value for future changes.
 */
#define MIN_SWAP_FILES 15

#define MNTPOINT	"mntpoint"
#define TEST_FILE	MNTPOINT"/testswap"

static int swapfiles;

static int setup_swap(void)
{
	int used_swapfiles, min_swapfiles;
	char filename[FILENAME_MAX];

	used_swapfiles = tst_count_swaps();
	min_swapfiles = MIN_SWAP_FILES - used_swapfiles;

	while (true) {
		/* Create the swapfile */
		snprintf(filename, sizeof(filename), "%s%02d", TEST_FILE, swapfiles);
		MAKE_SMALL_SWAPFILE(filename);

		/* Quit on a first swap file over max, check for EPERM */
		if (swapon(filename, 0) == -1) {
			if (errno == EPERM && swapfiles > min_swapfiles)
				break;

			tst_brk(TFAIL | TERRNO, "swapon(%s, 0)", filename);
		}
		swapfiles++;
	}

	tst_res(TINFO, "Successfully created %d swap files", swapfiles);

	return 0;
}

/*
 * Check if the file is at /proc/swaps and remove it giving swapoff
 */
static int check_and_swapoff(const char *filename)
{
	char cmd_buffer[FILENAME_MAX+28];
	int rc = -1;

	snprintf(cmd_buffer, sizeof(cmd_buffer), "grep -q '%s.*file' /proc/swaps", filename);

	if (system(cmd_buffer) == 0 && swapoff(filename) != 0) {
		tst_res(TWARN, "Failed to swapoff %s", filename);
		rc = -1;
	}

	return rc;
}

/*
 * Turn off all swapfiles previously turned on
 */
static void clean_swap(void)
{
	int j;
	char filename[FILENAME_MAX];

	for (j = 0; j < swapfiles; j++) {
		snprintf(filename, sizeof(filename), "%s%02d", TEST_FILE, j);
		check_and_swapoff(filename);
	}
}

static void verify_swapon(void)
{
	TST_EXP_FAIL(swapon(TEST_FILE, 0), EPERM, "swapon(%s, 0)", TEST_FILE);
}

static void setup(void)
{
	if (access("/proc/swaps", F_OK))
		tst_brk(TCONF, "swap not supported by kernel");

	is_swap_supported(TEST_FILE);
	if (setup_swap() < 0) {
		clean_swap();
		tst_brk(TBROK, "Setup failed, quitting the test");
	}
}

static void cleanup(void)
{
	clean_swap();
}

static struct tst_test test = {
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.needs_root = 1,
	.test_all = verify_swapon,
	.setup = setup,
	.cleanup = cleanup
};
