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
 *	statfs02.c
 *
 * DESCRIPTION
 *	Testcase to check that statfs(2) sets errno correctly.
 *
 * ALGORITHM
 *	1.	Use a component of the pathname, which is not a directory
 *		in the "path" parameter to statfs(). Expect ENOTDIR
 *	2.	Pass a filename which doesn't exist, and expect ENOENT.
 *	3.	Pass a pathname which is more than MAXNAMLEN, and expect
 *		ENAMETOOLONG.
 *	4.	Pass a pointer to the pathname outside the address space of
 *		the process, and expect EFAULT.
 *	5.	Pass a pointer to the buf paramter outside the address space
 *		of the process, and expect EFAULT.
 *
 * USAGE:  <for command-line>
 *  statfs02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	NONE
 *
 */
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <sys/mman.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

char *TCID = "statfs02";
int fileHandle = 0;
extern int Tst_count;

int exp_enos[] = {
	ENOTDIR, ENOENT, ENAMETOOLONG,
#if !defined(UCLINUX)
	EFAULT, 0
#endif
};

char *bad_addr = 0;

char bad_file[] =
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";
char good_dir[100] = "testdir";
char fname[30] = "testfile";
struct statfs fsbuf;
char fname1[30];

struct test_case_t {
	char *path;
	struct statfs *buf;
	int error;
} TC[] = {
	/* path has a component which is not a directory - ENOTDIR */
	{
	fname1, &fsbuf, ENOTDIR},
	    /* path does not exist - ENOENT */
	{
	good_dir, &fsbuf, ENOENT},
	    /* path is too long - ENAMETOOLONG */
	{
	bad_file, &fsbuf, ENAMETOOLONG},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	    /* path is an invalid address - EFAULT */
	{
	(char *)-1, &fsbuf, EFAULT},
	    /* buf is an invalid address - EFAULT */
	{
	fname, (struct statfs *)-1, EFAULT}
#endif
};

int TST_TOTAL = sizeof(TC) / sizeof(*TC);

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(statfs(TC[i].path, TC[i].buf));

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
	cleanup();

	 /*NOTREACHED*/ return 0;

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	sprintf(fname, "%s.%d", fname, getpid());
	if ((fileHandle = creat(fname, 0444)) == -1) {
		tst_resm(TFAIL, "creat(2) FAILED to creat temp file");
	}
	sprintf(fname1, "%s/%s", fname, fname);

	sprintf(good_dir, "%s.statfs.%d", good_dir, getpid());

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	TC[3].path = bad_addr;
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
	close(fileHandle);

	TEST_CLEANUP;

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
