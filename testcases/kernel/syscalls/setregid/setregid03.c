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
 * 	setregid03.c
 *
 * DESCRIPTION
 * 	Test setregid() when executed by a non-root user.
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
 *	setregid03 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
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
 *	users, sys and bin must be valid groups.
 */

#include <errno.h>
#include <grp.h>
#include <malloc.h>
#include <pwd.h>
#include <string.h>
#include <test.h>
#include <usctest.h>
#include <sys/wait.h>

extern int Tst_count;

char *TCID = "setregid03";
int fail = -1;
int pass = 0;
gid_t neg_one = -1;
int exp_enos[] = { 0 };
gid_t users_gr_gid, root_gr_gid, sys_gr_gid, bin_gr_gid;
uid_t nobody_pw_uid;

/* flag to tell parent if child passed or failed. */
int flag = 0;

struct group users, sys, root, bin;
struct passwd nobody;
/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	gid_t *real_gid;
	gid_t *eff_gid;
	int *exp_ret;
	struct group *exp_real_usr;
	struct group *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&sys_gr_gid, &bin_gr_gid, &pass, &sys, &bin,
		    "After setregid(sys, bin),"}, {
	&neg_one, &sys_gr_gid, &pass, &sys, &sys, "After setregid(-1, sys)"},
	{
	&neg_one, &bin_gr_gid, &pass, &sys, &bin, "After setregid(-1, bin),"},
	{
	&bin_gr_gid, &neg_one, &pass, &bin, &bin, "After setregid(bin, -1),"},
	{
	&neg_one, &neg_one, &pass, &bin, &bin, "After setregid(-1, -1),"}, {
	&neg_one, &bin_gr_gid, &pass, &bin, &bin, "After setregid(-1, bin),"},
	{
	&bin_gr_gid, &neg_one, &pass, &bin, &bin, "After setregid(bin, -1),"},
	{
	&bin_gr_gid, &bin_gr_gid, &pass, &bin, &bin,
		    "After setregid(bin, bin),"}, {
	&sys_gr_gid, &neg_one, &fail, &bin, &bin, "After setregid(sys, -1)"},
	{
	&neg_one, &sys_gr_gid, &fail, &bin, &bin, "After setregid(-1, sys)"},
	{
&sys_gr_gid, &sys_gr_gid, &fail, &bin, &bin,
		    "After setregid(sys, sys)"},};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

void setup(void);		/* Setup function for the test */
void cleanup(void);		/* Cleanup function for the test */
void gid_verify(struct group *ru, struct group *eu, char *when);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}

	/* Perform global setup for test */
	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int pid, status, i;

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* set the appropriate ownership values */
		if (setregid(sys_gr_gid, bin_gr_gid) == -1) {
			tst_brkm(TBROK, cleanup, "Initial setregid failed");
		 /*NOTREACHED*/}

		if (seteuid(nobody_pw_uid) == -1) {
			tst_brkm(TBROK, cleanup, "Initial seteuid failed");
		 /*NOTREACHED*/}

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork failed");
		 /*NOTREACHED*/} else if (pid == 0) {	/* child */
			for (i = 0; i < TST_TOTAL; i++) {
				gid_t test_ret;
				/* Set the real or effective group id */
				TEST(setregid(*test_data[i].real_gid,
					      *test_data[i].eff_gid));
				test_ret = TEST_RETURN;

				if (test_ret == *test_data[i].exp_ret) {
					if (test_ret == neg_one) {
						if (TEST_ERRNO != EPERM) {
							tst_resm(TFAIL,
								 "setregid(%d, %d) "
								 "did not set errno "
								 "value as expected.",
								 *test_data[i].
								 real_gid,
								 *test_data[i].
								 eff_gid);
							fail = -1;
							continue;
						} else {
							tst_resm(TPASS,
								 "setregid(%d, %d) "
								 "failed as expected.",
								 *test_data[i].
								 real_gid,
								 *test_data[i].
								 eff_gid);
						}
					} else {
						tst_resm(TPASS,
							 "setregid(%d, %d) "
							 "succeeded as expected.",
							 *test_data[i].real_gid,
							 *test_data[i].eff_gid);
					}
				} else {
					tst_resm(TFAIL, "setregid(%d, %d) "
						 "did not return as expected.",
						 *test_data[i].real_gid,
						 *test_data[i].eff_gid);
					flag = -1;
				}
				if (test_ret == -1) {
					TEST_ERROR_LOG(TEST_ERRNO);
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
			exit(flag);
		} else {	/* parent */
			waitpid(pid, &status, 0);
			if (WEXITSTATUS(status) != 0) {
				tst_resm(TFAIL, "test failed within "
					 "child process.");
			}
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	if (getpwnam("nobody") == NULL) {
		tst_brkm(TBROK, NULL, "nobody must be a valid user.");
		tst_exit();
	 /*NOTREACHED*/}

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be root for this test!");
		tst_exit();
	}

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	nobody = *(getpwnam("nobody"));
	nobody_pw_uid = nobody.pw_uid;

	root = *(getgrnam("root"));
	root_gr_gid = root.gr_gid;

	users = *(getgrnam("users"));
	users_gr_gid = users.gr_gid;

	sys = *(getgrnam("sys"));
	sys_gr_gid = sys.gr_gid;

	bin = *(getgrnam("bin"));
	bin_gr_gid = bin.gr_gid;

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

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}

void gid_verify(struct group *rg, struct group *eg, char *when)
{
	if ((getgid() != rg->gr_gid) || (getegid() != eg->gr_gid)) {
		tst_resm(TINFO, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_resm(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg->gr_gid, eg->gr_gid);
		flag = -1;
	}
}
