/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/******************************************************************************
 *
 *    TEST IDENTIFIER	: mount04
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Test for checking EPERM
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that mount(2) returns -1 and sets errno to  EPERM if the user
 *	is not the super-user.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Create a mount point.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed and errno == EPERM
 *		Issue sys call passed with expected return value and errno.
 *	  Otherwise,
 *		Issue sys call failed to produce expected error.
 *	  Do cleanup for each test.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *	  Delete the temporary directory(s)/file(s) created.
 *
 * USAGE:  <for command-line>
 *  mount04 [-T type] -D device [-e] [-i n] [-I x] [-p x] [-t]
 *			where,  -T type : specifies the type of filesystem to
 *					  be mounted. Default ext2.
 *				-D device : device to be mounted.
 *				-e   : Turn on errno logging.
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 * RESTRICTIONS
 *	test must be run with the -D option
 *	test doesn't support -c option to run it in parallel, as mount
 *	syscall is not supposed to run in parallel.
 *****************************************************************************/

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

static void help(void);
static void setup(void);
static void cleanup(void);

char *TCID = "mount04";		/* Test program identifier.    */

#define DEFAULT_FSTYPE "ext2"
#define FSTYPE_LEN	20
#define DIR_MODE	S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP

static char *Type;
static char mntpoint[PATH_MAX];
static char *fstype;
static char *device;
static int Tflag = 0;
static int Dflag = 0;

static struct test_case_t {
	char *err_desc;		/* error description            */
	int exp_errno;		/* Expected error no            */
	char *exp_errval;	/* Expected error value string  */
} testcases[] = {
	{
	"User not Super User/root", EPERM, "EPERM"}
};

/* Total number of test cases. */
int TST_TOTAL = sizeof(testcases) / sizeof(testcases[0]);

static int exp_enos[] = { EPERM, 0 };

static option_t options[] = {	/* options supported by mount04 test */
	{"T:", &Tflag, &fstype},	/* -T type of filesystem        */
	{"D:", &Dflag, &device},	/* -D device used for mounting  */
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Check for mandatory option of the testcase */
	if (Dflag == 0) {
		tst_brkm(TBROK, NULL, "You must specifiy the device used for "
			 " mounting with -D option.");
		tst_exit();
	}

	Type = (char *)malloc(FSTYPE_LEN);
	if (!Type) {
		tst_brkm(TBROK, NULL, "malloc - alloc of %d failed",
			 FSTYPE_LEN);
		tst_exit();
	}

	if (Tflag == 1) {
		strncpy(Type, fstype,
			(FSTYPE_LEN <
			 strlen(fstype)) ? FSTYPE_LEN : strlen(fstype));
	} else {
		strncpy(Type, DEFAULT_FSTYPE, strlen(DEFAULT_FSTYPE));
	}

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
			 "%s doesn't allow running more than one instance "
			 "at a time", TCID);
		STD_COPIES = 1;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			TEST(mount(device, mntpoint, Type, 0, NULL));

			/* check return code */
			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == testcases[i].exp_errno)) {
				tst_resm(TPASS, "mount(2) expected failure; "
					 "Got errno - %s : %s",
					 testcases[i].exp_errval,
					 testcases[i].err_desc);
			} else {
				if (umount(mntpoint) == -1) {
					tst_brkm(TBROK, cleanup, "umount(2) "
						 "failed to umount mntpoint %s "
						 "errno - %d : %s", mntpoint,
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				}
				tst_resm(TFAIL, "mount(2) failed to produce "
					 "expected error; %d, errno:%s got %d",
					 testcases[i].exp_errno,
					 testcases[i].exp_errval, TEST_ERRNO);
			}

			TEST_ERROR_LOG(TEST_ERRNO);

		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Check whether we are root */
	if (geteuid() != 0) {
		free(Type);
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}

	ltpuser = getpwnam(nobody_uid);
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK, cleanup,
			 "seteuid() failed to change euid to %d "
			 "errno = %d : %s", ltpuser->pw_uid, TEST_ERRNO,
			 strerror(TEST_ERRNO));
	}
	/* make a temp directory */
	tst_tmpdir();

	(void)sprintf(mntpoint, "mnt_%d", getpid());

	if (mkdir(mntpoint, DIR_MODE)) {
		tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) failed; "
			 "errno = %d: %s", mntpoint, DIR_MODE, errno,
			 strerror(errno));
	}

	/* set up expected error numbers */
	TEST_EXP_ENOS(exp_enos);

	/* Setup for mount(2) returning errno EACCES. */

	TEST_PAUSE;

	return;
}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	free(Type);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();

	/* Set effective user id back to root */
	if (seteuid(0) == -1) {
		tst_resm(TINFO, "seteuid failed to "
			 "to set the effective uid to root");
		perror("seteuid");
	}

	return;
}

/*
 * issue a help message
 */
void help()
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}