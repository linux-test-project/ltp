/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : flock04
 *
 *    EXECUTED BY       : anyone
 *
 *    TEST TITLE        : Testing different locks on flock(2)
 *
 *    TEST CASE TOTAL   : 2
 *
 *    AUTHOR            : Vatsal Avasthi <vatsal.avasthi@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 * 	Tests to verify flock(2) behavior with different locking combinations along
 *	with LOCK_SH.
 *    $
 *	Setup:
 *        Setup signal handling.
 *        Pause for SIGUSR1 if option specified.
 *        Create a temporary directory and chdir to it.
 * 	  Create a temporary file
 *
 *	Test:
 *	Loop if proper options are given.
 *		Parent flocks(2) a file
 *		fork() a child process
 * 		Child tries to flock() the already flocked file with different types of locks
 *		Check return code, if system call failed (return == -1)
 *				Log the error number and issue a FAIL message
 *		otherwise issue a PASS message
 *
 *      Cleanup:
 *        Print errno log and/or timing stats if options given
 *	  Deletes temporary directory.
 *
 * USAGE:  <for command-line>
 *      flock04 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *                      where,  -c n : Run n copies concurrently.
 *                              -f   : Turn off functional testing
 *    				-e   : Turn on errno logging.
 *                              -h   : Show help screen                        $
 *				-i n : Execute test n times.
 *                              -I x : Execute test for x seconds.
 *                              -p   : Pause for SIGUSR1 before starting
 *                              -P x : Pause for x seconds between iterations.
 *                              -t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);

int exp_enos[] = { EWOULDBLOCK, EAGAIN, 0 };

char *TCID = "flock04";		/* Test program identifier */
int TST_TOTAL = 2;		/* Total number of test cases */
char filename[100];
int fd, fd1, status;

int main(int argc, char **argv)
{
	int lc, retval;
	char *msg;		/* message returned from parse_opts */
	pid_t pid;

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(flock(fd, LOCK_SH));
		if (TEST_RETURN == 0) {

			pid = FORK_OR_VFORK();
			if (pid == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "fork failed");
			if (pid == 0) {
				fd1 = open(filename, O_RDONLY);
				retval = flock(fd1, LOCK_SH | LOCK_NB);
				if (retval == -1)
					tst_resm(TFAIL,
						 "flock() FAILED to acquire shared lock on existing "
						 "Share Locked file");
				else
					tst_resm(TPASS,
						 "flock() PASSED in acquiring shared lock on "
						 "Share Locked file");
				exit(0);
			} else
				if (wait(&status) == -1)
					tst_brkm(TBROK|TERRNO, cleanup,
					    "wait failed");

			pid = FORK_OR_VFORK();
			if (pid == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "fork failed");

			if (pid == 0) {
				fd1 = open(filename, O_RDWR);
				retval = flock(fd1, LOCK_EX|LOCK_NB);
				if (retval == -1) {
					tst_resm(TPASS,
						 "flock() failed to acquire exclusive lock on existing "
						 "share locked file as expected");
				} else {
					tst_resm(TFAIL,
						 "flock() unexpectedly passed in acquiring exclusive lock on "
						 "Share Locked file");
				}
				exit(0);
			} else
				if (wait(&status) == -1)
					tst_resm(TBROK|TERRNO, "wait failed");
			TEST(flock(fd, LOCK_UN));
		} else
			tst_resm(TFAIL|TERRNO, "flock failed");

		close(fd);
		close(fd1);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(filename, "flock04.%d", getpid());

	fd = creat(filename, 0666);
	if (fd == -1)
		tst_brkm(TFAIL, cleanup, "creating a new file failed");
}

void cleanup(void)
{
	TEST_CLEANUP;

	unlink(filename);

	tst_rmdir();
}
