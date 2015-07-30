/*
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
 * 06/2002 Ported by Jacky Malcles
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 * Verify that, link() fails with -1 and sets errno to EACCES when Write access
 * to the directory containing newpath is not allowed for the process's
 * effective uid.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"

#define NOBODY_USER	99
#define MODE_TO S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IXOTH|S_IROTH

static void setup(void);
static void cleanup(void);

char *TCID = "link06";
int TST_TOTAL = 1;

#define OLDPATH "oldpath"
#define NEWPATH "newpath"

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(link(OLDPATH, NEWPATH));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "link() returned %ld,"
				 "expected -1, errno=%d", TEST_RETURN,
				 EACCES);
		} else {
			if (TEST_ERRNO == EACCES) {
				tst_resm(TPASS, "link() fails with expected "
					 "error EACCES errno:%d", TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "link() fails with "
					 "errno=%d, expected errno=%d",
					 TEST_ERRNO, EACCES);
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct passwd *nobody_pwd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	/* Modify mode permissions on test directory */
	SAFE_CHMOD(cleanup, ".", MODE_TO);

	SAFE_TOUCH(cleanup, OLDPATH, 0777, NULL);
	nobody_pwd = SAFE_GETPWNAM(cleanup, "nobody");
	SAFE_SETEUID(cleanup, nobody_pwd->pw_uid);
}

static void cleanup(void)
{
	if (seteuid(0))
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	tst_rmdir();
}
