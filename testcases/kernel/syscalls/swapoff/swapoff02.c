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
 *    DESCRIPTION
 *	This test case checks whether swapoff(2) system call  returns
 *	1. EINVAL when the path does not exist
 *	2. ENOENT when the path exists but is invalid
 *	3. EPERM when user is not a superuser
 */

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include "test.h"
#include "usctest.h"
#include <stdlib.h>
#include "config.h"
#include "linux_syscall_numbers.h"
#include "swaponoff.h"

static void setup(void);
static void cleanup(void);
static int setup01(void);
static int cleanup01(void);
static int setup02(void);

char *TCID = "swapoff02";
int TST_TOTAL = 3;
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

static int exp_enos[] = { EPERM, EINVAL, ENOENT, 0 };

static struct test_case_t {
	char *err_desc;		/* error description */
	int exp_errno;		/* expected error number */
	char *exp_errval;	/* Expected errorvalue string */
	char *path;		/* path for swapon */
	int (*setupfunc) ();	/* Test setup function */
	int (*cleanfunc) ();	/* Test cleanup function */
} testcase[] = {
	{
	"path does not exist", ENOENT, "ENOENT", "./abcd", NULL, NULL}, {
	"Invalid path", EINVAL, "EINVAL ", "./nofile", setup02, NULL}, {
	"Permission denied", EPERM, "EPERM ", "./swapfile01",
		    setup01, cleanup01}
};

int main(int ac, char **av)
{

	int lc, i;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (testcase[i].setupfunc &&
			    testcase[i].setupfunc() == -1) {
				tst_resm(TWARN, "Failed to setup test %d."
					 " Skipping test", i);
				continue;
			} else {
				TEST(ltp_syscall(__NR_swapoff,
					testcase[i].path));
			}

			if (testcase[i].cleanfunc &&
			    testcase[i].cleanfunc() == -1) {
				tst_brkm(TBROK, cleanup, "cleanup failed,"
					 " quitting the test");
			}

			/* check return code */
			if ((TEST_RETURN == -1)
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

			TEST_ERROR_LOG(TEST_ERRNO);
		}
	}

	cleanup();
	tst_exit();
}

static int setup01(void)
{
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_resm(TWARN, "\"nobody\" user not present. skipping test");
		return -1;
	}

	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TWARN, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("seteuid");
		return -1;
	}

	return 0;
}

static int cleanup01(void)
{
	if (seteuid(0) == -1) {
		tst_brkm(TBROK, cleanup, "seteuid failed to set uid to root");
		perror("seteuid");
		return -1;
	}

	return 0;
}

static int setup02(void)
{
	int fd;
	fd = creat("nofile", S_IRWXU);
	if (fd == -1)
		tst_resm(TWARN, "Failed to create temporary file");
	return 0;
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	tst_require_root(NULL);

	TEST_PAUSE;

	tst_tmpdir();

	if (tst_is_cwd_tmpfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a tmpfs filesystem");
	}

	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a nfs filesystem");
	}

	if (!tst_cwd_has_free(1)) {
		tst_brkm(TBROK, cleanup,
			 "Insufficient disk space to create swap file");
	}

	if (tst_fill_file("./swapfile01", 0x00, 1024, 1))
		tst_brkm(TBROK, cleanup, "Failed to create swapfile");
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
