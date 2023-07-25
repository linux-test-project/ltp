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
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	05/2002 changed over to use tst_sig instead of sigaction
 */

#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include "test.h"

#include "safe_macros.h"

static void sighandler(int sig);
static void cleanup(void);
static void setup(void);

char *TCID = "mprotect02";
int TST_TOTAL = 1;
static int fd, status;
static char file1[BUFSIZ];

static char *addr = MAP_FAILED;
static char buf[] = "abcdefghijklmnopqrstuvwxyz";

int main(int ac, char **av)
{
	int lc;

	int bytes_to_write, fd;
	size_t num_bytes;
	pid_t pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		fd = SAFE_OPEN(cleanup, file1, O_RDWR | O_CREAT, 0777);

		num_bytes = getpagesize();

		do {

			bytes_to_write = MIN(strlen(buf), num_bytes);

			num_bytes -=
			    SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd, buf,
				bytes_to_write);

		} while (0 < num_bytes);

		/* mmap the PAGESIZE bytes as read only. */
		addr = SAFE_MMAP(cleanup, 0, sizeof(buf), PROT_READ,
				 MAP_SHARED, fd, 0);

		if ((pid = tst_fork()) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "fork #1 failed");

		if (pid == 0) {
			memcpy(addr, buf, strlen(buf));
			exit(255);
		}

		SAFE_WAITPID(cleanup, pid, &status, 0);
		if (!WIFEXITED(status))
			tst_brkm(TBROK, cleanup, "child exited abnormally "
				 "with status: %d", status);
		switch (status) {
		case 255:
			tst_brkm(TBROK, cleanup,
				 "memcpy did not generate SIGSEGV");
		case 0:
			tst_resm(TPASS, "got SIGSEGV as expected");
			break;
		default:
			tst_brkm(TBROK, cleanup, "got unexpected signal: %d",
				 status);
			break;
		}

		/* Change the protection to WRITE. */
		TEST(mprotect(addr, sizeof(buf), PROT_WRITE));

		if (TEST_RETURN != -1) {
			if ((pid = tst_fork()) == -1)
				tst_brkm(TBROK | TERRNO, cleanup,
					 "fork #2 failed");

			if (pid == 0) {
				memcpy(addr, buf, strlen(buf));
				exit(0);
			}

			SAFE_WAITPID(cleanup, pid, &status, 0);

			if (WIFEXITED(status) &&
			    WEXITSTATUS(status) == 0)
				tst_resm(TPASS, "didn't get SIGSEGV");
			else
				tst_brkm(TBROK, cleanup,
					 "child exited abnormally");
		} else {
			tst_resm(TFAIL | TERRNO, "mprotect failed");
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

static void sighandler(int sig)
{
	_exit((sig == SIGSEGV) ? 0 : sig);
}

static void setup(void)
{
	tst_sig(FORK, sighandler, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(file1, "mprotect02.tmp.%d", getpid());
}

static void cleanup(void)
{
	if (addr != MAP_FAILED) {
		SAFE_MUNMAP(NULL, addr, sizeof(buf));
		SAFE_CLOSE(NULL, fd);
	}

	tst_rmdir();
}
