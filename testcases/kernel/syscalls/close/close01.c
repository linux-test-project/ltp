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
 * 	close01.c
 *
 * DESCRIPTION
 * 	Test that closing a regular file and a pipe works correctly
 *
 * ALGORITHM
 * 	Creat a file, and dup() a fildes
 * 	Open a pipe
 *	call close() using the TEST macro
 *	if the call fails
 *	   issue a FAIL message and continue
 *	else if STD_FUNCTIONAL_TEST
 *	   attempt to close the file/pipe again
 *	   if there is an error
 *	      issue a PASS message
 *	   else
 *	      issue a FAIL message
 *	else
 *	   issue a PASS message
 *
 *
 * USAGE:  <for command-line>
 *  close01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

void cleanup(void);
void setup(void);

char *TCID = "close01";
int TST_TOTAL = 2;

char fname[40] = "";

int fild, newfd, pipefildes[2];

struct test_case_t {
	int *fd;
	char *type;
} TC[] = {
	/* file descriptor for a regular file */
	{ &newfd, "file"},
	/* file descriptor for a pipe */
	{ &pipefildes[0], "pipe"}
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

		if ((fild = creat(fname, 0777)) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "can't open file %s",
			    fname);

		if ((newfd = dup(fild)) == -1)
			tst_brkm(TBROK|TERRNO, cleanup,
			    "can't dup the file des");

		if (pipe(pipefildes) == -1) {
			tst_brkm(TBROK|TERRNO, cleanup,
			    "can't open pipe");
		}

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(close(*TC[i].fd));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "call failed unexpectedly");
				continue;
			}

			if (STD_FUNCTIONAL_TEST) {
				if (close(*TC[i].fd) == -1) {
					tst_resm(TPASS, "%s appears closed",
						 TC[i].type);
				} else {
					tst_resm(TFAIL, "%s close succeeded on"
						 "second attempt", TC[i].type);
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		}

	}
	cleanup();

	tst_exit();
 }

void setup(void)
{
	int mypid;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;

	tst_tmpdir();

	mypid = getpid();
	sprintf(fname, "fname.%d", mypid);
}

void cleanup(void)
{
	close(fild);

	TEST_CLEANUP;

	tst_rmdir();

}