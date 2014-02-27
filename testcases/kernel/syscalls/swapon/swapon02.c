/* Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
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
 *		  This test case checks whether swapon(2) system call  returns
 *		  1. ENOENT when the path does not exist
 *		  2. EINVAL when the path exists but is invalid
 *		  3. EPERM when user is not a superuser
 *		  4. EBUSY when the specified path is already being used as a swap area
 */

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <sys/utsname.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"
#include "config.h"
#include "linux_syscall_numbers.h"
#include "tst_fs_type.h"
#include "swaponoff.h"
#include "libswapon.h"

static void setup(void);
static void cleanup(void);
static int setup01(void);
static int cleanup01(void);
static int setup02(void);
static int setup03(void);
static int cleanup03(void);

char *TCID = "swapon02";
int TST_TOTAL = 4;

static char nobody_uid[] = "nobody";
static struct passwd *ltpuser;

static int exp_enos[] = { EPERM, EINVAL, ENOENT, EBUSY, 0 };

static struct test_case_t {
	char *err_desc;		/* error description */
	int exp_errno;		/* expected error number */
	char *exp_errval;	/* Expected errorvalue string */
	char *path;		/* path to swapon */
	int (*setupfunc) ();	/* Test setup function */
	int (*cleanfunc) ();	/* Test cleanup function */
} testcase[] = {
	{
	"Path does not exist", ENOENT, "ENOENT", "./abcd", NULL, NULL}, {
	"Invalid path", EINVAL, "EINVAL", "./nofile", setup02, NULL}, {
	"Permission denied", EPERM, "EPERM", "./swapfile01",
		    setup01, cleanup01}, {
	"The specified path is already being used as a swap area",
		    EBUSY, "EBUSY", "./alreadyused", setup03, cleanup03}
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

			/* do the setup if the test have one */
			if (testcase[i].setupfunc
			    && testcase[i].setupfunc() == -1) {
				tst_resm(TWARN,
					 "Failed to setup test %d."
					 " Skipping test", i);
				continue;
			} else {
				TEST(ltp_syscall(__NR_swapon,
					testcase[i].path, 0));
			}
			/* do the clean if the test have one */
			if (testcase[i].cleanfunc
			    && testcase[i].cleanfunc() == -1) {
				tst_brkm(TBROK, cleanup,
					 "Cleanup failed, quitting the test");
			}
			/* check return code */
			if ((TEST_RETURN == -1)
			    && (TEST_ERRNO == testcase[i].exp_errno)) {
				tst_resm(TPASS,
					 "swapon(2) expected failure;"
					 " Got errno - %s : %s",
					 testcase[i].exp_errval,
					 testcase[i].err_desc);
			} else {
				tst_resm(TFAIL, "swapon(2) failed to produce"
					 " expected error: %d, errno"
					 ": %s and got %d. "
					 " System reboot after"
					 " execution of LTP"
					 " test suite is"
					 " recommended.",
					 testcase[i].exp_errno,
					 testcase[i].exp_errval, TEST_ERRNO);
				/*If swapfile is turned on, turn it off */
				if (TEST_RETURN == 0) {
					if (ltp_syscall
					    (__NR_swapoff,
					     testcase[i].path) != 0) {
						tst_resm(TWARN,
							 "Failed to"
							 " turn off swapfile"
							 " swapfile. System"
							 " reboot after"
							 " execution of LTP"
							 " test suite is"
							 " recommended.");
					}
				}
			}
			TEST_ERROR_LOG(TEST_ERRNO);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup01() - This function creates the file and sets the user as nobody
 */
static int setup01(void)
{
	make_swapfile(cleanup, "swapfile01");

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

/*
 * cleanup01() - switch back to user root and gives swapoff to the swap file
 */
static int cleanup01(void)
{
	if (seteuid(0) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "seteuid failed to set uid to root");
	}

	return 0;
}

/*
 * setup02() - create a normal file, to be used with swapon
 */
static int setup02(void)
{
	int fd;
	fd = creat("nofile", S_IRWXU);
	if (fd == -1)
		tst_resm(TWARN, "Failed to create temporary file");
	close(fd);
	return 0;
}

/*
 * setup03() - This function creates the swap file and turn it on
 */
static int setup03(void)
{
	int res = 0;

	make_swapfile(cleanup, "alreadyused");

	/* turn on the swap file */
	res = ltp_syscall(__NR_swapon, "alreadyused", 0);
	if (res != 0) {
		tst_resm(TWARN, "Failed swapon for file alreadyused"
			 " returned %d", res);
		return -1;
	}

	return 0;
}

/*
 * cleanup03() - clearing the turned on swap file
 */
static int cleanup03(void)
{
	/* give swapoff to the test swap file */
	if (ltp_syscall(__NR_swapoff, "alreadyused") != 0) {
		tst_resm(TWARN, "Failed to turn off swap files. system"
			 " reboot after execution of LTP test"
			 " suite is recommended");
		return -1;
	}

	return 0;
}

static void setup(void)
{
	long type;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	tst_require_root(NULL);

	tst_tmpdir();

	switch ((type = tst_fs_type(cleanup, "."))) {
	case TST_NFS_MAGIC:
	case TST_TMPFS_MAGIC:
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file on %s filesystem",
			 tst_fs_type_name(type));
	break;
	}

	TEST_PAUSE;
}

void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
