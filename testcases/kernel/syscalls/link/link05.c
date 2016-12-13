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
 */

/*
 * Test if link(2) fails with EMLINK.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);
static void help(void);

char *TCID = "link05";
int TST_TOTAL = 1;

#define BASENAME	"lkfile"

static char fname[255];

static char *links_arg;

option_t options[] = {
	{"N:", NULL, &links_arg},
	{NULL, NULL, NULL}
};

static int nlinks = 1000;

int main(int ac, char **av)
{
	int lc;
	struct stat fbuf, lbuf;
	int cnt;
	char lname[255];

	tst_parse_opts(ac, av, options, &help);

	if (links_arg) {
		nlinks = atoi(links_arg);

		if (nlinks == 0) {
			tst_brkm(TBROK, NULL,
			         "nlinks is not a positive number");
		}
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (cnt = 1; cnt < nlinks; cnt++) {
			sprintf(lname, "%s%d", fname, cnt);
			TEST(link(fname, lname));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "link(%s, %s) Failed, errno=%d : %s",
					 fname, lname, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			}
		}

		SAFE_STAT(cleanup, fname, &fbuf);

		for (cnt = 1; cnt < nlinks; cnt++) {
			sprintf(lname, "%s%d", fname, cnt);

			SAFE_STAT(cleanup, lname, &lbuf);
			if (fbuf.st_nlink <= 1 || lbuf.st_nlink <= 1 ||
			    (fbuf.st_nlink != lbuf.st_nlink)) {

				tst_resm(TFAIL,
					 "link(%s, %s[1-%d]) ret %ld for %d "
				         "files, stat values do not match %d %d",
					 fname, fname, nlinks,
					 TEST_RETURN, nlinks,
					 (int)fbuf.st_nlink, (int)lbuf.st_nlink);
				break;
			}
		}
		if (cnt >= nlinks) {
			tst_resm(TPASS,
				 "link(%s, %s[1-%d]) ret %ld for %d files,"
			         "stat linkcounts match %d",
				 fname, fname, nlinks, TEST_RETURN,
				 nlinks, (int)fbuf.st_nlink);
		}

		for (cnt = 1; cnt < nlinks; cnt++) {
			sprintf(lname, "%s%d", fname, cnt);
			SAFE_UNLINK(cleanup, lname);
		}
	}

	cleanup();
	tst_exit();
}

static void help(void)
{
	printf("  -N #links : create #links hard links every iteration\n");
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "%s_%d", BASENAME, getpid());
	SAFE_TOUCH(cleanup, fname, 0700, NULL);
}

static void cleanup(void)
{
	tst_rmdir();
}
