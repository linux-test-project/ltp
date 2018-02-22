/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR		: Richard Logan
 *  CO-PILOT		: William Roske
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */

/*
 * Negative test cases for link(2).
 *
 * This test program should contain test cases where link will fail regardless
 * of who executed it (i.e. joe-user or root)
 */
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/mman.h>
#include "test.h"
#include "safe_macros.h"

static char longpath[PATH_MAX + 2];

struct test_case_t {
	char *file1;
	char *desc1;
	char *file2;
	char *desc2;
	int exp_errno;
} test_cases[] = {
	/* first path is invalid */
	{"nonexistfile", "non-existent file", "nefile", "nefile", ENOENT},
	{"", "path is empty string", "nefile", "nefile", ENOENT},
	{"neefile/file", "path contains a non-existent file", "nefile",
	 "nefile", ENOENT},
	{"regfile/file", "path contains a regular file", "nefile", "nefile",
	 ENOTDIR},
	{longpath, "pathname too long", "nefile", "nefile", ENAMETOOLONG},
	{NULL, "invalid address", "nefile", "nefile", EFAULT},
	/* second path is invalid */
	{"regfile", "regfile", "", "empty string", ENOENT},
	{"regfile", "regfile", "neefile/file",
		    "path contains a non-existent file", ENOENT},
	{"regfile", "regfile", "file/file",
		    "path contains a regular file", ENOENT},
	{"regfile", "regfile", longpath, "pathname too long", ENAMETOOLONG},
	{"regfile", "regfile", NULL, "invalid address", EFAULT},
	/* two existing files */
	{"regfile", "regfile", "regfile2", "regfile2", EEXIST},
};

char *TCID = "link04";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	char *fname1, *fname2;
	char *desc1, *desc2;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			fname1 = test_cases[i].file1;
			desc1 = test_cases[i].desc1;
			fname2 = test_cases[i].file2;
			desc2 = test_cases[i].desc2;

			TEST(link(fname1, fname2));

			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == test_cases[i].exp_errno) {
					tst_resm(TPASS | TTERRNO,
						 "link(<%s>, <%s>)",
						 desc1, desc2);
				} else {
					tst_resm(TFAIL | TTERRNO,
						 "link(<%s>, <%s>) Failed "
					         "expected errno: %d",
						 desc1, desc2,
						 test_cases[i].exp_errno);
				}
			} else {
				tst_resm(TFAIL,
					 "link(<%s>, <%s>) returned %ld, "
				         "expected -1, errno:%d",
					 desc1, desc2, TEST_RETURN,
					 test_cases[i].exp_errno);
			}
		}

	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int n;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	memset(longpath, 'a', PATH_MAX+1);
	SAFE_TOUCH(cleanup, "regfile", 0777, NULL);
	SAFE_TOUCH(cleanup, "regfile2", 0777, NULL);
	SAFE_MKDIR(cleanup, "dir", 0777);

	void *bad_addr = tst_get_bad_addr(cleanup);

	for (n = 0; n < TST_TOTAL; n++) {
		if (!test_cases[n].file1)
			test_cases[n].file1 = bad_addr;

		if (!test_cases[n].file2)
			test_cases[n].file2 = bad_addr;
	}
}

static void cleanup(void)
{
	tst_rmdir();
}
