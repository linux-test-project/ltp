/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/**********************************************************
 * 
 *    TEST IDENTIFIER   : flock03
 * 
 *    EXECUTED BY       : anyone
 * 
 *    TEST TITLE        : Error condition test for flock(2)
 * 
 *    TEST CASE TOTAL   : 1
 * 
 *    AUTHOR            : Paul Larson <plars@linuxtestproject.org>
 * 
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 * 	This test verifies that flock cannot unlock a file locked
 * 	by another task
 *		
 *	Test:
 * 		Fork a child processes
 * 		The parent flocks a file with LOCK_EX
 * 		Child waits for that to happen, then checks to make sure
 * 		it is locked.  Child then tries to unlock the file.  The 
 * 		test passes if child is unable to unlock the file.
 * 
 * USAGE:  <for command-line>
 *      flock03 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *                      where,  -c n : Run n copies concurrently
 *                              -f   : Turn off functional testing
 *    				-e   : Turn on errno logging
 *                              -h   : Show help screen
 *				-i n : Execute test n times
 *                              -I x : Execute test for x seconds
 *                              -p   : Pause for SIGUSR1 before starting
 *                              -P x : Pause for x seconds between iterations
 *                              -t   : Turn on syscall timing
 *
 ****************************************************************/



#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);
void childfunc(void);


char *TCID = "flock03";			/* Test program identifier */
int TST_TOTAL = 3;			/* Total number of test cases */
extern int Tst_count;
char filename[100];
int fd;

int main(int argc, char **argv)
{
	int lc;		/* loop counter */
	char *msg;	/* message returned from parse_opts */
	pid_t pid;
	int status;
	int fd;		/* for opening the temporary file */


	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *)NULL, NULL)) !=
	    (char *) NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		/*NOTREACHED*/
	}

	setup();

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		pid = fork();
		if(pid == -1)
			tst_brkm(TFAIL, cleanup, "fork() failed, errno %d",
					errno);
		if(pid == 0)
			childfunc();

		/* PARENT */
		fd = open(filename, O_RDWR);
		if(fd == -1)
			tst_brkm(TFAIL, cleanup, "parent failed to open the"
					"file, errno %d", errno);
		TEST(flock(fd, LOCK_EX | LOCK_NB));
		if(TEST_RETURN != 0)
			tst_resm(TFAIL, "Initial attempt to flock() failed, "
					"errno %d",TEST_ERRNO); 
		else
			tst_resm(TPASS, "Initial attempt to flock() passed");

		if((waitpid(pid, &status, 0)) < 0) {
			tst_resm(TFAIL, "wait() failed");
			continue;
		}
		if ((WIFEXITED(status)) && (WEXITSTATUS(status) == 0))
			tst_resm(TPASS, "flock03 Passed");
		else
			tst_resm(TFAIL, "flock03 Failed");

	}/* End of TEST_LOOPING */

	cleanup();

	return 0;

}

void childfunc(void )
{
	int fd;

	/* give the parent a chance to lock the file */
	sleep(2);

	fd = open(filename, O_RDWR);
	if(fd == -1)
		tst_brkm(TFAIL, cleanup, "parent failed to open the"
				"file, errno %d", errno);
	if(flock(fd, LOCK_EX | LOCK_NB) != -1)
		tst_resm(TFAIL, "The file was not already locked");

	TEST(flock(fd, LOCK_UN));
	if(TEST_RETURN != -1)
		tst_resm(TFAIL, "File unlock by another process allowed");
	else
		tst_resm(TPASS, "File unlock by another process not allowed");
	tst_exit();
	/* NOT REACHED */
	return;
}


/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* Create a unique temporary directory and chdir() to it. */
	tst_tmpdir();

	sprintf(filename, "flock03.%d", getpid());

	/* creating temporary file */
	fd = creat(filename, 0666);
	if (fd < 0) {
		tst_resm(TFAIL, "creating a new file failed");
		
		TEST_CLEANUP;

		/* Removing temp dir */
		tst_rmdir();

		/* exit with return code appropriate for result */
		tst_exit();
	}
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 * 	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	unlink(filename);
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
	/*NOTREACHED*/
}
