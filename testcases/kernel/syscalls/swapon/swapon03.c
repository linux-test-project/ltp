/******************************************************************************
 *
 * Copyright (c) International Business Machines  Corp., 2007
 *  Created by <rsalveti@linux.vnet.ibm.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 ******************************************************************************/

/*
 * This test case checks whether swapon(2) system call returns:
 *  - EPERM when there are more than MAX_SWAPFILES already in use.
 *
 */

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "lapi/syscalls.h"
#include "swaponoff.h"
#include "libswapon.h"

static void setup(void);
static void cleanup(void);
static int setup_swap(void);
static int clean_swap(void);
static int check_and_swapoff(const char *filename);

char *TCID = "swapon03";
int TST_TOTAL = 1;

static int swapfiles;

static long fs_type;

int testfiles = 3;
static struct swap_testfile_t {
	char *filename;
} swap_testfiles[] = {
	{"firstswapfile"},
	{"secondswapfile"},
	{"thirdswapfile"}
};

int expected_errno = EPERM;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if (setup_swap() < 0) {
			clean_swap();
			tst_brkm(TBROK, cleanup,
				 "Setup failed, quitting the test");
		}

		TEST(ltp_syscall(__NR_swapon, swap_testfiles[0].filename, 0));

		if ((TEST_RETURN == -1) && (TEST_ERRNO == expected_errno)) {
			tst_resm(TPASS, "swapon(2) got expected failure (%d),",
				 expected_errno);
		} else if (TEST_RETURN < 0) {
			tst_resm(TFAIL | TTERRNO,
				 "swapon(2) failed to produce expected error "
				 "(%d). System reboot recommended.",
				 expected_errno);
		} else {
			/* Probably the system supports MAX_SWAPFILES > 30,
			 * let's try with MAX_SWAPFILES == 32 */

			/* Call swapon sys call once again for 32
			 * now we can't receive an error */
			TEST(ltp_syscall
			     (__NR_swapon, swap_testfiles[1].filename, 0));

			/* Check return code (now we're expecting success) */
			if (TEST_RETURN < 0) {
				tst_resm(TFAIL | TTERRNO,
					 "swapon(2) got an unexpected failure");
			} else {
				/* Call swapon sys call once again for 33
				 * now we have to receive an error */
				TEST(ltp_syscall
				     (__NR_swapon, swap_testfiles[2].filename,
				      0));

				/* Check return code (should be an error) */
				if ((TEST_RETURN == -1)
				    && (TEST_ERRNO == expected_errno)) {
					tst_resm(TPASS,
						 "swapon(2) got expected failure;"
						 " Got errno = %d, probably your"
						 " MAX_SWAPFILES is 32",
						 expected_errno);
				} else {
					tst_resm(TFAIL,
						 "swapon(2) failed to produce"
						 " expected error: %d, got %s."
						 " System reboot after execution of LTP"
						 " test suite is recommended.",
						 expected_errno,
						 strerror(TEST_ERRNO));
				}

			}
		}

		if (clean_swap() < 0)
			tst_brkm(TBROK, cleanup,
				 "Cleanup failed, quitting the test");

	}

	cleanup();
	tst_exit();

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

	if (seteuid(0) < 0) {
		tst_brkm(TFAIL | TERRNO, cleanup, "Failed to call seteuid");
	}

	/* This includes the first (header) line */
	if ((fd = open("/proc/swaps", O_RDONLY)) == -1) {
		tst_brkm(TFAIL | TERRNO, cleanup,
			 "Failed to find out existing number of swap files");
	}
	do {
		char *p = buf;
		res = read(fd, buf, BUFSIZ);
		if (res < 0) {
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "Failed to find out existing number of swap "
				 "files");
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

	if (swapfiles < 0) {
		tst_brkm(TFAIL, cleanup,
			 "Failed to find existing number of swapfiles");
	}

	/* Determine how many more files are to be created */
	swapfiles = MAX_SWAPFILES - swapfiles;
	if (swapfiles > MAX_SWAPFILES) {
		swapfiles = MAX_SWAPFILES;
	}

	pid = FORK_OR_VFORK();
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
			make_swapfile(cleanup, filename, 0);

			/* turn on the swap file */
			res = ltp_syscall(__NR_swapon, filename, 0);
			if (res != 0) {
				if (fs_type == TST_BTRFS_MAGIC && errno == EINVAL)
					exit(2);

				if (errno == EPERM) {
					printf("Successfully created %d "
					       "swapfiles\n", j);
					break;
				} else {
					printf("Failed to create "
					       "swapfile: %s\n", filename);
					exit(1);
				}
			}
		}
		exit(0);
	} else
		waitpid(pid, &status, 0);

	switch (WEXITSTATUS(status)) {
	case 0:
	break;
	case 2:
		tst_brkm(TCONF, cleanup, "Swapfile on BTRFS not implemeted");
	break;
	default:
		tst_brkm(TFAIL, cleanup, "Failed to setup swaps");
	break;
	}

	/* Create all needed extra swapfiles for testing */
	for (j = 0; j < testfiles; j++)
		make_swapfile(cleanup, swap_testfiles[j].filename, 0);

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
			tst_resm(TWARN, "sprintf() failed to create filename");
			tst_resm(TWARN, "Failed to turn off swap files. System"
				 " reboot after execution of LTP test"
				 " suite is recommended");
			return -1;
		}
		if (check_and_swapoff(filename) != 0) {
			tst_resm(TWARN, "Failed to turn off swap file %s.",
				 filename);
			return -1;
		}
	}

	for (j = 0; j < testfiles; j++) {
		if (check_and_swapoff(swap_testfiles[j].filename) != 0) {
			tst_resm(TWARN, "Failed to turn off swap file %s.",
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
		tst_resm(TWARN,
			 "sprintf() failed to create the command string");
	} else {

		rc = 0;

		if (system(cmd_buffer) == 0) {

			/* now we need to swapoff the file */
			if (ltp_syscall(__NR_swapoff, filename) != 0) {

				tst_resm(TWARN, "Failed to turn off swap "
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
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	if (access("/proc/swaps", F_OK))
		tst_brkm(TCONF, NULL, "swap not supported by kernel");

	tst_tmpdir();

	is_swap_supported(cleanup, "./tstswap");

	TEST_PAUSE;
}

static void cleanup(void)
{
	clean_swap();

	tst_rmdir();
}
