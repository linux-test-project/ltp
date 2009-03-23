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
 *	write05.c
 *
 * DESCRIPTION
 *	Check the return value, and errnos of write(2)
 *	- when the file descriptor is invalid - EBADF
 *	- when the buf parameter is invalid - EFAULT
 *	- on an attempt to write to a pipe that is not open for reading - EPIPE
 *
 * ALGORITHM
 *	Attempt to write on a file with negative file descriptor, check for -1
 *	Attempt to write on a file with invalid buffer, check for -1
 *	Open a pipe and close the read end, attempt to write to the write
 *	end, check for -1.
 *
 * USAGE:  <for command-line>
 *      write05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -e   : Turn on errno logging.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *      04/2002 wjhuie sigset cleanups
 *      08/2007 Ricardo Salveti de Araujo <rsalveti@linux.vnet.ibm.com>
 *		- Closing the fd before removing the file
 *
 * Restrictions
 *	None
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <wait.h>
#include <sys/mman.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);

/* 0 terminated list of expected errnos */
int exp_enos[] = { 9, 14, 32, 0 };

char *TCID = "write05";		/* Test program identifier */
int TST_TOTAL = 1;		/* Total number of test cases */
extern int Tst_count;
char filename[100];
int fd;

char *bad_addr = 0;

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	char pbuf[BUFSIZ];
	int pipefildes[2];
	int status, pid;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	/* global setup */
	setup();

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

//block1:
		tst_resm(TINFO, "Enter Block 1: test with bad fd");
		if (write(-1, pbuf, 1) != -1) {
			tst_resm(TFAIL, "write of invalid fd passed");
		} else {
			TEST_ERROR_LOG(errno);
			if (errno != EBADF) {
				tst_resm(TFAIL, "expected EBADF got %d", errno);
			}
			tst_resm(TPASS, "received EBADF as expected.");
		}
		tst_resm(TINFO, "Exit Block 1");

//block2:
		tst_resm(TINFO, "Enter Block 2: test with a bad address");
		fd = creat(filename, 0644);
		if (fd < 0) {
			tst_resm(TFAIL, "creating a new file failed");
			cleanup();
		 /*NOTREACHED*/}
		if (write(fd, bad_addr, 10) != -1) {
			tst_resm(TFAIL, "write() on an invalid buffer "
				 "succeeded, but should have failed");
			cleanup();
		 /*NOTREACHED*/} else {
			TEST_ERROR_LOG(errno);
			if (errno != EFAULT) {
				tst_resm(TFAIL, "write() returned illegal "
					 "errno: expected EFAULT, got %d",
					 errno);
				cleanup();
			 /*NOTREACHED*/}
			tst_resm(TPASS, "received EFAULT as expected.");
		}
		tst_resm(TINFO, "Exit Block 2");

//block3:
		tst_resm(TINFO, "Enter Block 3: test with invalid pipe");
		if ((pid = FORK_OR_VFORK()) == 0) {	/* child */
			if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
				tst_resm(TINFO, "signal failed");
			}
			if (pipe(pipefildes) == -1) {
				tst_brkm(TBROK, NULL, "can't open pipe");
				exit(errno);
			}
			close(pipefildes[0]);
			if (write(pipefildes[1], pbuf, 1) != -1) {
				tst_resm(TFAIL, "write on read-closed"
					 "pipe succeeded");
				exit(-1);
			} else {
				if (errno != EPIPE) {
					tst_resm(TFAIL, "write() failed to set"
						 " errno to EPIPE, got: %d",
						 errno);
					exit(errno);
				}
				exit(0);
			}
		} else {
			if (pid < 0) {
				tst_resm(TFAIL, "Fork failed");
			}
			wait(&status);
			if (WIFSIGNALED(status) == SIGPIPE) {
				tst_resm(TFAIL, "child set SIGPIPE in exit");
			} else if (WEXITSTATUS(status) != 0) {
				TEST_ERROR_LOG(WEXITSTATUS(status));
				tst_resm(TFAIL, "exit status from child "
					 "expected 0 got %d", status >> 8);
			} else {
				TEST_ERROR_LOG(EPIPE);
				tst_resm(TPASS, "received EPIPE as expected.");
			}
			tst_resm(TINFO, "Exit Block 3");
		}
		close(fd);
	}
	cleanup();
	 /*NOTREACHED*/ return 0;
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* Create a unique temporary directory and chdir() to it. */
	tst_tmpdir();

	sprintf(filename, "write05.%d", getpid());

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		printf("mmap failed\n");
	}

}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the file descriptor befor removing the file */
	close(fd);

	unlink(filename);
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}
