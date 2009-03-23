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
 * Test Name: poll01
 *
 * Test Description:
 *  Verify that valid open file descriptor must be provided to poll() to
 *  succeed.
 *
 * Expected Result:
 *  poll should return the correct values when an valid file descriptor is
 *  provided.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  poll01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
 */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/poll.h>

#include "test.h"
#include "usctest.h"

#define BUF_SIZE	512

char *TCID = "poll01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int fildes[2];			/* file descriptors of the pipe. */
struct pollfd fds[1];		/* struct. for poll() */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counters */
	int length;		/* length of character string */
	char *msg;		/* message returned from parse_opts */
	pid_t cpid;		/* child process id */
	char write_buf[] = "Testing";	/* buffer string for write */
	char read_buf[BUF_SIZE];	/* buffer for read-end of pipe */
	int status;		/* exit status of child process */
	int rval;

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call poll() with the TEST macro.
		 */
		TEST(poll(fds, 1, -1));

		/* check return code of poll() */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "poll() failed on write, errno=%d"
				 " : %s", TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		/* write the message to the pipe */
		if (write(fildes[1], write_buf, sizeof(write_buf))
		    < sizeof(write_buf)) {
			tst_brkm(TBROK, cleanup, "write() failed on write "
				 "to pipe, error:%d", errno);
		}

		length = sizeof(write_buf);

		/* Fork child process */
		if ((cpid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (cpid == 0) {	/* Child process */
			/*
			 * close writing end of pipe and read from
			 * the pipe
			 */
			if (close(fildes[1]) == -1) {
				tst_brkm(TFAIL, NULL, "close() failed on write "
					 "endof pipe, errno:%d", errno);
				exit(1);
			}

			/*
			 * Set poll() data structures to check
			 * if data is present on read
			 */
			fds[0].fd = fildes[0];
			fds[0].events = POLLIN;

			/*
			 * If data are present, then read the data.  If poll()
			 * and read() return expected values, then the
			 * functionality of poll() is correct.
			 */
			rval = (poll(fds, 1, -1));

			if (rval == -1) {
				tst_resm(TFAIL, "poll() failed on read - "
					 "errno=%d : %s",
					 TEST_ERRNO, strerror(errno));
				exit(1);
			}

			/* Read data from read end of pipe */
			if (read(fildes[0], read_buf, sizeof(read_buf)) !=
			    sizeof(write_buf)) {
				tst_brkm(TFAIL, NULL, "read() failed - "
					 "error:%d", errno);
				exit(1);
			}

			/* Now, do the actual comparision */
			if (strcmp(read_buf, write_buf)) {
				tst_resm(TFAIL, "Data from reading pipe "
					 "are different");
				printf
				    (" read_buf is %s\n write_buf is %s\n length is %d\n",
				     read_buf, write_buf, length);
				exit(1);
			}

			/* Everything is fine, exit normally */
			exit(0);
		} else {	/* Parent process */
			/* Wait for child to complete execution */
			wait(&status);

			if (WEXITSTATUS(status) == 1) {
				tst_resm(TFAIL, "child exited abnormally");
			} else {
				tst_resm(TPASS,
					 "Functionality of poll() successful");
			}
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 * 	     Creat read/write pipe using pipe().
 * 	     Set poll data structures to check writing to the pipe.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Creat read/write pipe */
	if (pipe(fildes) < 0) {
		tst_brkm(TBROK, tst_exit,
			 "pipe() failed to create interprocess channel");
	}

	/* Set poll data structures */
	fds[0].fd = fildes[1];
	fds[0].events = POLLOUT;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 * 	       close read end of pipe if still open.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* close read end of pipe if still open */
	if (close(fildes[0]) < 0) {
		tst_brkm(TFAIL, NULL, "close() failed on read-end of pipe, "
			 "errno:%d", errno);
	}

	/* exit with return code appropriate for results */
	tst_exit();
}
