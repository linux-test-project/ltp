// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 * Created by <rsalveti@linux.vnet.ibm.com>
 *
 */

/*\
 * [Description]
 *
 * This test case checks whether swapon(2) system call returns:
 *  - EPERM when there are more than MAX_SWAPFILES already in use.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/swap.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "libswap.h"

#define MNTPOINT	"mntpoint"
#define TEST_FILE	MNTPOINT"/testswap"

static int swapfiles;

static int setup_swap(void)
{
	pid_t pid;
	int status;
	int j, max_swapfiles, used_swapfiles;
	char filename[FILENAME_MAX];

	SAFE_SETEUID(0);

	/* Determine how many more files are to be created */
	max_swapfiles = tst_max_swapfiles();
	used_swapfiles = tst_count_swaps();
	swapfiles = max_swapfiles - used_swapfiles;
	if (swapfiles > max_swapfiles)
		swapfiles = max_swapfiles;

	pid = SAFE_FORK();
	if (pid == 0) {
		/*create and turn on remaining swapfiles */
		for (j = 0; j < swapfiles; j++) {

			/* Create the swapfile */
			snprintf(filename, sizeof(filename), "%s%02d", TEST_FILE, j + 2);
			MAKE_SWAPFILE_BLKS(filename, 10);

			/* turn on the swap file */
			TST_EXP_PASS_SILENT(swapon(filename, 0));
		}
		exit(0);
	} else
		waitpid(pid, &status, 0);

	if (WEXITSTATUS(status))
		tst_brk(TFAIL, "Failed to setup swap files");

	tst_res(TINFO, "Successfully created %d swap files", swapfiles);
	MAKE_SWAPFILE_BLKS(TEST_FILE, 10);

	return 0;
}

/*
 * Check if the file is at /proc/swaps and remove it giving swapoff
 */
static int check_and_swapoff(const char *filename)
{
	char cmd_buffer[256];
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
		snprintf(filename, sizeof(filename), "%s%02d", TEST_FILE, j + 2);
		check_and_swapoff(filename);
	}

	check_and_swapoff("testfile");
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
	.forks_child = 1,
	.test_all = verify_swapon,
	.setup = setup,
	.cleanup = cleanup
};
