/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 *
 * Test Description:
 * Verify that,
 *   1) mknod(2) returns -1 and sets errno to EPERM if the process id of
 *	the caller is not super-user.
 *   2) mknod(2) returns -1 and sets errno to EACCES if parent directory
 *	does not allow  write  permission  to  the process.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

#define DIR_TEMP		"testdir_1"
#define DIR_TEMP_MODE		(S_IRUSR | S_IXUSR)

#define FIFO_MODE	(S_IFIFO | S_IRUSR | S_IRGRP | S_IROTH)
#define SOCKET_MODE	(S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO)
#define CHR_MODE	(S_IFCHR | S_IRUSR | S_IWUSR)
#define BLK_MODE	(S_IFBLK | S_IRUSR | S_IWUSR)

static struct test_case_t {
	char *pathname;
	int mode;
	int exp_errno;
} test_cases[] = {
	{ "testdir_1/tnode_1", SOCKET_MODE, EACCES },
	{ "testdir_1/tnode_2", FIFO_MODE, EACCES },
	{ "tnode_3", CHR_MODE, EPERM },
	{ "tnode_4", BLK_MODE, EPERM },
};

char *TCID = "mknod07";
int TST_TOTAL = ARRAY_SIZE(test_cases);
static int exp_enos[] = { EPERM, EACCES, 0 };

static void setup(void);
static void mknod_verify(const struct test_case_t *test_case);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int i;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			mknod_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct passwd *ltpuser;

	tst_require_root(NULL);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	tst_tmpdir();

	TEST_PAUSE;

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);

	SAFE_MKDIR(cleanup, DIR_TEMP, DIR_TEMP_MODE);
}

static void mknod_verify(const struct test_case_t *test_case)
{
	TEST(mknod(test_case->pathname, test_case->mode, 0));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "mknod succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test_case->exp_errno) {
		tst_resm(TPASS | TTERRNO, "mknod failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "mknod failed unexpectedly; expected: "
			 "%d - %s", test_case->exp_errno,
			 strerror(test_case->exp_errno));
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;

	if (seteuid(0) == -1)
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	tst_rmdir();
}
