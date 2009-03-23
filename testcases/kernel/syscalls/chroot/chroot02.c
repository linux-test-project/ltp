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
 *	chroot02.c
 *
 * DESCRIPTION
 *	Test functionality of chroot(2)
 *
 * ALGORITHM
 *	Change root directory and then stat a file.
 *
 * USAGE:  <for command-line>
 *  chroot02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	04/2003 Modified by Manoj Iyer - manjo@mail.utexas.edu
 *	Change testcase to chroot into a temporary directory
 *	and stat() a known file.
 *
 * RESTRICTIONS
 *	NONE
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>
#include <fcntl.h>

char *TCID = "chroot02";
int TST_TOTAL = 1;
int fileHandle = 0;
extern int Tst_count;
extern char *TESTDIR;

char tmpbuf[50];
struct stat buf;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int pid, e_code, status, retval = 0;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "Could not fork");
		}

		if (pid == 0) {	/* child */

			TEST(chroot(TESTDIR));

			if (TEST_RETURN == -1) {
				retval = 1;
				tst_resm(TFAIL, "chroot(2) failed errno = %d",
					 TEST_ERRNO);
				continue;
			}

			if (STD_FUNCTIONAL_TEST) {
				if (stat("/chroot02_testfile", &buf) == -1) {
					retval = 1;
					tst_resm(TFAIL, "stat(2) failed errno "
						 "= %d", errno);
				} else {
					tst_resm(TPASS, "We appear to be in "
						 "the right place");
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}
			exit(retval);
		}

		/* parent */
		wait(&status);
		/* make sure the child returned a good exit status */
		e_code = status >> 8;
		if (e_code != 0) {
			tst_resm(TFAIL, "Failures reported above");
		}
	}
	cleanup();

	return 0;
 /*NOTREACHED*/}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* make sure the process ID is root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, cleanup, "Test must be run as root.");
	}
	/*
	 * Now ensure that the testfile exists
	 */
	tst_tmpdir();
	sprintf(tmpbuf, "%s%s", TESTDIR, "/chroot02_testfile");
	if ((fileHandle = creat(tmpbuf, 0777)) == -1)
		tst_brkm(TBROK, cleanup, "failed to create tmporary file %s",
			 tmpbuf);
	if (stat(tmpbuf, &buf) != 0) {
		tst_brkm(TBROK, cleanup, "%s does not exist", tmpbuf);
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	close(fileHandle);

	TEST_CLEANUP;
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
