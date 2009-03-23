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
 *	pipe10.c
 *
 * DESCRIPTION
 *	Check that parent can open a pipe and have a child read from it
 *
 * ALGORITHM
 *	Parent opens pipe, child reads. Passes if child can read all the
 *	characters written by the parent.
 *
 * USAGE:  <for command-line>
 *  pipe10 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *	None
 */
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

char *TCID = "pipe10";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int fd[2];		/* fds for pipe read/write */
	char wrbuf[BUFSIZ], rebuf[BUFSIZ];
	int red, written;	/* no of chars read and */
	/* written to pipe */
	int length, greater, forkstat;
	int retval = 0, status, e_code;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(pipe(fd));

		if (TEST_RETURN == -1) {
			retval = 1;
			tst_resm(TFAIL, "pipe creation failed");
			continue;
		}

		if (!STD_FUNCTIONAL_TEST) {
			tst_resm(TPASS, "call succeeded");
			continue;
		}

		strcpy(wrbuf, "abcdefghijklmnopqrstuvwxyz");
		length = strlen(wrbuf) + 1;

		written = write(fd[1], wrbuf, length);

		/* did write write at least some chars */
		if ((written < 0) || (written > length)) {
			tst_brkm(TBROK, cleanup, "write to pipe failed");
		}

		forkstat = FORK_OR_VFORK();

		if (forkstat == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		 /*NOTREACHED*/}

		if (forkstat == 0) {	/* child */
			red = safe_read(fd[0], rebuf, written);

			/* did read , get at least some chars */
			if ((red < 0) || (red > written)) {
				tst_brkm(TBROK, cleanup, "read pipe failed");
			}

			greater = strcmp(rebuf, wrbuf);

			/* are the strings written and read equal */
			if (greater == 0) {
				tst_resm(TPASS, "functionality is correct");
			} else {
				retval = 1;
				tst_resm(TFAIL, "read & write strings do "
					 "not match");
			}
			exit(retval);
		} else {	/* parent */
			/* wait for the child to finish */
			wait(&status);
			/* make sure the child returned a good exit status */
			e_code = status >> 8;
			if (e_code != 0) {
				tst_resm(TFAIL, "Failures reported above");
			}
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
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
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
