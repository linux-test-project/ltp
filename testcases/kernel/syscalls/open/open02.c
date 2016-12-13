/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
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
 * DESCRIPTION
 *	1. open a new file without O_CREAT, ENOENT should be returned.
 *	2. open a file with O_RDONLY | O_NOATIME and the caller was not
 *	   privileged, EPERM should be returned.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"

#define TEST_FILE	"test_file"
#define TEST_FILE2	"test_file2"

char *TCID = "open02";

static void cleanup(void);
static void setup(void);

static struct test_case_t {
	char *filename;
	int flag;
	int exp_errno;
} test_cases[] = {
	{TEST_FILE, O_RDWR, ENOENT},
	{TEST_FILE2, O_RDONLY | O_NOATIME, EPERM},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);
static void open_verify(const struct test_case_t *);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			open_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct passwd *ltpuser;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_TOUCH(cleanup, TEST_FILE2, 0644, NULL);

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");

	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void open_verify(const struct test_case_t *test)
{
	TEST(open(test->filename, test->flag, 0444));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "open(%s) succeeded unexpectedly",
			 test->filename);
		return;
	}

	if (TEST_ERRNO != test->exp_errno) {
		tst_resm(TFAIL | TTERRNO,
			 "open() failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	} else {
		tst_resm(TPASS | TTERRNO, "open() failed as expected");
	}
}

static void cleanup(void)
{
	if (seteuid(0))
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	tst_rmdir();
}
