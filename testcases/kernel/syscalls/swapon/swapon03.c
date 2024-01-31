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

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "swaponoff.h"
#include "libswap.h"

#define MNTPOINT	"mntpoint"
#define TEST_FILE	MNTPOINT"/testswap"

static int setup_swap(void);
static int clean_swap(void);
static int check_and_swapoff(const char *filename);

static int swapfiles;

int testfiles = 3;
static struct swap_testfile_t {
	char *filename;
} swap_testfiles[] = {
	{"firstswapfile"},
	{"secondswapfile"},
	{"thirdswapfile"}
};

int expected_errno = EPERM;

static void verify_swapon(void)
{
	if (setup_swap() < 0) {
		clean_swap();
		tst_brk(TBROK, "Setup failed, quitting the test");
	}

	TEST(tst_syscall(__NR_swapon, swap_testfiles[0].filename, 0));

	if ((TST_RET == -1) && (TST_ERR == expected_errno)) {
		tst_res(TPASS, "swapon(2) got expected failure (%d),",
			expected_errno);
	} else if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO,
			"swapon(2) failed to produce expected error "
			"(%d). System reboot recommended.",
			expected_errno);
	} else {
		/* Probably the system supports MAX_SWAPFILES > 30,
		 * let's try with MAX_SWAPFILES == 32 */

		/* Call swapon sys call once again for 32
		 * now we can't receive an error */
		TEST(tst_syscall(__NR_swapon, swap_testfiles[1].filename, 0));

		/* Check return code (now we're expecting success) */
		if (TST_RET < 0) {
			tst_res(TFAIL | TTERRNO,
				"swapon(2) got an unexpected failure");
		} else {
			/* Call swapon sys call once again for 33
			 * now we have to receive an error */
			TEST(tst_syscall(__NR_swapon, swap_testfiles[2].filename, 0));

			/* Check return code (should be an error) */
			if ((TST_RET == -1) && (TST_ERR == expected_errno)) {
				tst_res(TPASS,
					"swapon(2) got expected failure;"
					" Got errno = %d, probably your"
					" MAX_SWAPFILES is 32",
					expected_errno);
			} else {
				tst_res(TFAIL,
					"swapon(2) failed to produce"
					" expected error: %d, got %s."
					" System reboot after execution of LTP"
					" test suite is recommended.",
					expected_errno, strerror(TST_ERR));
			}
		}
	}

	if (clean_swap() < 0)
		tst_brk(TBROK, "Cleanup failed, quitting the test");
}

/*
 * Create 33 and activate 30 swapfiles.
 */
static int setup_swap(void)
{
	pid_t pid;
	int j, fd;
	int status;
	int res = 0;
	char filename[FILENAME_MAX];
	char buf[BUFSIZ + 1];

	/* Find out how many swapfiles (1 line per entry) already exist */
	swapfiles = 0;

	if (seteuid(0) < 0)
		tst_brk(TFAIL | TERRNO, "Failed to call seteuid");

	/* This includes the first (header) line */
	if ((fd = open("/proc/swaps", O_RDONLY)) == -1) {
		tst_brk(TFAIL | TERRNO,
			"Failed to find out existing number of swap files");
	}
	do {
		char *p = buf;
		res = read(fd, buf, BUFSIZ);
		if (res < 0) {
			tst_brk(TFAIL | TERRNO,
				 "Failed to find out existing number of swap files");
		}
		buf[res] = '\0';
		while ((p = strchr(p, '\n'))) {
			p++;
			swapfiles++;
		}
	} while (BUFSIZ <= res);
	close(fd);
	if (swapfiles)
		swapfiles--;	/* don't count the /proc/swaps header */

	if (swapfiles < 0)
		tst_brk(TFAIL, "Failed to find existing number of swapfiles");

	/* Determine how many more files are to be created */
	swapfiles = MAX_SWAPFILES - swapfiles;
	if (swapfiles > MAX_SWAPFILES)
		swapfiles = MAX_SWAPFILES;
	pid = SAFE_FORK();
	if (pid == 0) {
		/*create and turn on remaining swapfiles */
		for (j = 0; j < swapfiles; j++) {

			/* prepare filename for the iteration */
			if (sprintf(filename, "swapfile%02d", j + 2) < 0) {
				printf("sprintf() failed to create "
				       "filename");
				exit(1);
			}

			/* Create the swapfile */
			make_swapfile(filename, 10, 0);

			/* turn on the swap file */
			res = tst_syscall(__NR_swapon, filename, 0);
			if (res != 0) {
				if (errno == EPERM) {
					printf("Successfully created %d swapfiles\n", j);
					break;
				} else {
					printf("Failed to create swapfile: %s\n", filename);
					exit(1);
				}
			}
		}
		exit(0);
	} else
		waitpid(pid, &status, 0);

	if (WEXITSTATUS(status))
		tst_brk(TFAIL, "Failed to setup swaps");

	/* Create all needed extra swapfiles for testing */
	for (j = 0; j < testfiles; j++)
		make_swapfile(swap_testfiles[j].filename, 10, 0);

	return 0;
}

/*
 * Turn off all swapfiles previously turned on
 */
static int clean_swap(void)
{
	int j;
	char filename[FILENAME_MAX];

	for (j = 0; j < swapfiles; j++) {
		if (snprintf(filename, sizeof(filename),
			     "swapfile%02d", j + 2) < 0) {
			tst_res(TWARN, "sprintf() failed to create filename");
			tst_res(TWARN, "Failed to turn off swap files. System"
				 " reboot after execution of LTP test"
				 " suite is recommended");
			return -1;
		}
		if (check_and_swapoff(filename) != 0) {
			tst_res(TWARN, "Failed to turn off swap file %s.", filename);
			return -1;
		}
	}

	for (j = 0; j < testfiles; j++) {
		if (check_and_swapoff(swap_testfiles[j].filename) != 0) {
			tst_res(TWARN, "Failed to turn off swap file %s.",
				 swap_testfiles[j].filename);
			return -1;
		}
	}

	return 0;
}

/*
 * Check if the file is at /proc/swaps and remove it giving swapoff
 */
static int check_and_swapoff(const char *filename)
{
	char cmd_buffer[256];
	int rc = -1;

	if (snprintf(cmd_buffer, sizeof(cmd_buffer),
		     "grep -q '%s.*file' /proc/swaps", filename) < 0) {
		tst_res(TWARN, "sprintf() failed to create the command string");
	} else {

		rc = 0;

		if (system(cmd_buffer) == 0) {

			/* now we need to swapoff the file */
			if (tst_syscall(__NR_swapoff, filename) != 0) {

				tst_res(TWARN, "Failed to turn off swap "
					 "file. system reboot after "
					 "execution of LTP test suite "
					 "is recommended");
				rc = -1;

			}

		}
	}

	return rc;
}

static void setup(void)
{
	if (access("/proc/swaps", F_OK))
		tst_brk(TCONF, "swap not supported by kernel");

	is_swap_supported(TEST_FILE);
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
	.needs_tmpdir = 1,
	.forks_child = 1,
	.test_all = verify_swapon,
	.setup = setup,
	.cleanup = cleanup
};
