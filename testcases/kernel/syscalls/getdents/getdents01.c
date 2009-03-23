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
 *	getdents01.c
 *
 * DESCRIPTION
 *	getdents01 - get a directory entry
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the errno value
 *	  if failure, log the errno and issue a FAIL message
 *	otherwise,
 *	  if doing functionality testing,
 *	    if the directory name of the first entry is "."
 *	      issue a PASS message
 *	    otherwise,
 *	      issue a FAIL message
 *	      break any remaining tests
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  getdents01 [-c n] [-f] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	Any restrictions
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

char *TCID = "getdents01";
int TST_TOTAL = 1;
extern int Tst_count;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int rval, fd;
	int count;
	size_t size = 0;
	char *dir_name = NULL;
	struct dirent *dirp;

	/*
	 * Here's a case where invoking the system call directly
	 * doesn't seem to work.  getdents.h has an assembly
	 * macro to do the job.
	 *
	 * equivalent to  - getdents(fd, dirp, count);
	 * if we could call getdents that way.
	 */

#define getdents(arg1, arg2, arg3) syscall(__NR_getdents, arg1, arg2, arg3)

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

		/*
		 * Set up count to be equal to the sizeof struct dirent,
		 * just to pick a decent size.
		 */

		count = (int)sizeof(struct dirent);

		/* open the directory and get a file descriptor */

		if ((fd = open(dir_name, O_RDONLY)) == -1) {
			tst_brkm(TBROK, cleanup, "open of directory failed");
		}

		rval = getdents(fd, dirp, count);
		if (rval < 0) {	/* call returned an error */

			rval *= -1;
			TEST_ERROR_LOG(rval);

			tst_resm(TFAIL, "%s call failed - errno = %d "
				 ": %s", TCID, rval, strerror(rval));
			continue;
		}

		if (rval == 0) {	/* call returned end of directory */
			tst_resm(TFAIL, "%s call failed - returned "
				 "end of directory", TCID);
			continue;
		}

		/* Removed this b/c there isn't any documentation on its validity
		 */
		//      if (STD_FUNCTIONAL_TEST) {
		//              /*
		//               * Now we have dirp pointing to the "base" dirent
		//               * structure for the directory that we opened.
		//               */
		//
		//              /*
		//               * The first dirent structure returned should point
		//               * to the current directory, AKA "."
		//               */
		//
		//              if (strcmp(".", dirp->d_name) == 0) {
		//                      tst_resm(TPASS, "%s call succeeded", TCID);
		//              } else {
		//                      tst_resm(TFAIL, "%s call failed - "
		//                               "unexpected directory name: %s", TCID, dirp->d_name);
		//              }
		//      } else {
		tst_resm(TPASS, "call succeeded");
		//      }

		/*
		 * clean up things in case we are looping
		 */
		free(dir_name);
		dir_name = NULL;

		free(dirp);

		if ((rval = close(fd)) == -1) {
			tst_brkm(TBROK, cleanup, "file close failed");
		}
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
