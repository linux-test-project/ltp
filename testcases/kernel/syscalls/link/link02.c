/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR		: William Roske
 *  CO-PILOT		: Dave Fenner
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
 * Tests that link(2) succeds.
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

char *TCID = "link02";
int TST_TOTAL = 1;

#define OLDPATH "oldpath"
#define NEWPATH "newpath"

static void verify_link(void)
{
	struct stat fbuf, lbuf;

	TEST(link(OLDPATH, NEWPATH));

	if (TEST_RETURN == 0) {
		SAFE_STAT(cleanup, OLDPATH, &fbuf);
		SAFE_STAT(cleanup, NEWPATH, &lbuf);
		if (fbuf.st_nlink > 1 && lbuf.st_nlink > 1 &&
		    fbuf.st_nlink == lbuf.st_nlink) {
			tst_resm(TPASS, "link("OLDPATH","NEWPATH") "
			         "returned 0 and link counts match");
		} else {
			tst_resm(TFAIL, "link("OLDPATH","NEWPATH") returned 0"
				 " but stat lin count do not match %d %d",
				 (int)fbuf.st_nlink, (int)lbuf.st_nlink);
		}
		SAFE_UNLINK(cleanup, NEWPATH);
	} else {
		tst_resm(TFAIL | TTERRNO,
		         "link("OLDPATH","NEWPATH") returned %ld",
		         TEST_RETURN);
	}
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		verify_link();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_TOUCH(cleanup, OLDPATH, 0700, NULL);
}

static void cleanup(void)
{
	tst_rmdir();
}
