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
 *	chroot03.c
 *
 * DESCRIPTION
 *	Testcase to test whether chroot(2) sets errno correctly.
 *
 * CALLS
 *	chroot(2)
 *
 * ALGORITHM
 *	1.	Test for ENAMETOOLONG:
 *		Create a bad directory name with length more than
 *		VFS_MAXNAMELEN (Linux kernel variable), and pass it as the
 *		path to chroot(2).
 *
 *	2.	Test for ENOENT:
 *		Attempt to chroot(2) on a non-existent directory
 *
 *	3.	Test for ENOTDIR:
 *		Attempt to chdir(2) on a file.
 *
 *	4.	Test for EFAULT:
 *		The pathname parameter to chroot() points to an invalid address,
 *		chroot(2) fails with EPERM.
 *
 * USAGE:  <for command-line>
 *  chroot03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "test.h"
#include "usctest.h"
#include <fcntl.h>

char *TCID = "chroot03";

int fd = 0;
char fname[255];
char good_dir[100] = "/tmp/testdir";
char bad_dir[] =
    "abcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";

int exp_enos[] = { ENAMETOOLONG, ENOENT, ENOTDIR, EFAULT, 0 };

struct test_case_t {
	char *dir;
	int error;
} TC[] = {
	/*
	 * to test whether chroot() is setting ENAMETOOLONG if the
	 * pathname is more than VFS_MAXNAMELEN
	 */
	{
	bad_dir, ENAMETOOLONG},
	    /*
	     * to test whether chroot() is setting ENOTDIR if the argument
	     * is not a directory.
	     */
	{
	fname, ENOTDIR},
	    /*
	     * to test whether chroot() is setting ENOENT if the directory
	     * does not exist.
	     */
	{
	good_dir, ENOENT},
#if !defined(UCLINUX)
	    /*
	     * attempt to chroot to a path pointing to an invalid address
	     * and expect EFAULT as errno
	     */
	{
	(char *)-1, EFAULT}
#endif
};

int TST_TOTAL = (sizeof(TC) / sizeof(*TC));

char *bad_addr = 0;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	int i;
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(chroot(TC[i].dir));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS|TTERRNO, "failed as expected");
			} else {
				tst_resm(TFAIL|TTERRNO,
				    "didn't fail as expected (expected errno "
				    "= %d : %s)",
				    TC[i].error, strerror(TC[i].error));
			}
		}
	}
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	/*
	 * create a file and use it to test whether chroot() is setting
	 * ENOTDIR if the argument is not a directory.
	 */
	(void)sprintf(fname, "tfile_%d", getpid());
	if ((fd = creat(fname, 0777)) == -1) {
		tst_brkm(TBROK, cleanup, "Failed to creat a temp file");
	}

	/*
	 * set up good_dir to test whether chroot() is setting ENOENT if the
	 * directory does not exist.
	 */
	(void)sprintf(good_dir, "%s.%d", good_dir, getpid());

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	TC[3].dir = bad_addr;
#endif
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
	close(fd);

	TEST_CLEANUP;

	/* delete the test directory created in setup() */
	tst_rmdir();

}