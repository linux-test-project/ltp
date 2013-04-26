/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*
 * Description:
 *   Basic test for access(2) using F_OK, R_OK, W_OK and X_OK on tmp file
 *   AUTHOR		: William Roske
 */

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"


char *TCID = "access01";

char fname[255];

static struct test_case_t {
	char *file;
	int mode;
	char *string;
	int experrno;
} test_cases[] = {
	{fname, F_OK, "F_OK", 0},
	{fname, X_OK, "X_OK", 0},
	{fname, W_OK, "W_OK", 0},
	{fname, R_OK, "R_OK", 0},
};

int TST_TOTAL = sizeof(test_cases) / sizeof(struct test_case_t);

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int tc;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (tc = 0; tc < TST_TOTAL; tc++) {
			TEST(access(test_cases[tc].file, test_cases[tc].mode));

			if (TEST_RETURN == -1 && test_cases[tc].experrno == 0) {
				tst_resm(TFAIL | TTERRNO,
					 "access(%s, %s) failed",
					 test_cases[tc].file,
					 test_cases[tc].string);

			} else if (TEST_RETURN != -1
				   && test_cases[tc].experrno != 0) {
				tst_resm(TFAIL,
					 "access(%s, %s) returned %ld, "
					 "exp -1, errno:%d",
					 test_cases[tc].file,
					 test_cases[tc].string, TEST_RETURN,
					 test_cases[tc].experrno);
			} else {
				if (STD_FUNCTIONAL_TEST) {
					tst_resm(TPASS,
						 "access(%s, %s) returned %ld",
						 test_cases[tc].file,
						 test_cases[tc].string,
						 TEST_RETURN);
				}
			}
		}

	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;
	struct stat stbuf;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);
	TEST_PAUSE;
	tst_tmpdir();

	/*
	 * Since files inherit group ids, make sure our dir has a valid grp
	 * to us.
	 */
	if (chown(".", -1, getegid()) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "chown(\".\", -1, %d) failed", getegid());
	}

	snprintf(fname, sizeof(fname), "accessfile");

	fd = open(fname, O_RDWR | O_CREAT, 06777);
	if (fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 06777) failed", fname);
	else if (close(fd) == -1)
		tst_resm(TINFO | TERRNO, "close(%s) failed", fname);

	/*
	 * force the mode to be set to 6777
	 */
	if (chmod(fname, 06777) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "chmod(%s, 06777) failed",
			 fname);

	stat(fname, &stbuf);

	if ((stbuf.st_mode & 06777) != 06777) {
		tst_brkm(TBROK, cleanup, "'%s' can't be properly setup",
			 fname);
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;
	tst_rmdir();
}
