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
 *	setreuid05.c
 *
 * DESCRIPTION
 *	Test the setreuid() feature, verifying the role of the saved-set-uid
 *	and setreuid's effect on it.
 *
 * ALGORITHM
 *
*      Setup:
 *	  Setup signal handling
 *	  Get user information.
 *	  Pause for SIGUSER1 if option specified.
 *
 *	Setup test values.
 *	Loop if the proper options are given.
 *	For each test set execute the system call
 *	  Check that we received the expected result.
 *	  If setreuid failed as expected
 *		check that the correct errno value was set.
 *	otherwise
 *		Issue Pass message.
 *	  Verify that the uid and euid values are still correct.
 *	Cleanup:
 *	  Print errno log and/or timing stats if option given.
 *
 * USAGE:  <for command-line>
 *	setreuid05 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	This test must be run by root.
 *	nobody must be a valid user.
 */

#include <errno.h>
#include <pwd.h>
#include <malloc.h>
#include <test.h>
#include <usctest.h>
#include <sys/wait.h>

char *TCID = "setreuid05";
extern int Tst_count;

/* flag to tell parent if child passed or failed. */
int flag = 0;

int fail = -1;
int pass = 0;
uid_t neg_one = -1;
int exp_enos[] = { EPERM, 0 };

uid_t root_pw_uid, nobody_pw_uid, daemon_pw_uid, bin_pw_uid;
char user1name[] = "nobody";
char user2name[] = "daemon";
char rootname[] = "root";
char binname[] = "bin";

struct passwd nobody, daemonpw, root, bin;

struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	int *exp_ret;
	struct passwd *exp_real_usr;
	struct passwd *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&nobody_pw_uid, &root_pw_uid, &pass, &nobody, &root, "Initially"}, {
	&neg_one, &nobody_pw_uid, &pass, &nobody, &nobody,
		    "After setreuid(-1, nobody),"}, {
	&neg_one, &root_pw_uid, &pass, &nobody, &root,
		    "After setreuid(-1, root),"}, {
	&daemon_pw_uid, &neg_one, &pass, &daemonpw, &root,
		    "After setreuid(daemon, -1),"}, {
	&neg_one, &bin_pw_uid, &pass, &daemonpw, &bin,
		    "After setreuid(-1, bin),"}, {
	&neg_one, &root_pw_uid, &fail, &daemonpw, &bin,
		    "After setreuid(-1, root),"}, {
	&neg_one, &nobody_pw_uid, &fail, &daemonpw, &bin,
		    "After setreuid(-1, nobody),"}, {
	&neg_one, &daemon_pw_uid, &pass, &daemonpw, &daemonpw,
		    "After setreuid(-1, daemon),"}, {
	&neg_one, &bin_pw_uid, &pass, &daemonpw, &bin,
		    "After setreuid(-1, bin),"}, {
	&bin_pw_uid, &daemon_pw_uid, &pass, &bin, &daemonpw,
		    "After setreuid(bin, daemon),"}, {
	&neg_one, &bin_pw_uid, &pass, &bin, &bin, "After setreuid(-1, bin),"},
	{
	&neg_one, &daemon_pw_uid, &pass, &bin, &daemonpw,
		    "After setreuid(-1, daemon),"}, {
	&daemon_pw_uid, &neg_one, &pass, &daemonpw, &daemonpw,
		    "After setreuid(daemon, -1),"}, {
&neg_one, &bin_pw_uid, &fail, &daemonpw, &daemonpw,
		    "After setreuid(-1, bin),"},};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

void setup(void);
void cleanup(void);
void uid_verify(struct passwd *, struct passwd *, char *);

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}

	/* Perform global setup for test */
	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i, pid, status;

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork failed");
		 /*NOTREACHED*/} else if (pid == 0) {	/* child */
			for (i = 0; i < TST_TOTAL; i++) {
				/* Set the real or effective user id */
				TEST(setreuid(*test_data[i].real_uid,
					      *test_data[i].eff_uid));

				if (TEST_RETURN == *test_data[i].exp_ret) {
					if (TEST_RETURN == neg_one) {
						if (TEST_ERRNO != EPERM) {
							tst_resm(TFAIL,
								 "setreuid(%d, %d) "
								 "did not set errno "
								 "value as expected.",
								 *test_data[i].
								 real_uid,
								 *test_data[i].
								 eff_uid);
							flag = -1;
							continue;
						}
						tst_resm(TPASS,
							 "setreuid(%d, %d) "
							 "failed as expected.",
							 *test_data[i].real_uid,
							 *test_data[i].eff_uid);
					} else {
						tst_resm(TPASS,
							 "setreuid(%d, %d) "
							 "succeeded as expected.",
							 *test_data[i].real_uid,
							 *test_data[i].eff_uid);
					}
				} else {
					tst_resm(TFAIL, "setreuid(%d, %d) "
						 "did not return as expected.",
						 *test_data[i].real_uid,
						 *test_data[i].eff_uid);
					flag = -1;
				}

				if (TEST_RETURN == -1) {
					TEST_ERROR_LOG(TEST_ERRNO);
				}
				/*
				 * Perform functional verification if test
				 * executed without (-f) option.
				 */
				if (STD_FUNCTIONAL_TEST) {
					uid_verify(test_data[i].exp_real_usr,
						   test_data[i].exp_eff_usr,
						   test_data[i].test_msg);
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

	if (getpwnam("daemon") == NULL) {
		tst_brkm(TBROK, NULL, "daemon must be a valid user.");
		tst_exit();
	 /*NOTREACHED*/}

	if (getuid() != 0) {
		tst_resm(TBROK, "Must be run as root");
		tst_exit();
	 /*NOTREACHED*/}

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	nobody = *(getpwnam("nobody"));
	nobody_pw_uid = nobody.pw_uid;

	daemonpw = *(getpwnam("daemon"));
	daemon_pw_uid = daemonpw.pw_uid;

	root = *(getpwnam("root"));
	root_pw_uid = root.pw_uid;

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
		tst_resm(TINFO, "ERROR: %s real uid = %d; effective uid = %d",
			 when, getuid(), geteuid());
		tst_resm(TINFO, "Expected: real uid = %d; effective uid = %d",
			 ru->pw_uid, eu->pw_uid);
		flag = -1;
	}
}
