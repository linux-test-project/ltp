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

gid_t gidset[NGROUPS];		/* storage for all group ids */
gid_t cmpset[NGROUPS];

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *ptr;		/* message returned from parse_opts */

	gid_t group;

	int i;			/* counter */
	int entries;		/* number of group entries */

	initgroups("root", 0);
	if ((ptr = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", ptr);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(getgroups(-1, gidset));

		if (TEST_RETURN == 0)
			tst_resm(TFAIL, "getgroups succeeded unexpectedly");
		else if (STD_FUNCTIONAL_TEST) {
			if (errno == EINVAL)
				tst_resm(TPASS,
				    "getgroups failed as expected with EINVAL");
			else
				tst_resm(TFAIL|TTERRNO,
				    "getgroups didn't fail as expected with EINVAL");
		}

		/*
		 * Check that if ngrps is zero that the number of groups is
		 * return and the the gidset array is not modified.
		 * This is a POSIX special case.
		 */

		memset(gidset, 052, NGROUPS);
		memset(cmpset, 052, NGROUPS);

		TEST(getgroups(0, gidset));
		if (TEST_RETURN == -1)
			tst_resm(TFAIL|TTERRNO, "getgroups failed");
		else if (STD_FUNCTIONAL_TEST) {
			if (memcmp(cmpset, gidset, NGROUPS) != 0)
				tst_resm(TFAIL,
				    "getgroups modified the gidset array");
			else
				tst_resm(TPASS,
				    "getgroups did not modify the gidset "
				    "array");
		}

		/*
		 * Check to see that is -1 is returned and errno is set to
		 * EINVAL when ngroups is not big enough to hold all groups.
		 */

		if (TEST_RETURN <= 1)
			tst_resm(TCONF,
				"getgroups returned %ld; unable to test that using ngrps >=1 but less than number of grps", TEST_RETURN);
		else {
			TEST(getgroups(TEST_RETURN - 1, gidset));
			if (TEST_RETURN == -1) {
				if (STD_FUNCTIONAL_TEST) {
					if (errno == EINVAL)
						tst_resm(TPASS,
						    "getgroups failed as "
						    "expected with EINVAL");
					else
						tst_resm(TFAIL|TERRNO,
						    "getgroups didn't fail "
						    "with EINVAL");
				}
			} else
				tst_resm(TFAIL,
				    "getgroups succeeded unexpectedly with %ld",
				    TEST_RETURN);
		}


		TEST(getgroups(NGROUPS, gidset));
		if ((entries = TEST_RETURN) == -1)
			tst_resm(TFAIL|TTERRNO,
			    "getgroups failed unexpectedly");
		else if (STD_FUNCTIONAL_TEST) {

			group = getgid();

			for (i = 0; i < entries; i++)
				if (gidset[i] == group) {
					tst_resm(TPASS,
					    "getgroups(NGROUPS,gidset) "
					    "returned %d contains gid %d "
					    "(from getgid)",
					    entries, group);
					break;
				}

			if (i == entries)
				tst_resm(TFAIL,
				    "getgroups(NGROUPS,gidset) ret %d, does "
				    "not contain gid %d (from getgid)",
				    entries, group);
		}

	}
	cleanup();

	tst_exit();
}

void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

void cleanup()
{
	TEST_CLEANUP;
}