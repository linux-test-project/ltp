/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *      05/2002 changed over to use tst_sig instead of sigaction
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>
#include "test.h"

#include "safe_macros.h"

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif
#define FAILED 1

static void cleanup(void);
static void setup(void);

char *TCID = "mprotect03";
int TST_TOTAL = 1;
int status;
char file1[BUFSIZ];

int main(int ac, char **av)
{
	int lc;

	char *addr;
	int fd, pid;
	char *buf = "abcdefghijklmnopqrstuvwxyz";

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if ((fd = open(file1, O_RDWR | O_CREAT, 0777)) < 0)
			tst_brkm(TBROK, cleanup, "open failed");

		SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd, buf, strlen(buf));

		/*
		 * mmap the PAGESIZE bytes as read only.
		 */
		addr = mmap(0, strlen(buf), PROT_READ | PROT_WRITE, MAP_SHARED,
			    fd, 0);
		if (addr == MAP_FAILED)
			tst_brkm(TBROK, cleanup, "mmap failed");

		/*
		 * Try to change the protection to WRITE.
		 */
		TEST(mprotect(addr, strlen(buf), PROT_READ));

		if (TEST_RETURN != -1) {
			if ((pid = tst_fork()) == -1) {
				tst_brkm(TBROK, cleanup, "fork failed");
			}

			if (pid == 0) {
				memcpy(addr, buf, strlen(buf));
				tst_resm(TINFO, "memcpy() did "
					 "not generate SIGSEGV");
				exit(1);
			}

			waitpid(pid, &status, 0);
			if (WEXITSTATUS(status) != 0) {
				tst_resm(TFAIL, "child returned "
					 "unexpected status");
			} else {
				tst_resm(TPASS, "SIGSEGV generated "
					 "as expected");
			}
		} else {
			tst_resm(TFAIL, "mprotect failed "
				 "unexpectedly, errno: %d", errno);
		}

		/* clean up things in case we are looping */
		SAFE_MUNMAP(cleanup, addr, strlen(buf));
		SAFE_CLOSE(cleanup, fd);
		SAFE_UNLINK(cleanup, file1);
	}

	cleanup();
	tst_exit();
}

static void sighandler(int sig)
{
	if (sig == SIGSEGV) {
		tst_resm(TINFO, "received signal: SIGSEGV");
		tst_exit();
	} else
		tst_brkm(TBROK, 0, "Unexpected signal %d received.", sig);
}

static void setup(void)
{
	tst_sig(FORK, sighandler, NULL);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(file1, "mprotect03.tmp.%d", getpid());
}

static void cleanup(void)
{
	tst_rmdir();
}
