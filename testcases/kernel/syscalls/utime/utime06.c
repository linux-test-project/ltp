/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 John George
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
 */

/*
 * Test Description:
 * 1. Verify that the system call utime() fails to set the modification
 *    and access times of a file to the current time, under the following
 *    constraints,
 *	 - The times argument is null.
 *	 - The user ID of the process is not "root".
 * 2. Verify that the system call utime() fails to set the modification
 *    and access times of a file if the specified file doesn't exist.
 */

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

#define TEMP_FILE	"tmp_file"

char *TCID = "utime06";
static int exp_enos[] = { EACCES, ENOENT, 0 };

static struct passwd *ltpuser;

struct test_case_t {
	char *pathname;
	int exp_errno;
} Test_cases[] = {
	{TEMP_FILE, EACCES},
	{"", ENOENT},
};

int TST_TOTAL = ARRAY_SIZE(Test_cases);
static void setup(void);
static void utime_verify(const struct test_case_t *);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	const char *msg;
	int i;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			utime_verify(&Test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_TOUCH(cleanup, TEMP_FILE, 0644, NULL);

	TEST_EXP_ENOS(exp_enos);

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");

	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void utime_verify(const struct test_case_t *test)
{
	TEST(utime(test->pathname, NULL));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "utime succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO, "utime failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "utime failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
	if (seteuid(0) != 0)
		tst_resm(TWARN | TERRNO, "seteuid failed");

	TEST_CLEANUP;

	tst_rmdir();
}
