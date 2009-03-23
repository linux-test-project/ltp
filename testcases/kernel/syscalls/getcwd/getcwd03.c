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
 *	getcwd03
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of the getcwd(2) system call
 *	for symbolically linked directories.
 *
 * ALGORITHM
 *	This testcase checks for the functionality of the getcwd(2) system call
 *	on a symbolic link. First create a directory (dir1), and create a
 *	symbolic link (dir2) to it at the same directory level. Then, chdir()
 *	to dir1, and get the working directory (cwd1), and its pathname (pwd1).
 *	Then, chdir() to dir2, and get the working directory (cwd2), its
 *	pathname (pwd2), and its readlink info (link2).
 *	Testcase succeeds if:
 *	i.	pwd1 == pwd2
 *	ii.	cwd1 == cwd2
 *	iii.	link2 == basename(cwd1)
 *
 * USAGE:  <for command-line>
 *  getcwd03 [-c n] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#define FAILED 1

int flag;
char *TCID = "getcwd03";
int TST_TOTAL = 1;
extern int Tst_count;

void cleanup(void);
void setup(void);
char *getpwd();

int main(int ac, char **av)
{
	char dir1[BUFSIZ], dir2[BUFSIZ];
	char cwd1[BUFSIZ], cwd2[BUFSIZ];
	char *pwd1, *pwd2;
	char link2[BUFSIZ];
	int n;
	int lc;			/* loop counter */
	char *msg;		/* parse_opts() return message */

	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/*
	 * The following loop checks looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		flag = 0;

		/*
		 * Create dir1, then chdir to dir1, and get the pwd,
		 * and cwd informations
		 */
		sprintf(dir1, "getcwd1.%d", getpid());
		if (mkdir(dir1, 00755) < 0) {
			tst_brkm(TBROK, cleanup, "mkdir(2) failed");
		 /*NOTREACHED*/}
		if (chdir(dir1) != 0) {
			tst_brkm(TBROK, cleanup, "chdir(2) failed");
		 /*NOTREACHED*/}

		pwd1 = getpwd();
		if (getcwd(cwd1, sizeof cwd1) == NULL) {
			tst_resm(TFAIL, "getcwd() failed unexpectedly: "
				 "errno = %d\n", errno);
			flag = FAILED;
		}
		if ((flag != FAILED) && (strcmp(pwd1, cwd1) != 0)) {
			tst_brkm(TFAIL, cleanup, "getcwd() returned unexpected "
				 "working directory: expected: %s, got: %s\n",
				 pwd1, cwd1);
		 /*NOTREACHED*/}

		tst_resm(TINFO, "getcwd(2) succeeded in returning correct path "
			 "for dir1");

		/*
		 * Now create dir2, then chdir to dir2, and get the pwd,
		 * cwd, and link informations
		 */
		chdir("..");
		flag = 0;

		sprintf(dir2, "getcwd2.%d", getpid());
		if (symlink(dir1, dir2) < 0) {
			tst_brkm(TBROK, cleanup, "symlink(2) failed: errno: %d",
				 errno);
		 /*NOTREACHED*/}

		if (chdir(dir2) != 0) {
			tst_brkm(TBROK, cleanup, "chdir(2) failed: errno: %d",
				 errno);
		 /*NOTREACHED*/}

		pwd2 = getpwd();
		if (getcwd(cwd2, sizeof cwd2) == NULL) {
			tst_resm(TFAIL, "getcwd() failed unexpectedly: "
				 "errno = %d\n", errno);
			flag = FAILED;
		}

		chdir("..");
		if ((flag != FAILED) &&
		    ((n = readlink(dir2, link2, sizeof(link2))) < 0)) {
			tst_brkm(TBROK, cleanup, "readlink(2) failed: errno:%d",
				 errno);
		 /*NOTREACHED*/}

		/*
		 * Finally compare the pwd, cwd, link informations:
		 * The test should pass iff all the following are true:
		 * a.   pwd1 == pwd2
		 * b.   cwd1 == cwd2
		 * c.   link2 == basename(cwd1)
		 */
		if (flag != FAILED) {
			if (strcmp(pwd1, pwd2) != 0) {
				tst_resm(TFAIL, "pwd1: %s, pwd2: %s",
					 pwd1, pwd2);
				flag = FAILED;
			}
			if (strcmp(cwd1, cwd2) != 0) {
				tst_resm(TFAIL, "cwd1: %s, cwd2: %s",
					 cwd1, cwd2);
				flag = FAILED;
			}
			if (memcmp(link2, (char *)basename(cwd1), n) != 0) {
				tst_resm(TFAIL, "link2: %s, cwd1: %s",
					 link2, cwd1);
				flag = FAILED;
			}
			if (flag != FAILED) {
				tst_resm(TINFO, "getcwd(2) succeeded in "
					 "returning correct path for symbolic "
					 "link dir2 -> dir1");
			}
		}

		if (flag == FAILED) {
			tst_resm(TFAIL, "Test FAILED");
		} else {
			tst_resm(TPASS, "Test PASSED");
		}

		/* clean up things in case we are looping */
		if (unlink(dir2) == -1) {
			tst_brkm(TBROK, cleanup, "couldnt remove dir2");
		}
		if (rmdir(dir1) == -1) {
			tst_brkm(TBROK, cleanup, "couldnt remove dir1");
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

void setup()
{
	/* capture signals */
	/* FORK is set here because of the popen() call below */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* create a test directory and cd into it */
	tst_tmpdir();
}

void cleanup()
{
	/* remove the test directory */
	tst_rmdir();

	/* print timing stats if that option was specified */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

char *getpwd()
{
	FILE *fin;
	char *pwd = "/bin/pwd";
	char *cp, *cp_cur;
	char *buf;

	buf = (char *)malloc(BUFSIZ);
	if ((fin = popen(pwd, "r")) == NULL) {
		tst_resm(TINFO, "%s: can't run %s", TCID, pwd);
		tst_brkm(TBROK, cleanup, "%s FAILED", TCID);
	 /*NOTREACHED*/}
	while (fgets(buf, BUFSIZ, fin) != NULL) {
		if ((cp = strchr(buf, '\n')) == (char *)NULL) {
			tst_brkm(TBROK, cleanup, "pwd output too long");
		 /*NOTREACHED*/}
		*cp = 0;
		cp_cur = buf;
	}
	pclose(fin);
	return buf;
}
