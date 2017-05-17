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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : flock05
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
 *	with LOCK_EX.
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
 *      flock05 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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

void setup(void);
void cleanup(void);

char *TCID = "flock05";
int TST_TOTAL = 2;
char filename[100];
int fd, fd1, status;

int main(int argc, char **argv)
{
	int lc, retval;
	pid_t pid;

	tst_parse_opts(argc, argv, NULL, NULL);

	/* global setup */
	setup();

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		/* Testing Shared lock on Exclusive Locked file */
		TEST(flock(fd, LOCK_EX));
		if (TEST_RETURN == 0) {

			pid = FORK_OR_VFORK();
			if (pid == 0) {
				fd1 = open(filename, O_RDWR);
				retval = flock(fd1, LOCK_SH | LOCK_NB);
				if (retval == -1) {
					tst_resm(TPASS,
						 "flock() failed to acquire shared lock on an already"
						 "exclusive locked file as expected");
				} else {
					tst_resm(TFAIL,
						 "flock() unexpectedly PASSED in acquiring shared lock on "
						 "an already exclusive locked file");
				}
				exit(0);
			} else {
				/* parent waiting */
				wait(&status);
			}

			/* Testing Exclusive lock on a Exclusive Locked file */
			pid = FORK_OR_VFORK();

			if (pid == 0) {
				fd1 = open(filename, O_RDWR);
				retval = flock(fd1, LOCK_EX | LOCK_NB);
				if (retval == -1) {
					tst_resm(TPASS,
						 "flock() failed to acquire exclusive lock on existing "
						 " exclusive locked file as expected");
				} else {
					tst_resm(TFAIL,
						 "flock() unexpectedly passed in acquiring exclusive lock on "
						 "an exclusive locked file");
				}
				exit(0);
			} else {
				/* parent waiting */
				wait(&status);
			}
			TEST(flock(fd, LOCK_UN));
		} else {
			tst_resm(TFAIL,
				 "flock() failed to acquire exclusive lock");
		}

	}

	close(fd);
	close(fd1);
	cleanup();
	tst_exit();

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* Create a unique temporary directory and chdir() to it. */
	tst_tmpdir();

	sprintf(filename, "flock05.%d", getpid());

	/* creating temporary file */
	fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0666);
	if (fd == -1) {
		tst_resm(TFAIL, "creating a new file failed");

		/* Removing temp dir */
		tst_rmdir();

	}
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 * 	completion or premature exit
 */
void cleanup(void)
{

	unlink(filename);

	tst_rmdir();

}
