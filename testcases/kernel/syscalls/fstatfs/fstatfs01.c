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
 * DETAILED DESCRIPTION
 *   This is a Phase I test for the fstatfs(2) system call.  It is intended
 *   to provide a limited exposure of the system call, for now.  It
 *   should/will be extended when full functional tests are written for
 *   fstatfs(2).
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/statfs.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

static void setup(void);
static void cleanup(void);

char *TCID = "fstatfs01";
int TST_TOTAL = 1;

static int exp_enos[] = { 0, 0 };

static char fname[255];
static int fd;
static struct statfs stats;

int main(int ac, char **av)
{
	int lc;
	const char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(fstatfs(fd, &stats));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "fstatfs failed");
		} else {
			tst_resm(TPASS,
				 "fstatfs(%d, &stats, sizeof(struct statfs), 0) returned %ld",
				 fd, TEST_RETURN);
		}

	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "tfile_%d", getpid());
	if ((fd = open(fname, O_RDWR | O_CREAT, 0700)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "open failed");
}

static void cleanup(void)
{
	TEST_CLEANUP;

	if (close(fd) == -1)
		tst_resm(TWARN | TERRNO, "close failed");

	tst_rmdir();
}
