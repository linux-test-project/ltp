/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 * 	creat01.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of the creat(2) system call.
 *
 * ALGORITHM
 * 	1.	creat() a file using 0444 mode, write to the fildes, write
 * 		should return a positive count.
 *
 * 	2.	creat() should truncate a file to 0 bytes if it already
 *		exists, and should not fail.
 *
 * USAGE:  <for command-line>
 *  creat01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);
void functest1(void);
void functest2(void);

char *TCID = "creat01";
int TST_TOTAL = 2;
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

char filename[40];
int fd;

#define MODE1 0644
#define MODE2 0444

struct test_case_t {
	char *fname;
	int mode;
	void (*functest)();
} TC[] = {
	{ filename, MODE1, functest1},
	{ filename, MODE2, functest2}
};

int main(int ac, char **av)
{
	int i;
	int lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(fd = creat(filename, TC[i].mode));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL|TTERRNO, "creat failed");
				continue;
			}

			if (STD_FUNCTIONAL_TEST)
				(*TC[i].functest)();
			else
				tst_resm(TPASS, "call succeeded");
			close(fd);
		}
	}
	cleanup();

	tst_exit();
}

void functest1()
{
	if (write(TEST_RETURN, "A", 1) != 1)
		tst_resm(TFAIL, "write was unsuccessful");
	else
		tst_resm(TPASS, "file was created and written to successfully");
}

void functest2()
{
	struct stat buf;

	if (stat(filename, &buf) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "stat failed");
	if (buf.st_size == 0)
		tst_resm(TPASS, "creat truncated existing file to 0 bytes");
	else
		tst_resm(TFAIL, "creat FAILED to truncate file to 0 bytes");
}

void setup()
{
	fd = -1;

	tst_require_root(NULL);

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getpwnam failed");
	if (setuid(ltpuser->pw_uid) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "setuid failed");

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(filename, "creat01.%d", getpid());
}

void cleanup()
{
	TEST_CLEANUP;

	if (fd != -1)
		close(fd);

	unlink(filename);

	tst_rmdir();
}