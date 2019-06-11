/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*
 * This test case checks whether swapoff(2) system call  returns
 *  1. EINVAL when the path does not exist
 *  2. ENOENT when the path exists but is invalid
 *  3. EPERM when user is not a superuser
 */

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include "test.h"
#include "lapi/syscalls.h"
#include "safe_macros.h"
#include "../swapon/libswapon.h"

static void setup(void);
static void cleanup(void);
static int setup01(void);
static void cleanup01(void);

char *TCID = "swapoff02";
int TST_TOTAL = 3;

static uid_t nobody_uid;

static struct test_case_t {
	char *err_desc;
	int exp_errno;
	char *exp_errval;
	char *path;
	int (*setup)(void);
	void (*cleanup)(void);
} testcase[] = {
	{"path does not exist", ENOENT, "ENOENT", "./doesnotexist", NULL, NULL},
	{"Invalid file", EINVAL, "EINVAL", "./swapfile01", NULL, NULL},
	{"Permission denied", EPERM, "EPERM", "./swapfile01", setup01, cleanup01}
};

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (testcase[i].setup)
				testcase[i].setup();

			TEST(ltp_syscall(__NR_swapoff, testcase[i].path));

			if (testcase[i].cleanup)
				testcase[i].cleanup();

			if (TEST_RETURN == -1
			    && (TEST_ERRNO == testcase[i].exp_errno)) {
				tst_resm(TPASS,
					 "swapoff(2) expected failure;"
					 " Got errno - %s : %s",
					 testcase[i].exp_errval,
					 testcase[i].err_desc);

			} else {
				tst_resm(TFAIL, "swapoff(2) failed to produce"
					 " expected error; %d, errno"
					 ": %s and got %d",
					 testcase[i].exp_errno,
					 testcase[i].exp_errval, TEST_ERRNO);

				if ((TEST_RETURN == 0) && (i == 2)) {
					if (ltp_syscall
					    (__NR_swapon, "./swapfile01",
					     0) != 0) {
						tst_brkm(TBROK, cleanup,
							 " Failed to turn on"
							 " swap file");
					}
				}
			}
		}
	}

	cleanup();
	tst_exit();
}

static int setup01(void)
{
	SAFE_SETEUID(cleanup, nobody_uid);
	return 0;
}

static void cleanup01(void)
{
	SAFE_SETEUID(cleanup, 0);
}

static void setup(void)
{
	struct passwd *nobody;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	nobody = SAFE_GETPWNAM(NULL, "nobody");
	nobody_uid = nobody->pw_uid;

	TEST_PAUSE;

	tst_tmpdir();

	is_swap_supported(cleanup, "./tstswap");

	if (!tst_fs_has_free(NULL, ".", 1, TST_KB)) {
		tst_brkm(TBROK, cleanup,
			 "Insufficient disk space to create swap file");
	}

	if (tst_fill_file("./swapfile01", 0x00, 1024, 1))
		tst_brkm(TBROK, cleanup, "Failed to create swapfile");
}

static void cleanup(void)
{
	tst_rmdir();
}
