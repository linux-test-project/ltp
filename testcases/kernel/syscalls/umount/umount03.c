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
 *    TEST IDENTIFIER	: umount03
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
 *	Verify that umount(2) returns -1 and sets errno to  EPERM if the user
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
 *  umount03 [-T type] -D device [-e] [-i n] [-I x] [-p x] [-t]
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
#include <sys/wait.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

static void help(void);
static void setup(void);
static void cleanup(void);
static void cleanup1(void);

char *TCID = "umount03";	/* Test program identifier.    */
extern int Tst_count;		/* TestCase counter for tst_* routine */

#define DEFAULT_FSTYPE "ext2"
#define FSTYPE_LEN	20
#define DIR_MODE	S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

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
} testcases = {
"User not Super User/root", EPERM, "EPERM"};

/* Total number of test cases. */
int TST_TOTAL = 1;

static int exp_enos[] = { EPERM, 0 };

static option_t options[] = {	/* options supported by mount04 test */
	{"T:", &Tflag, &fstype},	/* -T type of filesystem        */
	{"D:", &Dflag, &device},	/* -D device used for mounting  */
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;
	int status;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Check for mandatory option of the testcase */
	if (Dflag == 0) {
		tst_brkm(TBROK, NULL, "You must specifiy the device used for "
			 " mounting with -D option, Run '%s  -h' for option "
			 " information.", TCID);
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
			 (strlen(fstype)+1)) ? FSTYPE_LEN : (strlen(fstype)+1));
	} else {
		strncpy(Type, DEFAULT_FSTYPE, strlen(DEFAULT_FSTYPE)+1);
	}

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
			 "%s doesn't allow running more than one instance "
			 "at a time", TCID);
		STD_COPIES = 1;
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		switch (fork()) {

		case -1:
			/* fork() failed */
			tst_resm(TWARN, "fork() failed");
			continue;

		case 0:
			/* Child */

			/* Switch to nobody user */
			if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
				tst_brkm(TBROK, tst_exit, "\"nobody\" user"
					 "not present");
			}
			if (setuid(ltpuser->pw_uid) == -1) {
				tst_resm(TWARN, "setuid failed "
					 "to set the effective uid to %d",
					 ltpuser->pw_uid);
				exit(1);
			}
			TEST(umount(mntpoint));

			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == testcases.exp_errno)) {
				tst_resm(TPASS, "umount(2) expected failure "
					 "Got errno - %s : %s",
					 testcases.exp_errval,
					 testcases.err_desc);
			} else {
				tst_resm(TFAIL, "umount(2) failed to produce "
					 "expected error; %d, errno:%s got %d",
					 testcases.exp_errno,
					 testcases.exp_errval, TEST_ERRNO);
			}

			TEST_ERROR_LOG(TEST_ERRNO);
			exit(1);

		default:
			/* Parent */
			if ((wait(&status)) < 0) {
				tst_resm(TFAIL, "wait() failed");
			}
		}

	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Check whether we are root */
	if (geteuid() != 0) {
		if (Type != NULL) {
			free(Type);
		}
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	/* Switch to nobody user */
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		if (Type != NULL) {
			free(Type);
		}
		tst_brkm(TBROK, tst_exit, "\"nobody\" user not present");
	}
	if (seteuid(ltpuser->pw_uid) == -1) {
		if (Type != NULL) {
			free(Type);
		}
		tst_brkm(TBROK, tst_exit, "setuid failed to set the "
			 "effective uid to %d", ltpuser->pw_uid);
	}
	/* make a temp directory */
	tst_tmpdir();

	(void)sprintf(mntpoint, "mnt_%d", getpid());

	if (mkdir(mntpoint, DIR_MODE)) {
		tst_brkm(TBROK, cleanup1, "mkdir(%s, %#o) failed; errno = %d:"
			 " %s", mntpoint, DIR_MODE, errno, strerror(errno));
	}

	if (seteuid(0) == -1) {
		tst_brkm(TBROK, cleanup1, "setuid failed to set the effective"
			 " uid to %d", ltpuser->pw_uid);
	}
	/* set up expected error numbers */
	TEST_EXP_ENOS(exp_enos);

	if(access(device,F_OK)) {
		tst_brkm(TBROK, cleanup1,
			"Device '%s' does not exist", device);
	}

	TEST(mount(device, mntpoint, Type, 0, NULL));

	if (TEST_RETURN != 0) {
		tst_brkm(TBROK, cleanup1, "mount(2) failed to mount device %s "
			 "errno = %d : %s", device, TEST_ERRNO,
			 strerror(TEST_ERRNO));
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

	return;
}				/* End setup() */

/*
 *cleanup1() -  performs cleanup for this test at premature exit.
 */
void cleanup1()
{
	if (Type != NULL) {
		free(Type);
	}

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it. */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();

	return;
}				/* End cleanup() */

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	TEST(umount(mntpoint));
	if (TEST_RETURN != 0) {
		tst_resm(TWARN, "umount(2) failed to umount device %s while"
			 " cleanuo errno = %d : %s", mntpoint, TEST_ERRNO,
			 strerror(TEST_ERRNO));
	}

	if (Type != NULL) {
		free(Type);
	}

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it. */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();

	return;
}				/* End cleanup() */

/*
 * issue a help message
 */
void help()
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}
