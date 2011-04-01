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
 *	mprotect02.c
 *
 * DESCRIPTION
 *	Testcase to check the mprotect(2) system call.
 *
 * ALGORITHM
 *	Create a mapped region using mmap with READ permission.
 *	Try to write into that region in a child process using memcpy.
 *	Verify that a SIGSEGV is generated.
 *	Now change the protection to WRITE using mprotect(2).
 *	Again try to write into the mapped region.
 *	Verify that no SIGSEGV is generated.
 *
 * USAGE:  <for command-line>
 *  mprotect02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	05/2002 changed over to use tst_sig instead of sigaction
 *
 * RESTRICTIONS
 *	None
 */

#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include "test.h"
#include "usctest.h"

#include "safe_macros.h"

static void cleanup(void);
static void setup(void);

char *TCID = "mprotect02";
int TST_TOTAL = 1;
int fd, status;
char file1[BUFSIZ];

char *addr = MAP_FAILED;
char buf[] = "abcdefghijklmnopqrstuvwxyz";

#ifndef UCLINUX

int main(int ac, char **av)
{
	int lc;
	char *msg;

	int bytes_to_write, fd, num_bytes;
	pid_t pid;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		fd = SAFE_OPEN(cleanup, file1, O_RDWR|O_CREAT, 0777);

		num_bytes = getpagesize();

		do {

			if (num_bytes > strlen(buf))
				bytes_to_write = strlen(buf);
			else
				bytes_to_write = num_bytes;

			num_bytes -= SAFE_WRITE(cleanup, 1, fd, buf, bytes_to_write);

		} while (0 < num_bytes);

		/* mmap the PAGESIZE bytes as read only. */
		addr = SAFE_MMAP(cleanup, 0, sizeof(buf), PROT_READ,
		    MAP_SHARED, fd, 0);

		if ((pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "fork #1 failed");

		if (pid == 0) {
			(void)memcpy(addr, buf, strlen(buf));
			printf("memcpy did not generate SIGSEGV\n");
			exit(1);
		}

		if (waitpid(pid, &status, 0) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid failed");

		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
			tst_resm(TPASS, "got SIGSEGV as expected");
		else
			tst_brkm(TBROK, cleanup,
			    "child exited abnormally; wait status = %d",
			    status);

		/* Change the protection to WRITE. */
		TEST(mprotect(addr, sizeof(buf), PROT_WRITE));

		if (TEST_RETURN != -1) {

			if (STD_FUNCTIONAL_TEST) {

				if ((pid = FORK_OR_VFORK()) == -1)
					tst_brkm(TBROK|TERRNO, cleanup,
						 "fork #2 failed");

				if (pid == 0) {
					(void)memcpy(addr, buf, strlen(buf));
					exit(0);
				}

				if (waitpid(pid, &status, 0) == -1)
					tst_brkm(TBROK|TERRNO, cleanup,
					    "waitpid failed");

				if (WIFEXITED(status) &&
				    WEXITSTATUS(status) == 0)
					tst_resm(TPASS, "didn't get SIGSEGV");
				else
					tst_brkm(TBROK, cleanup,
					    "child exited abnormally");

			} else
				tst_resm(TPASS, "call succeeded");

		} else {
			tst_resm(TFAIL|TERRNO, "mprotect failed");
			continue;
		}

		SAFE_MUNMAP(cleanup, addr, sizeof(buf));
		addr = MAP_FAILED;

		SAFE_CLOSE(cleanup, fd);

		SAFE_UNLINK(cleanup, file1);

	}

	cleanup();
	tst_exit();
}

#else

int main()
{
	tst_brkm(TCONF, NULL, "test not runnable on uClinux");
}

#endif /* UCLINUX */

static void setup()
{

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(file1, "mprotect02.tmp.%d", getpid());
}

static void cleanup()
{
	TEST_CLEANUP;

	if (addr != MAP_FAILED) {
		SAFE_MUNMAP(NULL, addr, sizeof(buf));
		SAFE_CLOSE(NULL, fd);
	}

	tst_rmdir();
}
