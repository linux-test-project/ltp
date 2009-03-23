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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

char *TCID = "execve02";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
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
	int e_code, status, retval = 3;
	pid_t pid;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	if (!Fflag) {
		tst_resm(TWARN, "You must specify a test executable with"
			 "the -F option.");
		tst_resm(TWARN, "Run '%s -h' for option information.", TCID);
		tst_exit();
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if (chmod(fname, 0700) != 0) {
			tst_resm(TFAIL, "Failed to change permissions of "
				 "test file");
		}

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (pid == 0) {	/* child */
			if (seteuid(ltpuser1->pw_uid) == -1) {
				tst_brkm(TBROK, cleanup, "setuid() failed");
			}

			TEST(execve(fname, NULL, NULL));

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO != EACCES) {
				retval = 1;
				tst_resm(TFAIL, "Expected EACCES got %d",
					 TEST_ERRNO);
			} else {
				tst_resm(TPASS, "Received EACCES");
			}

			/* change back to root */
			if (seteuid(0) == -1) {
				tst_brkm(TBROK, cleanup, "setuid(0) failed");
			}

			/* reset the file permissions */
			if (chmod(fname, 0755) == -1) {
				tst_brkm(TBROK, cleanup, "chmod() #2 failed");
			}
			exit(retval);
		} else {
			/* wait for the child to finish */
			wait(&status);
			/* make sure the child returned a good exit status */
			e_code = status >> 8;
			if ((e_code != 3) || (retval != 3)) {
				tst_resm(TFAIL, "Failures reported above");
			}

			cleanup();
		}
	}

	 /*NOTREACHED*/ return 0;
}

/*
 * help() - Prints out the help message for the -F option defined
 *          by this test.
 */
void help()
{
	printf("  -F <test file> : for example, 'execve02 -F test3'\n");
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	char *cmd, *dirc, *basec, *bname, *dname, *path, *pwd = NULL;
	int res;

	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Get file name of the passed test file and the absolute path to it.
	 * We will need these informations to copy the test file in the temp
	 * directory.
	 */
	dirc = strdup(fname);
	basec = strdup(fname);
	dname = dirname(dirc);
	bname = basename(basec);

	if (dname[0] == '/')
		path = dname;
	else {
		if ((pwd = getcwd(NULL, 0)) == NULL) {
			tst_brkm(TBROK, tst_exit,
				 "Could not get current directory");
		}
		path = malloc(strlen(pwd) + strlen(dname) + 2);
		if (path == NULL) {
			tst_brkm(TBROK, tst_exit, "Cannot alloc path string");
		}
		sprintf(path, "%s/%s", pwd, dname);
	}

	/* make a temp dir and cd to it */
	tst_tmpdir();

	/* Copy the given test file to the private temp directory.
	 */
	cmd = malloc(strlen(path) + strlen(bname) + 15);
	if (cmd == NULL) {
		tst_brkm(TBROK, tst_exit, "Cannot alloc command string");
	}

	sprintf(cmd, "cp -p %s/%s .", path, bname);
	res = system(cmd);
	free(cmd);
	if (res == -1) {
		tst_brkm(TBROK, tst_exit, "Cannot copy file %s", fname);
	}

	fname = bname;

	umask(0);

	ltpuser1 = my_getpwnam(user1name);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* remove files and temp dir */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
