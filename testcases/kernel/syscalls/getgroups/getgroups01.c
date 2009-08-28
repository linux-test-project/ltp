/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: getgroups01.c,v 1.7 2009/08/28 13:03:01 vapier Exp $ */
/***********************************************************************
TEST IDENTIFIER:  getgroups01 :	Getgroups system call critical test

PARENT DOCUMENT:  ggrtds01:  Getgroups system call test design spec

AUTHOR: Barrie Kletscher
	Rewrote :  11-92 by Richard Logan

CO-PILOT: Dave Baumgartner

TEST ITEMS:
	1. Check to see if getgroups(-1, gidset) fails and sets errno to EINVAL
	2. Check to see if getgroups(0, gidset) does not return -1 and gidset is
		not modified.
	3. Check to see if getgroups(x, gigset) fails and sets errno to EINVAL,
		where x is one less then what is returned by getgroups(0, gidset).
	4. Check to see if getgroups() succeeds and gidset contains
		group id returned from getgid().

INPUT SPECIFICATIONS:
	NONE

OUTPUT SPECIFICATIONS:
	Standard tst_res output format

ENVIRONMENTAL NEEDS:
	NONE.

SPECIAL PROCEDURAL REQUIREMENTS:
	None

INTERCASE DEPENDENCIES:
	Test case #3 depends on test case #2.

DETAILED DESCRIPTION:
	Set up the signal handling capabilities.
	execute tests
	exit

BUGS:
	None known.

************************************************************/

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <grp.h>
#include <sys/param.h>
#include <sys/types.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "getgroups01";	/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

gid_t gidset[NGROUPS];		/* storage for all group ids */
gid_t cmpset[NGROUPS];

/***********************************************************************
 * MAIN
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *ptr;		/* message returned from parse_opts */

	int i,			/* counter */
	 group,			/* return value from Getgid() call */
	 entries;		/* number of group entries */

	int ret;
	int ret2;
	int errors = 0;

	/* Initialize the group access list */
	initgroups("root", 0);
    /***************************************************************
     * parse standard options, and exit if there is an error
     ***************************************************************/
	if ((ptr = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", ptr);
		tst_exit();
	}

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Check to see if getgroups() fails on erraneous condition.
		 */
		TEST(getgroups(-1, gidset));

		if ((ret = TEST_RETURN) != -1) {
			tst_resm(TFAIL, "getgroups(-1,gidset) returned %d, expected -1 and errno = EINVAL", ret);
			errors++;
		} else if (STD_FUNCTIONAL_TEST) {
			if (errno != EINVAL) {
				tst_resm(TFAIL, "getgroups(-1,gidset) returned %d, errno = %d, expected errno %d (EINVAL)",
					ret, errno, EINVAL);
				errors++;
			} else {
				tst_resm(TPASS,
					"getgroups(-1,gidset) returned %d and error = %d (EINVAL) as expected",
					ret, errno);
			}
		}

		/*
		 * Check that if ngrps is zero that the number of groups is return and
		 * the the gidset array is not modified.
		 * This is a POSIX special case.
		 */

		memset(gidset, 052, NGROUPS);
		memset(cmpset, 052, NGROUPS);

		TEST(getgroups(0, gidset));
		if ((ret = TEST_RETURN) < 0) {
			tst_resm(TFAIL,
				"getgroups(0,gidset) returned %d with errno = %d, expected num gids with no change to gidset",
				ret, errno);
			errors++;
		} else if (STD_FUNCTIONAL_TEST) {
			/*
			 * check that gidset was unchanged
			 */
			if (memcmp(cmpset, gidset, NGROUPS) != 0) {
				tst_resm(TFAIL,
					"getgroups(0,gidset) returned %d, the gidset array was modified",
					ret);
				errors++;
			} else {
				tst_resm(TPASS,
					"getgroups(0,gidset) returned %d, the gidset array not was modified",
					ret);
			}
		}

		/*
		 * Check to see that is -1 is returned and errno is set to EINVAL when
		 * ngroups is not big enough to hold all groups.
		 */

		if (ret <= 1) {
			tst_resm(TCONF,
				"getgroups(0,gidset) returned %d, Unable to test that\nusing ngrps >=1 but less than number of grps",
				ret);
			errors++;
		} else {
			TEST(getgroups(ret - 1, gidset));
			if ((ret2 = TEST_RETURN) == -1) {
				if (STD_FUNCTIONAL_TEST) {
					if (errno != EINVAL) {
						tst_resm(TFAIL,
							"getgroups(%d, gidset) returned -1, but not errno %d (EINVAL) but %d",
							ret - 1, EINVAL, errno);
						errors++;
					} else {
						tst_resm(TPASS,
							"getgroups(%d, gidset) returned -1, and errno %d (EINVAL) when %d grps",
							ret - 1, errno, ret);
					}
				}
			} else {
				tst_resm(TFAIL,
					"getgroups(%d, gidset) returned %d, expected -1 and errno EINVAL.",
					ret - 1, ret2);
				errors++;
			}
		}

		/*
		 * Check to see if getgroups() succeeds and contains getgid's gid.
		 */

		TEST(getgroups(NGROUPS, gidset));
		if ((entries = TEST_RETURN) == -1) {
			tst_resm(TFAIL,
				"getgroups(NGROUPS,gidset) returned -1 and errno = %d",
				errno);
			errors++;
		} else if (STD_FUNCTIONAL_TEST) {

			/*
			 * Check to see if getgroups() contains getgid().
			 */

			group = getgid();

			for (i = 0; i < entries; i++) {
				if (gidset[i] == group) {
					tst_resm(TPASS,
						"getgroups(NGROUPS,gidset) ret %d contains gid %d (from getgid)",
						entries, group);
					break;
				}
			}

			if (i == entries) {
				tst_resm(TFAIL,
					"getgroups(NGROUPS,gidset) ret %d, does not contain gid %d (from getgid)",
					entries, group);
				errors++;
			}
		}

	}
	cleanup();

	return 0;
}				/* end main() */

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();

}				/* End cleanup() */
