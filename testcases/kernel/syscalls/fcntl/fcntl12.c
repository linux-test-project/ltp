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
#include <test.h>
#include <usctest.h>

char *TCID = "fcntl12";
int TST_TOTAL = 1;
extern int Tst_count;

int fail;
char fname[20];
void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	pid_t pid;
	int fd, i, status, max_files;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

/* //block1: */
		tst_resm(TINFO, "Enter block 1");
		tst_resm(TINFO, "Test for errno EMFILE");
		fail = 0;

		pid = FORK_OR_VFORK();
		if (pid < 0) {
			tst_resm(TFAIL, "Fork failed");
			cleanup();
		 /*NOTREACHED*/} else if (pid == 0) {	/* child */
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
		if (WEXITSTATUS(status) == 0) {
			tst_resm(TINFO, "block 1 PASSED");
		} else {
			tst_resm(TINFO, "block 1 FAILED");
		}
		tst_resm(TINFO, "Exit block 1");
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

	/* Pause if that option was specified */
	TEST_PAUSE;

	sprintf(fname, "fcnlt12.%d", getpid());
	tst_tmpdir();
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

	unlink(fname);
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
