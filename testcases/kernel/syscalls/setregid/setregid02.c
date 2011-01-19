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
 * 	setregid02.c
 *
 * DESCRIPTION
 * 	Test that setregid() fails and sets the proper errno values when a
 *	non-root user attemps to change the real or effective group id to a
 *	value other than the current gid or the current effective gid.
 *
 * ALGORITHM
 *
 *	Setup:
 *	  Setup signal handling
 *	  Get user information.
 *	  Pause for SIGUSER1 if option specified.
 *	Setup test values.
 *	Loop if the proper options are given.
 *	For each test set execute the system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise,
 *		Verify the Functionality of system call
 *		if successful,
 *			Issue Functionality-Pass message.
 *		Otherwise,
 *			Issue Functionality-Fail message.
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given.
 *
 * USAGE:  <for command-line>
 *	setregid02 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 * 	This test must be ran as root.
 *	users must be a valid group.
 */

#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "usctest.h"
#include <errno.h>


char *TCID = "setregid02";
gid_t users_gr_gid, root_gr_gid, bin_gr_gid;
gid_t neg_one = -1;
int exp_enos[] = { EPERM, 0 };
gid_t inval_user = (USHRT_MAX);
char nobody_uid[] = "nobody";
struct passwd *nobody;

struct group users, root, bin;
struct passwd *nobody;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	gid_t *real_gid;
	gid_t *eff_gid;
	int exp_errno;
	struct group *exp_real_usr;
	struct group *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&neg_one, &root_gr_gid, EPERM, &users, &users,
		    "After setregid(-1, root),"}, {
	&neg_one, &bin_gr_gid, EPERM, &users, &users,
		    "After setregid(-1, bin)"}, {
	&root_gr_gid, &neg_one, EPERM, &users, &users,
		    "After setregid(root,-1),"}, {
	&bin_gr_gid, &neg_one, EPERM, &users, &users,
		    "After setregid(bin, -1),"}, {
	&root_gr_gid, &bin_gr_gid, EPERM, &users, &users,
		    "After setregid(root, bin)"}, {
	&bin_gr_gid, &root_gr_gid, EPERM, &users, &users,
		    "After setregid(bin, root),"}, {
	&inval_user, &neg_one, EINVAL, &users, &users,
		    "After setregid(invalid group, -1),"}, {
&neg_one, &inval_user, EINVAL, &users, &users,
		    "After setregid(-1, invalid group),"},};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

void setup(void);		/* Setup function for the test */
void cleanup(void);		/* Cleanup function for the test */
void gid_verify(struct group *ru, struct group *eu, char *when);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i;

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			/* Set the real or effective group id */
			TEST(setregid(*test_data[i].real_gid,
				      *test_data[i].eff_gid));

			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == test_data[i].exp_errno) {
					tst_resm(TPASS, "setregid(%d, %d) "
						 "failed as expected.",
						 *test_data[i].real_gid,
						 *test_data[i].eff_gid);
				} else if (TEST_ERRNO == test_data[0].exp_errno) {
					tst_resm(TPASS, "setregid(%d, %d) "
						 "failed as expected.",
						 *test_data[i].real_gid,
						 *test_data[i].eff_gid);
				} else {
					tst_resm(TFAIL, "setregid(%d, %d) "
						 "failed (%d) but did not set the "
						 "expected errno (%d).",
						 *test_data[i].real_gid,
						 *test_data[i].eff_gid,
						 TEST_ERRNO,
						 test_data[i].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "setregid(%d, %d) "
					 "did not fail (ret: %ld) as expected (ret: -1).",
					 *test_data[i].real_gid,
					 *test_data[i].eff_gid, TEST_RETURN);
			}
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				gid_verify(test_data[i].exp_real_usr,
					   test_data[i].exp_eff_usr,
					   test_data[i].test_msg);
			} else {
				tst_resm(TINFO, "Call succeeded.");
			}
		}
	}
	cleanup();
	tst_exit();
	tst_exit();

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	struct group *junk;

	tst_require_root(NULL);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	if ((nobody = getpwnam("nobody")) == NULL) {
		tst_brkm(TBROK, NULL, "getpwnam(\"nobody\") failed");
	}

	if (setgid(nobody->pw_gid) == -1) {
		tst_brkm(TBROK|TERRNO, NULL,
		    "setgid failed to set the effective gid to %d",
		    nobody->pw_gid);
	}
	if (setuid(nobody->pw_uid) == -1) {
		tst_brkm(TBROK|TERRNO, NULL,
		    "setuid failed to to set the effective uid to %d",
		    nobody->pw_uid);
	}

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

#define GET_GID(group)	do {		\
	junk = getgrnam(#group);	\
	if (junk == NULL) {		\
		tst_brkm(TBROK|TERRNO, NULL, "getgrnam(\"%s\") failed", #group); \
	}				\
	group ## _gr_gid = junk->gr_gid;\
} while (0)

	GET_GID(root);
	GET_GID(users);
	GET_GID(bin);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;
 }

void gid_verify(struct group *rg, struct group *eg, char *when)
{
	if ((getgid() != rg->gr_gid) || (getegid() != eg->gr_gid)) {
		tst_resm(TINFO, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_resm(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg->gr_gid, eg->gr_gid);
	}
}
