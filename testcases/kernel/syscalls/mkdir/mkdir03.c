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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1303 USA
 */

/*
 * NAME
 *	mkdir03
 *
 * DESCRIPTION
 *	check mkdir() with various error conditions that should produce
 *	EFAULT, ENAMETOOLONG, EEXIST, ENOENT and ENOTDIR
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Pause for SIGUSR1 if option specified.
 *		Create temporary directory.
 *
 *	Test:
 *		Loop if the proper options are given.
 *              Loop through the test cases
 *                 call the test case specific setup routine if necessary
 *                 call mkdir() using the TEST macro
 *		   if the call succeeds
 *		      print a FAIL message and continue
 *		   Log the errno value
 *		   if the errno is expected
 *			issue a PASS message
 *		   else
 *			issue a FAIL message
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 * USAGE
 *	mkdir03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None.
 *
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
void setup1();
void setup2();
void setup3();
void setup4();
void setup5();

#define PERMS		0777
#define PERMS2		0277

#define NAMELEN		50

char *TCID = "mkdir03";		/* Test program identifier.    */
int fileHandle, fileHandle2 = 0;

char tstdir3[NAMELEN];
char tstdir4[NAMELEN];
char tstdir5[NAMELEN];

char long_dir[] =
    "abcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";

int exp_enos[] = { EFAULT, ENAMETOOLONG, EEXIST, ENOENT, ENOTDIR, 0 };

char *bad_addr = 0;

struct test_case_t {
	char *dir;
	int perms;
	int error;
	void (*setupfunc) ();
} TC[] = {
#if !defined(UCLINUX)
	/* try to create a directory with an illegal name/address */
	{
	(void *)-1, PERMS, EFAULT, NULL},
#endif
	    /* try to create a directory using a name that is too long */
	{
	long_dir, PERMS2, ENAMETOOLONG, NULL},
	    /* try to create a directory with the same name as an existing file */
	{
	tstdir3, PERMS, EEXIST, setup3},
	    /* try to create a directory under a directory that doesn't exist */
	{
	tstdir4, PERMS, ENOENT, setup4},
	    /*
	     * try to create a directory under a path with a non-directory
	     * component
	     */
	{
	tstdir5, PERMS, ENOTDIR, setup5}
};

int TST_TOTAL = sizeof(TC) / sizeof(TC[0]);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	/*
	 * perform global setup for test
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			/* perform test specific setup if necessary */
			if (TC[i].setupfunc != NULL) {
				(*TC[i].setupfunc) ();
			}

			TEST(mkdir(TC[i].dir, TC[i].perms));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
			}
		}
	}

	/*
	 * cleanup and exit
	 */
	cleanup();

	tst_exit();
}

/*
 * setup3() - performs all ONE TIME setup for this test case 3.
 */
void setup3()
{
	char tstfile3[NAMELEN];

	/* Initialize the test directory name and file name */
	sprintf(tstfile3, "tst.%d", getpid());
	sprintf(tstdir3, "%s", tstfile3);

	/* create a file */
	if ((fileHandle = creat(tstfile3, PERMS)) == -1) {
		tst_brkm(TBROK, cleanup, "file creation failed is setup3");
	}
}

/*
 * setup4() - performs all ONE TIME setup for this test case 4.
 */
void setup4()
{
	char tstdir[NAMELEN];
	struct stat statbuf;

	/* Initialize the test directory name */
	sprintf(tstdir, "tstdir4.%d", getpid());
	sprintf(tstdir4, "%s/tst", tstdir);
/*
	sprintf(tstdir4, "%s/tst", tstdir4);

  This fails with EACCES           ^^^^^^^
  add this as testcase?

*/

	/* make sure tstdir4 does not exist */
	if (stat(tstdir4, &statbuf) != -1) {
		tst_brkm(TBROK, cleanup, "directory exists - test #4");
	}

}

/*
 * setup5() - performs all ONE TIME setup for this test case 5.
 */
void setup5()
{
	char tstfile5[NAMELEN];

	/* Initialize the test directories name and file name */
	sprintf(tstfile5, "tstfile5.%d", getpid());
	sprintf(tstdir5, "%s/tst", tstfile5);

	/* create a file */
	if ((fileHandle2 = creat(tstfile5, PERMS)) == -1) {
		tst_brkm(TBROK, cleanup, "creat a file failed");
	 }
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	TC[0].dir = bad_addr;
#endif
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	close(fileHandle);
	close(fileHandle2);

	TEST_CLEANUP;

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

	/*
	 * Exit with return code appropriate for results.
	 */

}