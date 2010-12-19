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
 *	execve02.c
 *
 * DESCRIPTION
 *	Testcase to check whether execve(2) sets errno to EACCES correctly
 *
 * ALGORITHM
 *	1.	Attempt to execve(2) an executable owned by root with
 *		no execute permissions for the other users, fails when
 *		execve(2) is used as a non-root user (ltpuser1). The errno
 *		should be EACCES.
 *
 * USAGE:  <for command-line>
 *  execve02 -F <test file> [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 *      21/04/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Fix concurrency issue. In case of concurrent executions, all tasks
 *        was using the same file, changing its mode and leading to invalid
 *        mode for some of them.
 *
 * RESTRICTIONS
 *	Must run test with the -F <test file> option.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <libgen.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

char *TCID = "execve02";
int TST_TOTAL = 1;

void setup(char *);
void cleanup(void);
void help(void);

int exp_enos[] = { EACCES, 0 };

int Fflag = 0;
char *fname;

/* for test specific parse_opts options - in this case "-F" */
option_t options[] = {
	{"F:", &Fflag, &fname},
	{NULL, NULL, NULL}
};

char user1name[] = "nobody";
extern struct passwd *my_getpwnam(char *);
struct passwd *ltpuser1;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int status, retval = 3;
	pid_t pid;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (!Fflag)
		tst_brkm(TWARN, NULL, "You must specify a test executable with"
			 "the -F option.");

	setup(av[0]);

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if (chmod(fname, 0700) != 0)
			tst_resm(TFAIL|TERRNO,
			    "Failed to change permissions of test file");

		if ((pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "fork() failed");

		if (pid == 0) {
			if (seteuid(ltpuser1->pw_uid) == -1)
				tst_brkm(TBROK|TERRNO, cleanup,
				    "setuid(%d) failed", ltpuser1->pw_uid);
			char *argv[0];

			argv[0] = basename(fname);
			if (argv[0] == NULL)
				tst_brkm(TBROK|TERRNO, cleanup,
				    "basename failed");
			TEST(execve(fname, argv, NULL));

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO != EACCES) {
				retval = 1;
				perror("execve failed unexpectedly");
			} else
				printf("execve failed with EACCES as expected\n");

			/* change back to root */
			if (seteuid(0) == -1)
				perror("setuid(0) failed");

			/* reset the file permissions */
			if (chmod(fname, 0755) == -1)
				tst_brkm(TBROK, cleanup, "chmod() #2 failed");
			exit(retval);
		} else {
			/* wait for the child to finish */
			if (wait(&status) == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "wait failed");
			/* make sure the child returned a good exit status */
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				tst_resm(TFAIL, "child exited abnormally");

		}
	}
	cleanup();

	tst_exit();
}

/*
 * help() - Prints out the help message for the -F option defined
 *          by this test.
 */
void help()
{
	printf("  -F <test file> : for example, 'execve02 -F test3'\n");
}

void setup(char *argv0)
{
	char *cmd, *path, *pwd;
	int res;

	path = NULL;
	pwd = NULL;

	tst_require_root(NULL);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	FIXME: (find reference to other code that does similar to solve this problem and squish it into libltp or something.

	cmd = malloc(strlen(path) + strlen(bname) + 15);
	if (cmd == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "Cannot alloc command string");

	sprintf(cmd, "cp -p %s/%s .", path, bname);
	res = system(cmd);
	if (res != 0)
		tst_brkm(TBROK, NULL, "system(%s) failed", cmd);
	free(cmd);

	fname = bname;

	umask(0);

	ltpuser1 = my_getpwnam(user1name);
}

void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();
}