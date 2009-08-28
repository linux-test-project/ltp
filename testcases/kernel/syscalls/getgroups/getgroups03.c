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
 * Test Name: getgroups03
 *
 * Test Description:
 *  Verify that, getgroups() system call gets the supplementary group IDs
 *  of the calling process.
 *
 * Expected Result:
 *  The call succeeds in getting all the supplementary group IDs of the
 *  calling process. The effective group ID may or may not be returned.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  getgroups01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define TESTUSER	"root"
#define PRESENT		1
#define NOT_PRESENT	0

char *TCID = "getgroups03";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test conditions */
int ngroups;			/* No. of groups */
extern int Tst_count;		/* Test Case counter for tst_* routines */
gid_t groups_list[NGROUPS];	/* Array to hold gids for getgroups() */
gid_t groups[NGROUPS];		/* Array to hold gids read fr. /etc/group */
int fflag;			/* functionality flag variable */

int verify_groups(int);		/* function to verify groups returned */
int readgroups(gid_t *);	/* function to read gids of testuser */
void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int ret_val;		/* getgroups(2) return value */
	int gidsetsize = NGROUPS;	/* total groups */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call getgroups() to get supplimentary group IDs of
		 * TESTUSER ("root").
		 */
		TEST(getgroups(gidsetsize, groups_list));

		/* check return code of getgroups(2) */
		if ((ret_val = TEST_RETURN) == -1) {
			tst_resm(TFAIL|TTERRNO, "getgroups(%d, groups_list) failed", gidsetsize);
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Call verify_groups() to verify the groups
			 * returned by getgroups(2) match with the
			 * expected groups.
			 */
			fflag = verify_groups(ret_val);
			if (fflag) {
				tst_resm(TPASS, "Functionality of "
					 "getgroups(%d, groups_list) "
					 "successful", gidsetsize);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Get the supplimentary gid(s) of root from /etc/group.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/*
	 * Get the IDs of all the groups of "root"
	 * from /etc/group file
	 */
	ngroups = readgroups(groups);

	/* Setgroups is called by the login(1) process
	 * if the testcase is executed via an ssh session this
	 * testcase will fail. So execute setgroups() before executing
	 * getgroups()
	 */
	if (setgroups(ngroups, groups) == -1) {
		tst_resm(TFAIL|TERRNO, "setgroups() failed");
		cleanup();
	}

}				/* End setup() */

/*
 * readgroups(gid_t *)  - Read supplimentary group ids of "root" user
 * Scans the /etc/group file to get IDs of all the groups to which TESTUSER
 * belongs and puts them into the array passed.
 * Returns the no of gids read.
 */
int readgroups(gid_t groups[NGROUPS])
{
	struct group *grp;	/* To hold the group entry */
	int ngrps = 0;		/* No of groups */
	int i;
	int found;
	gid_t g;

	setgrent();

	while ((grp = getgrent()) != 0) {
		for (i = 0; grp->gr_mem[i]; i++) {
			if (strcmp(grp->gr_mem[i], TESTUSER) == 0) {
				groups[ngrps++] = grp->gr_gid;
			}
		}
	}

	/* The getgroups specification says:
	   It is unspecified whether the effective group ID of the
	   calling process is included in the returned list.  (Thus,
	   an application should also call getegid(2) and add or
	   remove the resulting value.).  So, add the value here if
	   it's not in.  */

	found = 0;
	g = getegid();

	for (i = 0; i < ngrps; i++) {
		if (groups[i] == g)
			found = 1;
	}
	if (found == 0)
		groups[ngrps++] = g;

	endgrent();
	return (ngrps);
}

/*
 * verify_groups(int)  - Verify supplimentary group id values.
 *   This function verifies the gid values returned by getgroups() with
 *   the read values from /etc/group file.
 *  This function returns flag value which indicates success or failure
 *  of verification.
 */
int verify_groups(ret_val)
int ret_val;
{
	int i, j;		/* counter variables */
	gid_t egid;		/* Effective gid of the process */
	int egid_flag = PRESENT;	/* egid present or not */

	/* Set the functionality flag */
	fflag = 1;

	/*
	 * Loop through the array to verify the gids
	 * returned by getgroups().
	 * First, compare each element of the array
	 * returned by getgroups() with that read from
	 * group file.
	 */
	for (i = 0; i < ret_val; i++) {
		for (j = 0; j < ngroups; j++) {
			if (groups_list[i] != groups[j]) {
				/* If loop ends and gids are not matching */
				if (j == ngroups - 1) {
					tst_resm(TFAIL, "getgroups() returned "
						 "incorrect gid %d",
						 groups_list[i]);
					fflag = 0;
				} else {
					continue;
				}
			} else {	/* if equal, continue the outer loop */
				break;
			}
		}
	}

	/* Now do the reverse comparison */
	egid = getegid();	/* get egid */
	for (i = 0; i < ngroups; i++) {
		for (j = 0; j < ret_val; j++) {
			if (groups[i] != groups_list[j]) {
				/*
				 * If the loop ends & gids are not matching
				 * if gid is not egid, exit with error
				 * else egid is returned by getgroups()
				 */
				if (j == (ret_val - 1)) {
					if (groups[i] != egid) {
						tst_resm(TFAIL, "getgroups(2) "
							 "didn't return %d one "
							 "of the gids of %s",
							 groups[i], TESTUSER);
						fflag = 0;
					} else {
						/*
						 * egid is not present in
						 * group_list.
						 * Reset the egid flag
						 */
						egid_flag = NOT_PRESENT;
					}
				} else {	/* if the loop is not over */
					continue;
				}
			} else {	/* if equal, continue the outer loop */
				break;
			}
		}
	}

	/*
	 * getgroups() should return the no. of gids for TESTUSER with
	 * or without egid taken into account.
	 * Decrement ngroups, if egid is not returned by getgroups()
	 * Now, if ngroups matches ret_val, as above comparisons of the array
	 * are successful, this implies that the array contents match.
	 */
	if (egid_flag == 0) {	/* If egid is not returned */
		ngroups--;
	}
	if (ngroups != ret_val) {
		tst_resm(TFAIL,
			 "getgroups(2) returned incorrect no. of gids %d (expect %d)",
			 ret_val, ngroups);
		fflag = 0;
	}

	/* Return the status of functionality */
	return (fflag);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
