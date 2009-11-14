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
 * 	setregid04.c
 *
 * DESCRIPTION
 * 	Test setregid() when executed by root.
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
 *	setregid04 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
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
 */

#include <pwd.h>
#include <grp.h>
#include <malloc.h>
#include <string.h>
#include <test.h>
#include <usctest.h>
#include <errno.h>

extern int Tst_count;

char *TCID = "setregid04";
gid_t users_gr_gid, root_gr_gid, daemon_gr_gid, bin_gr_gid;
gid_t neg_one = -1;
int exp_enos[] = { 0 };

/* Avoid clashing with daemon in unistd.h. */
struct group users_gr, daemon_gr, root_gr, bin_gr;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	gid_t *real_gid;
	gid_t *eff_gid;
	struct group *exp_real_usr;
	struct group *exp_eff_usr;
	const char *test_msg;
} test_data[] = {
	{
		&root_gr_gid, &root_gr_gid, &root_gr, &root_gr,
		"After setregid(root, root),"
	}, {
		&users_gr_gid, &neg_one, &users_gr, &root_gr,
		"After setregid(users, -1)"
	}, {
		&root_gr_gid, &neg_one, &root_gr, &root_gr,
		"After setregid(root,-1),"
	}, {
		&neg_one, &neg_one, &root_gr, &root_gr,
		"After setregid(-1, -1),"
	}, {
		&neg_one, &root_gr_gid, &root_gr, &root_gr,
		"After setregid(-1, root)"
	}, {
		&root_gr_gid, &neg_one, &root_gr, &root_gr,
		"After setregid(root, -1),"
	}, {
		&daemon_gr_gid, &users_gr_gid, &daemon_gr, &users_gr,
		"After setregid(daemon, users)"
	}, {
		&neg_one, &neg_one, &daemon_gr, &users_gr,
		"After setregid(-1, -1)"
	}, {
		&neg_one, &users_gr_gid, &daemon_gr, &users_gr,
		"After setregid(-1, users)"
	}
};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

void setup(void);		/* Setup function for the test */
void cleanup(void);		/* Cleanup function for the test */
void gid_verify(struct group *ru, struct group *eu, const char *when);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL)	{
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
		/*NOTREACHED*/
	}

	/* Perform global setup for test */
	setup();

	/* check looping state if -i option is given */
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
				tst_resm(TBROK, "setregid(%d, %d) failed",
					 *test_data[i].real_gid,
					 *test_data[i].eff_gid);
			} else {
				/*
				 * Perform functional verification if test
				 * executed without (-f) option.
				 */
				if (STD_FUNCTIONAL_TEST) {
					gid_verify(test_data[i].exp_real_usr,
						   test_data[i].exp_eff_usr,
						   test_data[i].test_msg);
				} else {
					tst_resm(TPASS, "Call succeeded.");
				}
			}
		}
	}
	cleanup();
	/*NOTREACHED*/
	return 0;
}

#define SAFE_GETGROUP(GROUPNAME)	\
	if ((junk = getgrnam(#GROUPNAME)) == NULL) { \
		tst_brkm(TBROK, NULL, "Couldn't find the `" #GROUPNAME "' group"); \
		tst_exit(); \
	} \
	memcpy((void*) &GROUPNAME ## _gr, (const void*) junk, sizeof(struct group)); \
	GROUPNAME ## _gr_gid = GROUPNAME ## _gr.gr_gid

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	struct group *junk;

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be root for this test!");
		tst_exit();
	}

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	SAFE_GETGROUP(root);
	SAFE_GETGROUP(users);
	SAFE_GETGROUP(daemon);
	SAFE_GETGROUP(bin);

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
	/*NOTREACHED*/
}

void gid_verify(struct group *rg, struct group *eg, const char *when)
{
	if ((getgid() != rg->gr_gid) || (getegid() != eg->gr_gid)) {
		tst_resm(TFAIL, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_resm(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg->gr_gid, eg->gr_gid);
	} else {
		tst_resm(TPASS, "real or effective gid was modified as "
			 "expected");
	}
}
