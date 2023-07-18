/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  Ported by Wayne Boyer
 * Copyright (c) Cyril Hrubis <chrubis@suse.cz> 2013
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify that, getgroups() system call gets the supplementary group IDs
 *  of the calling process.
 *
 * Expected Result:
 *  The call succeeds in getting all the supplementary group IDs of the
 *  calling process. The effective group ID may or may not be returned.
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

/*
 * Don't forget to remove USE_LEGACY_COMPAT_16_H from Makefile after
 * rewriting all tests to the new API.
 */
#include "compat_16.h"

#define TESTUSER "root"

TCID_DEFINE(getgroups03);
int TST_TOTAL = 1;

static int ngroups;
static GID_T groups_list[NGROUPS];
static GID_T groups[NGROUPS];

static void verify_groups(int ret_ngroups);
static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	int gidsetsize = NGROUPS;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(GETGROUPS(cleanup, gidsetsize, groups_list));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "getgroups failed");
			continue;
		}

		verify_groups(TEST_RETURN);
	}

	cleanup();
	tst_exit();
}

/*
 * readgroups(GID_T *)  - Read supplimentary group ids of "root" user
 * Scans the /etc/group file to get IDs of all the groups to which TESTUSER
 * belongs and puts them into the array passed.
 * Returns the no of gids read.
 */
static int readgroups(GID_T groups[NGROUPS])
{
	struct group *grp;
	int ngrps = 0;
	int i;
	int found;
	GID_T g;

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
	return ngrps;
}

static void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

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
	if (SETGROUPS(cleanup, ngroups, groups) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "setgroups failed");
}

/*
 * verify_groups(int)  - Verify supplimentary group id values.
 *   This function verifies the gid values returned by getgroups() with
 *   the read values from /etc/group file.
 *  This function returns flag value which indicates success or failure
 *  of verification.
 */
static void verify_groups(int ret_ngroups)
{
	int i, j;
	GID_T egid;
	int egid_flag = 1;
	int fflag = 1;

	/*
	 * Loop through the array to verify the gids
	 * returned by getgroups().
	 * First, compare each element of the array
	 * returned by getgroups() with that read from
	 * group file.
	 */
	for (i = 0; i < ret_ngroups; i++) {
		for (j = 0; j < ngroups; j++) {
			if (groups_list[i] != groups[j]) {
				/* If loop ends and gids are not matching */
				if (j == ngroups - 1) {
					tst_resm(TFAIL, "getgroups returned "
						 "incorrect gid %d",
						 groups_list[i]);
					fflag = 0;
				} else {
					continue;
				}
			} else {
				break;
			}
		}
	}

	/* Now do the reverse comparison */
	egid = getegid();
	for (i = 0; i < ngroups; i++) {
		for (j = 0; j < ret_ngroups; j++) {
			if (groups[i] != groups_list[j]) {
				/*
				 * If the loop ends & gids are not matching
				 * if gid is not egid, exit with error
				 * else egid is returned by getgroups()
				 */
				if (j == (ret_ngroups - 1)) {
					if (groups[i] != egid) {
						tst_resm(TFAIL, "getgroups "
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
						egid_flag = 0;
					}
				}
			} else {
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
	if (egid_flag == 0)
		ngroups--;
	if (ngroups != ret_ngroups) {
		tst_resm(TFAIL,
			 "getgroups(2) returned incorrect no. of gids %d "
			 "(expected %d)", ret_ngroups, ngroups);
		fflag = 0;
	}

	if (fflag)
		tst_resm(TPASS, "getgroups functionality correct");
}

static void cleanup(void)
{
}
