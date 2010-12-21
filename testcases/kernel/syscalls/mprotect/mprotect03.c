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
 *	mprotect03.c
 *
 * DESCRIPTION
 *	Testcase to check the mprotect(2) system call.
 *
 * ALGORITHM
 *	Create a shared mapped file region with PROT_READ | PROT_WRITE
 *	using the mmap(2) call. Then, use mprotect(2) to disable the
 *	write permission on the mapped region. Then, attempt to write to
 *	the mapped region using memcpy(). This would generate a sigsegv.
 *	Since the sigsegv is generated, this needs to be done in a child
 *	process (as sigsegv would repeatedly be generated). The testcase
 *	succeeds only when this sigsegv is generated while attempting to
 *	memcpy() on a shared region with only read permission.
 *
 * USAGE:  <for command-line>
 *  mprotect03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *      05/2002 changed over to use tst_sig instead of sigaction
 *
 * RESTRICTIONS
 *	None
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <limits.h>		/* for PAGESIZE */
#include <signal.h>
#include <wait.h>
#include "test.h"
#include "usctest.h"

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif
#define FAILED 1

void cleanup(void);
void setup(void);

char *TCID = "mprotect03";
int TST_TOTAL = 1;
int status;
char file1[BUFSIZ];


#ifndef UCLINUX

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	char *addr;
	int fd, pid;
	char *buf = "abcdefghijklmnopqrstuvwxyz";

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((fd = open(file1, O_RDWR | O_CREAT, 0777)) < 0) {	//mode must be specified when O_CREAT is in the flag
			tst_brkm(TBROK, cleanup, "open failed");
		 }

		(void)write(fd, buf, strlen(buf));

		/*
		 * mmap the PAGESIZE bytes as read only.
		 */
		addr = mmap(0, strlen(buf), PROT_READ | PROT_WRITE, MAP_SHARED,
			    fd, 0);
		if (addr < 0) {
			tst_brkm(TBROK, cleanup, "mmap failed");
		 }

		/*
		 * Try to change the protection to WRITE.
		 */
		TEST(mprotect(addr, strlen(buf), PROT_READ));

		if (TEST_RETURN != -1) {
			if (STD_FUNCTIONAL_TEST) {
				if ((pid = FORK_OR_VFORK()) == -1) {
					tst_brkm(TBROK, cleanup, "fork failed");
				}

				if (pid == 0) {	/* child */
					(void)memcpy((void *)addr, (void *)buf,
						     strlen(buf));
					tst_resm(TINFO, "memcpy() did "
						 "not generate SIGSEGV");
					exit(1);
				 }

				/* parent */
				(void)waitpid(pid, &status, 0);
				if (WEXITSTATUS(status) != 0) {
					tst_resm(TFAIL, "child returned "
						 "unexpected status");
				} else {
					tst_resm(TPASS, "SIGSEGV generated "
						 "as expected");
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		} else {
			tst_resm(TFAIL, "mprotect failed "
				 "unexpectedly, errno: %d", errno);
		 }

		/* clean up things in case we are looping */
		if (munmap(addr, strlen(buf)) == -1) {
			tst_brkm(TBROK, cleanup, "munamp failed");
		}
		if (close(fd) == -1) {
			tst_brkm(TBROK, cleanup, "close failed");
		}
		if (unlink(file1) == -1) {
			tst_brkm(TBROK, cleanup, "unlink failed");
		}
	}
	cleanup();
	tst_exit();

}

#else

int main()
{
	tst_resm(TINFO, "Ignore this test on uClinux");
	tst_exit();
}

#endif /* UCLINUX */

void sighandler(int sig)
{
	if (sig == SIGSEGV) {
		tst_resm(TINFO, "received signal: SIGSEGV");
		tst_exit();
	} else
		tst_brkm(TBROK, 0, "Unexpected signal %d received.", sig);
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{
	tst_sig(FORK, sighandler, NULL);

	TEST_PAUSE;

	tst_tmpdir();		/* create a temporary directory, cd to it */

	sprintf(file1, "mprotect03.tmp.%d", getpid());
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void cleanup()
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	tst_rmdir();

}
