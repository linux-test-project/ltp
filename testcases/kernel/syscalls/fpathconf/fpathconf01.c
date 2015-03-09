/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Author: William Roske
 * Co-pilot: Dave Fenner
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
 * Testcase to test the basic functionality of fpathconf(2) system call.
 */

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

static struct pathconf_args {
	char *name;
	int value;
} test_cases[] = {
	{"_PC_MAX_CANON", _PC_MAX_CANON},
	{"_PC_MAX_INPUT", _PC_MAX_INPUT},
	{"_PC_VDISABLE", _PC_VDISABLE},
	{"_PC_LINK_MAX", _PC_LINK_MAX},
	{"_PC_NAME_MAX", _PC_NAME_MAX},
	{"_PC_PATH_MAX", _PC_PATH_MAX},
	{"_PC_PIPE_BUF", _PC_PIPE_BUF},
	{"_PC_CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED},
	{"_PC_NO_TRUNC", _PC_NO_TRUNC},
};

char *TCID = "fpathconf01";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static int fd;

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			errno = 0;

			TEST(fpathconf(fd, test_cases[i].value));

			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == 0) {
					tst_resm(TINFO,
						 "fpathconf has NO limit for "
						 "%s", test_cases[i].name);
				} else {
					tst_resm(TFAIL | TTERRNO,
						 "fpathconf(fd, %s) failed",
						 test_cases[i].name);
				}
			} else {
				tst_resm(TPASS,
					 "fpathconf(fd, %s) returned %ld",
					 test_cases[i].name, TEST_RETURN);
			}
		}
	}

	cleanup();

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, "fpafile01", O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(NULL, fd);

	tst_rmdir();
}
