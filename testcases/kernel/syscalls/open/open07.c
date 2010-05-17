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
 *	open07.c
 *
 * DESCRIPTION
 *	Test the open(2) system call to ensure that it sets ELOOP correctly.
 *
 * CALLS
 *	open()
 *
 * ALGORITHM
 *	1. Create a symbolic link to a file, and call open(O_NOFOLLOW). Check
 *	   that it returns ELOOP.
 *
 *	2. Create a symbolic link to a directory, and call open(O_NOFOLLOW).
 *	   Check that it returns ELOOP.
 *
 *	3. Create a symbolic link to a symbolic link, and call open(O_NOFOLLOW).
 *	   Check that it returns ELOOP.
 *
 *	4. Create a symbolic link to a symbolically linked directory, and call
 *	   open(O_NOFOLLOW). Check that it returns ELOOP.
 *
 * 	5. Create a symbolic link to a directory, and call
 *         open("link/", O_NOFOLLOW). Check that it succeeds.
 *
 * USAGE:  <for command-line>
 *  open07 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 * RESTRICTIONS
 *	None
 */
#define _GNU_SOURCE		/* for O_NOFOLLOW */
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);
void setupfunc_test1();
void setupfunc_test2();
void setupfunc_test3();
void setupfunc_test4();
void setupfunc_test5();

char *TCID = "open07";
int TST_TOTAL = 5;
extern int Tst_count;
int fd1, fd2;

int exp_enos[] = { ELOOP, 0 };

struct test_case_t {
	char *desc;
	char filename[100];
	int flags;
	int mode;
	void (*setupfunc) ();
	int exp_errno;
} TC[] = {
	{
	"Test for ELOOP on f2: f1 -> f2", {}, O_NOFOLLOW, 00700,
		    setupfunc_test1, ELOOP}, {
	"Test for ELOOP on d2: d1 -> d2", {}, O_NOFOLLOW, 00700,
		    setupfunc_test2, ELOOP}, {
	"Test for ELOOP on f3: f1 -> f2 -> f3", {}, O_NOFOLLOW,
		    00700, setupfunc_test3, ELOOP}, {
	"Test for ELOOP on d3: d1 -> d2 -> d3", {}, O_NOFOLLOW,
		    00700, setupfunc_test4, ELOOP}, {
	"Test for success on d2: d1 -> d2", {}, O_NOFOLLOW, 00700,
		    setupfunc_test5, 0}, {
	NULL, {}, 0, 0, NULL, 0}
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* run the setup routines for the individual tests */
	for (i = 0; i < TST_TOTAL; i++) {
		if (TC[i].setupfunc != NULL) {
			TC[i].setupfunc();
		}
	}

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (i = 0; TC[i].desc != NULL; ++i) {

			TEST(open(TC[i].filename, TC[i].flags, TC[i].mode));

			if (TC[i].exp_errno != 0) {
				if (TEST_RETURN != -1) {
					tst_resm(TFAIL, "open succeeded "
						 "unexpectedly");
				}
				TEST_ERROR_LOG(TEST_ERRNO);

				if (TEST_ERRNO != TC[i].exp_errno) {
					tst_resm(TFAIL, "open returned "
						 "unexpected errno, expected: "
						 "%d, got: %d",
						 TC[i].exp_errno, TEST_ERRNO);
				} else {
					tst_resm(TPASS, "open returned "
						 "expected ELOOP error");
				}
			} else {
				if (TEST_RETURN == -1) {
					tst_resm(TFAIL, "open failed "
						 "unexpectedly with errno %d",
						 TEST_ERRNO);
				} else {
					tst_resm(TPASS, "open succeeded as "
						 "expected");
				}
 			}
			if (TEST_RETURN != -1)
				close(TEST_RETURN);
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;
}

void setupfunc_test1()
{
	char file1[100], file2[100];

	sprintf(file1, "open03.1.%d", getpid());
	sprintf(file2, "open03.2.%d", getpid());
	if ((fd1 = creat(file1, 00700)) < 0) {
		tst_brkm(TBROK, cleanup, "creat(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	if (symlink(file1, file2) < 0) {
		tst_brkm(TBROK, cleanup, "symlink(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	strcpy(TC[0].filename, file2);
}

void setupfunc_test2()
{
	char file1[100], file2[100];

	sprintf(file1, "open03.3.%d", getpid());
	sprintf(file2, "open03.4.%d", getpid());
	if (mkdir(file1, 00700) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	if (symlink(file1, file2) < 0) {
		tst_brkm(TBROK, cleanup, "symlink(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	strcpy(TC[1].filename, file2);
}

void setupfunc_test3()
{
	char file1[100], file2[100], file3[100];

	sprintf(file1, "open03.5.%d", getpid());
	sprintf(file2, "open03.6.%d", getpid());
	sprintf(file3, "open03.7.%d", getpid());
	if ((fd2 = creat(file1, 00700)) < 0) {
		tst_brkm(TBROK, cleanup, "creat(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	if (symlink(file1, file2) < 0) {
		tst_brkm(TBROK, cleanup, "symlink(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	if (symlink(file2, file3) < 0) {
		tst_brkm(TBROK, cleanup, "symlink(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	strcpy(TC[2].filename, file3);
}

void setupfunc_test4()
{
	char file1[100], file2[100], file3[100];

	sprintf(file1, "open03.8.%d", getpid());
	sprintf(file2, "open03.9.%d", getpid());
	sprintf(file3, "open03.10.%d", getpid());
	if (mkdir(file1, 00700) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	if (symlink(file1, file2) < 0) {
		tst_brkm(TBROK, cleanup, "symlink(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	if (symlink(file2, file3) < 0) {
		tst_brkm(TBROK, cleanup, "symlink(2) failed: errno: %d", errno);
	 /*NOTREACHED*/}
	strcpy(TC[3].filename, file3);
}

void setupfunc_test5()
{
	char file1[100], file2[100];

	sprintf(file1, "open11.3.%d", getpid());
	sprintf(file2, "open12.4.%d", getpid());
	if (mkdir(file1, 00700) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(2) failed: errno: %d", errno);
	/*NOTREACHED*/}
	if (symlink(file1, file2) < 0) {
		tst_brkm(TBROK, cleanup, "symlink(2) failed: errno: %d", errno);
	/*NOTREACHED*/}
	strcpy(TC[4].filename, file2);
	strcat(TC[4].filename, "/");
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	umask(0);

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;
	close(fd1);
	close(fd2);

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
