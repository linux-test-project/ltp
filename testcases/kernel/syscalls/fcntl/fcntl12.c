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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	fcntl12.c
 *
 * DESCRIPTION
 *	Testcase to test that fcntl() sets EMFILE for F_DUPFD command.
 *
 * ALGORITHM
 *	Get the size of the descriptor table of a process, by calling the
 *	getdtablesize() system call. Then attempt to use the F_DUPFD command
 *	for fcntl(), which should fail with EMFILE.
 *
 * USAGE
 *	fcntl12
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "test.h"

char *TCID = "fcntl12";
int TST_TOTAL = 1;

int fail;
char fname[20];
void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	pid_t pid;
	int fd, i, status, max_files;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		tst_resm(TINFO, "Test for errno EMFILE");
		fail = 0;

		pid = FORK_OR_VFORK();
		if (pid < 0) {
			tst_brkm(TBROK | TERRNO, cleanup, "Fork failed");
		} else if (pid == 0) {
			max_files = getdtablesize();
			for (i = 0; i < max_files; i++) {
				if ((fd = open(fname, O_CREAT | O_RDONLY,
					       0444)) == -1) {
					break;
				}
			}

			if (fcntl(1, F_DUPFD, 1) != -1) {
				tst_resm(TFAIL, "fcntl failed to FAIL");
				exit(1);
			} else if (errno != EMFILE) {
				tst_resm(TFAIL, "Expected EMFILE got %d",
					 errno);
				exit(1);
			}
			exit(0);
		}
		waitpid(pid, &status, 0);
		if (WEXITSTATUS(status) == 0)
			tst_resm(TPASS, "block 1 PASSED");
		else
			tst_resm(TFAIL, "block 1 FAILED");
	}
	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	sprintf(fname, "fcnlt12.%d", getpid());
	tst_tmpdir();
}

void cleanup(void)
{
	unlink(fname);
	tst_rmdir();
}
