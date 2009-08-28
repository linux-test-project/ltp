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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);
void functest1(void);
void functest2(void);

char *TCID = "creat01";
int TST_TOTAL = 2;
extern int Tst_count;
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

char filename[40];
int fd[2];

#define MODE1 0644
#define MODE2 0444

struct test_case_t {
	char *fname;
	int mode;
	void (*functest) ();
} TC[] = {
	/* creat() the file and write to it */
	{
	filename, MODE1, functest1},
	    /* creat() the same file and check that it is now 0 length */
	{
	filename, MODE2, functest2}
};

int main(int ac, char **av)
{
	int i;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* set "tstdir", and "testfile" variables */

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* loop through the test cases */

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(fd[i] = creat(filename, TC[i].mode));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "Could not creat file %s",
					 filename);
				continue;
			}

			if (STD_FUNCTIONAL_TEST) {
				(*TC[i].functest) ();
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		}
	}
	cleanup();

	return 0;
 /*NOTREACHED*/}

/*
 * functest1() - check the functionality of the first test by making sure
 *		 that a write to the file succeeds
 */
void functest1()
{
	if (write(TEST_RETURN, "A", 1) != 1) {
		tst_resm(TFAIL, "write was unsuccessful");
	} else {
		tst_resm(TPASS, "file was created and written to successfully");
	}
}

/*
 * functest2() - check the functionality of the second test by making sure
 *		 that the file is now 0 length
 */
void functest2()
{
	struct stat buf;

	if (stat(filename, &buf) < 0) {
		tst_brkm(TBROK, cleanup, "failed to stat test file");
	 /*NOTREACHED*/}
	if (buf.st_size != 0) {
		tst_resm(TFAIL, "creat() FAILED to truncate "
			 "file to zero bytes");
	} else {
		tst_resm(TPASS, "creat() truncated existing file to 0 bytes");
	}
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{
	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1)
		tst_resm(TINFO|TERRNO, "setuid(%d) failed", ltpuser->pw_uid);

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified */
	TEST_PAUSE;

	tst_tmpdir();

	sprintf(filename, "creat01.%d", getpid());
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	int i;
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	for (i = 0; i < TST_TOTAL; i++) {
		close(fd[i]);
	}

	unlink(filename);

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
