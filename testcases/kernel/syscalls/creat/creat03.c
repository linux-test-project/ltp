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
 * 	creat03.c
 *
 * DESCRIPTION
 *	Testcase to check whether the sticky bit cleared.
 *
 * ALGORITHM
 * 	Creat a new file, fstat.st_mode should have the 01000 bit off
 *
 * USAGE:  <for command-line>
 *  creat03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "test.h"
#include "usctest.h"

char *TCID = "creat03";		/* Test program identifier */
int TST_TOTAL = 1;		/* Total number of test cases */
extern int Tst_count;		/* Test case counter */

char pfilname[40] = "";
#define FMODE	0444

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	struct stat statbuf;
	unsigned short filmode;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(creat(pfilname, FMODE));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "Cannot creat %s", pfilname);
			continue;
		 /*NOTREACHED*/}

		if (STD_FUNCTIONAL_TEST) {
			if (fstat(TEST_RETURN, &statbuf) == -1) {
				tst_brkm(TBROK, cleanup, "fstat() failed");
			}
			filmode = statbuf.st_mode;
			tst_resm(TINFO, "Created file has mode = 0%o", filmode);
			if ((filmode & S_ISVTX) != 0) {
				tst_resm(TFAIL, "save text bit not cleared");
			} else {
				tst_resm(TPASS, "save text bit cleared");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}

		close(TEST_RETURN);
		/* clean up things in case we are looping */
		if (unlink(pfilname) == -1) {
			tst_brkm(TBROK, cleanup, "couldn't remove file");
		}
	}
	cleanup();

	return 0;
 /*NOTREACHED*/}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp dir and cd to it */
	tst_tmpdir();

	sprintf(pfilname, "./creat4.%d", getpid());
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion or
 *	       premature exit
 */
void cleanup(void)
{
	TEST_CLEANUP;

	/* remove the tmp dir and all its files */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
