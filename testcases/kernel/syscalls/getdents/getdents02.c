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
 *	getdents02.c
 *
 * DESCRIPTION
 *	getdents02 - check that we get a failure with a bad file descriptor
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call using a bad file descriptor
 *	check the errno value
 *	  issue a PASS message if we get EBADF - errno 9
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  getdents02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "getdents.h"
#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/types.h>
#include <dirent.h>
#include <linux/unistd.h>
#include <unistd.h>

void cleanup(void);
void setup(void);

char *TCID = "getdents02";
int TST_TOTAL = 1;
extern int Tst_count;

int exp_enos[] = { EBADF, 0 };	/* 0 terminated list of expected errnos */

#ifndef __i386__
int main()
{
	tst_resm(TINFO, "This test includes x86 asm and will not work on "
		 "this machine");
	tst_exit();
	return 0;
}
#else

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int rval, fd;
	int count;
	const int cnum = 141;	/* system call number 141 = getdents */
	size_t size = 0;
	char *dir_name = NULL;
	struct dirent *dirp;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* get the current working directory */

		if ((dir_name = getcwd(dir_name, size)) == NULL) {
			tst_brkm(TBROK, cleanup, "Can not get current "
				 "directory name");
		}

		/* allocate some space for the dirent structure */

		if ((dirp =
		     (struct dirent *)malloc(sizeof(struct dirent))) == NULL) {
			tst_brkm(TBROK, cleanup, "malloc failed");
		}

		/* set up count to be equal to the sizeof struct dirent */

		count = (int)sizeof(struct dirent);

		/* set up a bad file descriptor */

		fd = -5;

		/*
		 * here's a case where invoking the system call directly
		 * doesn't seem to work.  getdents.h has an assembly
		 * macro to do the job.
		 *
		 * equivalent to  - getdents(fd, dirp, count);
		 * if we could call getdents that way.
		 */

		rval = GETDENTS_ASM();

		/*
		 * Hopefully we get an error due to the bad file descriptor.
		 */

		if (rval < 0) {	/* call returned an error */
			rval *= -1;
			TEST_ERROR_LOG(rval);

			switch (rval) {
			case EBADF:
				tst_resm(TPASS, "expected failure - errno = %d "
					 "- %s", rval, strerror(rval));
				break;
			default:
				tst_resm(TFAIL, "%s call failed to "
					 "produce EBADF - errno = %d : %s",
					 TCID, rval, strerror(rval));
				break;
			}
		} else {
			tst_resm(TFAIL, "call failed to produce an "
				 "expected error - errno = %d - %s", rval,
				 strerror(rval));
		}

		/*
		 * clean up things in case we are looping
		 */
		free(dir_name);
		dir_name = NULL;

		free(dirp);
	}

	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* create a test directory and cd into it */
	tst_tmpdir();

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void cleanup(void)
{
	/* remove the test directory */
	tst_rmdir();

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

#endif /* __i386__ */
