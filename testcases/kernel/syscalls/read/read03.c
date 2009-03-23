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
 *	read03.c
 *
 * DESCRIPTION
 *	Testcase to check that read() sets errno to EAGAIN
 *
 * ALGORITHM
 *	Create a named pipe (fifo), open it in O_NONBLOCK mode, and
 *	attempt to read from it, without writing to it. read() should fail
 *	with EAGAIN.
 *
 * USAGE:  <for command-line>
 *  read03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

char *TCID = "read03";
int TST_TOTAL = 1;
extern int Tst_count;

char fifo[100] = "fifo";
int rfd, wfd;
struct stat buf;

int exp_enos[] = { EAGAIN, 0 };

void alarm_handler();
void setup();
void cleanup();

int main(int ac, char **av)
{
	int lc;
	char *msg;

	int c;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/*
	 * The following loop checks looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(read(rfd, &c, 1));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "read() failed to fail when nothing "
				 "is written to a pipe");
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		if (TEST_ERRNO != EAGAIN) {
			tst_resm(TFAIL, "read set bad errno, expected "
				 "EAGAIN, got %d", TEST_ERRNO);
		} else {
			tst_resm(TINFO, "read() succeded in setting errno to "
				 "EAGAIN");
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* create a temporary filename */
	sprintf(fifo, "%s.%d", fifo, getpid());

	/* Create a temporary directory and cd to it */
	tst_tmpdir();

	if (mknod(fifo, S_IFIFO | 0777, 0) < 0) {
		tst_brkm(TBROK, cleanup, "mknod() failed, errno: %d", errno);
	 /*NOTREACHED*/}
	if (stat(fifo, &buf) != 0) {
		tst_brkm(TBROK, cleanup, "stat() failed, errno: %d", errno);
	 /*NOTREACHED*/}
	if ((buf.st_mode & S_IFIFO) == 0) {
		tst_brkm(TBROK, cleanup, "Mode does not indicate fifo file");
	 /*NOTREACHED*/}

	rfd = open(fifo, O_RDONLY | O_NONBLOCK);
	wfd = open(fifo, O_WRONLY | O_NONBLOCK);
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

	close(rfd);
	close(wfd);
	unlink(fifo);

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
