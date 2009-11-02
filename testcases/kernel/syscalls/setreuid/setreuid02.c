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
 *	setreuid02.c
 *
 * DESCRIPTION
 *	Test setreuid() when executed by root.
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
 *	setreuid02 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
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
 *	This test must be ran as root.
 *	nobody, bin, and daemon must be valid users.
 */

#include <pwd.h>
#include <malloc.h>
#include <string.h>
#include <test.h>
#include <usctest.h>
#include <errno.h>

extern int Tst_count;

char *TCID = "setreuid02";
uid_t nobody_pw_uid, root_pw_uid, daemon_pw_uid, bin_pw_uid;
uid_t neg_one = -1;
int exp_enos[] = { 0 };

struct passwd nobody, daemonpw, root, bin;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	struct passwd *exp_real_usr;
	struct passwd *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&neg_one, &neg_one, &root, &root, "After setreuid(-1, -1),"}, {
	&nobody_pw_uid, &neg_one, &nobody, &root, "After setreuid(nobody, -1)"},
	{
	&root_pw_uid, &neg_one, &root, &root, "After setreuid(root,-1),"}, {
	&neg_one, &daemon_pw_uid, &root, &daemonpw,
		    "After setreuid(-1, daemon)"}, {
	&neg_one, &root_pw_uid, &root, &root, "After setreuid(-1,root),"}, {
	&bin_pw_uid, &neg_one, &bin, &root, "After setreuid(bin, -1)"}, {
&root_pw_uid, &neg_one, &root, &root, "After setreuid(-1, root)"},};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

void setup(void);		/* Setup function for the test */
void cleanup(void);		/* Cleanup function for the test */
void uid_verify(struct passwd *ru, struct passwd *eu, char *when);

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
		int i;

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			/* Set the real or effective user id */
			TEST(setreuid(*test_data[i].real_uid,
				      *test_data[i].eff_uid));

			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TBROK, "setreuid(%d, %d) failed",
					 *test_data[i].real_uid,
					 *test_data[i].eff_uid);
			} else {
				/*
				 * Perform functional verification if test
				 * executed without (-f) option.
				 */
				if (STD_FUNCTIONAL_TEST) {
					uid_verify(test_data[i].exp_real_usr,
						   test_data[i].exp_eff_usr,
						   test_data[i].test_msg);
				} else {
					tst_resm(TPASS, "Call succeeded.");
				}
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

	if (getpwnam("daemon") == NULL) {
		tst_brkm(TBROK, NULL, "daemon must be a valid user.");
		tst_exit();
	 /*NOTREACHED*/}

	/* Check that the test process id is root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be root for this test!");
		tst_exit();
	}

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	root = *(getpwnam("root"));
	root_pw_uid = root.pw_uid;

	nobody = *(getpwnam("nobody"));
	nobody_pw_uid = nobody.pw_uid;

	daemonpw = *(getpwnam("daemon"));
	daemon_pw_uid = daemonpw.pw_uid;

	bin = *(getpwnam("bin"));
	bin_pw_uid = bin.pw_uid;

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

void uid_verify(struct passwd *ru, struct passwd *eu, char *when)
{
	if ((getuid() != ru->pw_uid) || (geteuid() != eu->pw_uid)) {
		tst_resm(TFAIL, "ERROR: %s real uid = %d; effective uid = %d",
			 when, getuid(), geteuid());
		tst_resm(TFAIL, "Expected: real uid = %d; effective uid = %d",
			 ru->pw_uid, eu->pw_uid);
	} else {
		tst_resm(TPASS, "real or effective uid was modified as "
			 "expected");
	}
}
