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
 *	pipe01.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of the pipe(2) syscall:
 *	Check that both ends of the pipe (both file descriptors) are
 *	available to a process opening the pipe.
 *
 * ALGORITHM
 *	Write a string of characters down a pipe; read the string from the
 *	other file descriptor. Test passes if both can be done, as reported
 *	by the number of characters written and read.
 *
 * USAGE:  <for command-line>
 *  pipe01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * RESTRICITONS
 *	NONE
 */
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

char *TCID = "pipe01";
int TST_TOTAL = 1;

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

	int fildes[2];		/* fds for pipe read and write */
	char wrbuf[BUFSIZ], rebuf[BUFSIZ];
	int red, written;	/* no. of chars read/written to pipe */
	int greater, length;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(pipe(fildes));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "pipe() failed unexpectedly - errno %d",
				 TEST_ERRNO);
			continue;
		}

		if (STD_FUNCTIONAL_TEST) {

			strcpy(wrbuf, "abcdefghijklmnopqrstuvwxyz");
			length = strlen(wrbuf);

			if ((written = write(fildes[1], wrbuf, length)) == -1) {
				tst_brkm(TBROK, cleanup, "write() failed");
			}

			if (written < 0 || written > 26) {
				tst_resm(TFAIL, "Condition #1 test failed");
				continue;
			}

			if ((red = safe_read(fildes[0], rebuf, written)) == -1) {
				tst_brkm(TBROK|TERRNO, cleanup, "read() failed");
			}

			if (red < 0 || red > written) {
				tst_resm(TFAIL, "Condition #2 test failed");
				continue;
			}

			/* are the strings written and read equal */
			if ((greater = strcmp(rebuf, wrbuf)) != 0) {
				tst_resm(TFAIL, "Condition #3 test failed");
				continue;
			}
			tst_resm(TPASS, "pipe() functionality is correct");
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

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
}