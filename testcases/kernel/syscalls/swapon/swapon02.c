/* Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
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
/**************************************************************************
 *
 *    TEST IDENTIFIER	  : swapon02
 *
 *    EXECUTED BY	  : root / superuser
 *
 *    TEST TITLE	  : Test checking for basic error conditions
 *    		            for swapon(2)
 *
 *    TEST CASE TOTAL	  : 5
 *
 *    AUTHOR	 	  : Aniruddha Marathe <aniruddha.marathe@wipro.com> and
 *                          Ricardo Salveti de Araujo <rsalveti@linux.vnet.ibm.com> for
 *                          the EBUSY test case
 *
 *    SIGNALS
 * 		 Uses SIGUSR1 to pause before test if option set.
 * 		  (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *		  This test case checks whether swapon(2) system call  returns
 *		  1. ENOENT when the path does not exist
 *		  2. EINVAL when the path exists but is invalid
 *		  3. EPERM when user is not a superuser
 *		  4. EBUSY when the specified path is already being used as a swap area
 *		$
 * 		  Setup:
 *		    Setup signal handling.
 *		    Pause for SIGUSR1 if option specified.
 *		   1. For testing error on invalid user, change the effective uid
 *
 * 		  Test:
 *		    Loop if the proper options are given.
 *		    Execute system call.
 *		    Check return code, if system call fails with errno == expected errno
 *	 	    Issue syscall passed with expected errno
 *		    Otherwise,
 *		    Issue syscall failed to produce expected errno
 *
 * 		  Cleanup:
 * 		    Do cleanup for the test.
 * 		  $
 * USAGE:  <for command-line>
 *  swapon02 [-e] [-i n] [-I x] [-p x] [-t] [-h] [-f] [-p]
 *  where
 *		  -e   : Turn on errno logging.
 *		  -i n : Execute test n times.
 *		  -I x : Execute test for x seconds.
 *		  -p   : Pause for SIGUSR1 before starting
 *		  -P x : Pause for x seconds between iterations.
 *		  -t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 *Incompatible with kernel versions below 2.1.35.
 *Incompatible if MAX_SWAPFILES definition in later kernels is changed
 * -c option can't be used.
 *
 *CHANGES:
 * 2005/01/01  Add extra check to stop test if swap file is on tmpfs
 *             -Ricky Ng-Adam (rngadam@yahoo.com)
 * 01/02/03  Added fork to handle SIGSEGV generated when the intentional EPERM
 * error for hitting MAX_SWAPFILES is hit.
 * -Robbie Williamson <robbiew@us.ibm.com>
 * 05/17/07  Fixing the test description and added the EBUSY test case
 *           - Ricardo Salveti de Araujo (rsalveti@linux.vnet.ibm.com)
 * 08/16/07  Removed the the 'more than MAX_SWAPFILES swapfiles in use' test
 * case as now we have a separated file only for this test.
 *           - Ricardo Salveti de Araujo (rsalveti@linux.vnet.ibm.com)
 *****************************************************************************/

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <sys/utsname.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"
#include "config.h"
#include "linux_syscall_numbers.h"
#include "swaponoff.h"

static void setup();
static void cleanup();
static int setup01();
static int cleanup01();
static int setup02();
static int setup03();
static int cleanup03();
void handler(int);

char *TCID = "swapon02";	/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

struct utsname uval;
char *kmachine;

static int exp_enos[] = { EPERM, EINVAL, ENOENT, EBUSY, 0 };

static struct test_case_t {
	char *err_desc;		/* error description */
	int exp_errno;		/* expected error number */
	char *exp_errval;	/* Expected errorvalue string */
	char *path;		/* path to swapon */
	int (*setupfunc) ();	/* Test setup function */
	int (*cleanfunc) ();	/* Test cleanup function */
} testcase[] = {
	{
	"Path does not exist", ENOENT, "ENOENT", "./abcd", NULL, NULL}, {
	"Invalid path", EINVAL, "EINVAL", "./nofile", setup02, NULL}, {
	"Permission denied", EPERM, "EPERM", "./swapfile01",
		    setup01, cleanup01}, {
	"The specified path is already being used as a swap area",
		    EBUSY, "EBUSY", "./alreadyused", setup03, cleanup03}
};

int main(int ac, char **av)
{
	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	uname(&uval);
	kmachine = uval.machine;
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++) {

			/* do the setup if the test have one */
			if (testcase[i].setupfunc
			    && testcase[i].setupfunc() == -1) {
				tst_resm(TWARN,
					 "Failed to setup test %d."
					 " Skipping test", i);
				continue;
			} else {
				/* run the test */
				TEST(syscall(__NR_swapon,testcase[i].path, 0));
			}
			/* do the clean if the test have one */
			if (testcase[i].cleanfunc
			    && testcase[i].cleanfunc() == -1) {
				tst_brkm(TBROK, cleanup,
					 "Cleanup failed, quitting the test");
			}
			/* check return code */
			if ((TEST_RETURN == -1)
			    && (TEST_ERRNO == testcase[i].exp_errno)) {
				tst_resm(TPASS,
					 "swapon(2) expected failure;"
					 " Got errno - %s : %s",
					 testcase[i].exp_errval,
					 testcase[i].err_desc);
			} else {
				tst_resm(TFAIL, "swapon(2) failed to produce"
					 " expected error: %d, errno"
					 ": %s and got %d. "
					 " System reboot after"
					 " execution of LTP"
					 " test suite is"
					 " recommended.",
					 testcase[i].exp_errno,
					 testcase[i].exp_errval, TEST_ERRNO);
				/*If swapfile is turned on, turn it off */
				if (TEST_RETURN == 0) {
					if (syscall(__NR_swapoff, testcase[i].path) != 0) {
						tst_resm(TWARN, "Failed to"
							 " turn off swapfile"
							 " swapfile. System"
							 " reboot after"
							 " execution of LTP"
							 " test suite is"
							 " recommended.");
					}
				}
			}
			TEST_ERROR_LOG(TEST_ERRNO);
		}		/*End of TEST LOOPS */
	}			/*End of TEST LOOPING */

	cleanup();
	tst_exit();
}				/*End of main */

/*
 * setup01() - This function creates the file and sets the user as nobody
 */
int setup01()
{
	int pagesize = getpagesize();

	/*create file */
	if ((strncmp(kmachine, "ia64", 4)) == 0) {
		if (system
		    ("dd if=/dev/zero of=swapfile01 bs=1024  count=65536 > tmpfile"
		     " 2>&1") != 0) {
			tst_brkm(TBROK, cleanup,
				 "Failed to create file for swap");
		}
	} else if (pagesize == 65536) {
		if (system
		    ("dd if=/dev/zero of=swapfile01 bs=1048  count=655 > tmpfile"
		     " 2>&1") != 0) {
			tst_brkm(TBROK, cleanup,
				 "Failed to create file for swap");
		}
	} else {
		if (system
		    ("dd if=/dev/zero of=swapfile01 bs=1048  count=40 > tmpfile"
		     " 2>&1") != 0) {
			tst_brkm(TBROK, cleanup,
				 "Failed to create file for swap");
		}
	}

	/* make above file a swap file */
	if (system("mkswap swapfile01 > tmpfile 2>&1") != 0) {
		tst_brkm(TBROK, cleanup, "Failed to make swapfile");
	}

	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_resm(TWARN, "\"nobody\" user not present. skipping test");
		return -1;
	}

	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TWARN, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("seteuid");
		return -1;
	}
	return 0;		/* user switched to nobody */
}

/*
 * cleanup01() - switch back to user root and gives swapoff to the swap file
 */
int cleanup01()
{
	if (seteuid(0) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup,
			"seteuid failed to set uid to root");
	}

	return 0;
}

/*
 * setup02() - create a normal file, to be used with swapon
 */
int setup02()
{
	int fd;
	fd = creat("nofile", S_IRWXU);
	if (fd == -1)
		tst_resm(TWARN, "Failed to create temporary file");
	close(fd);
	return 0;
}

/*
 * setup03() - This function creates the swap file and turn it on
 */
int setup03()
{
	int pagesize = getpagesize();
	int res = 0;

	/*create file */
	if ((strncmp(kmachine, "ia64", 4)) == 0) {
		if (system
		    ("dd if=/dev/zero of=alreadyused bs=1024  count=65536 > tmpfile"
		     " 2>&1") != 0) {
			tst_brkm(TBROK, cleanup,
				 "Failed to create file for swap");
		}
	} else if (pagesize == 65536) {
		if (system
		    ("dd if=/dev/zero of=alreadyused bs=1048  count=655 > tmpfile"
		     " 2>&1") != 0) {
			tst_brkm(TBROK, cleanup,
				 "Failed to create file for swap");
		}
	} else {
		if (system
		    ("dd if=/dev/zero of=alreadyused bs=1048  count=40 > tmpfile"
		     " 2>&1") != 0) {
			tst_brkm(TBROK, cleanup,
				 "Failed to create file for swap");
		}
	}

	/* make above file a swap file */
	if (system("mkswap alreadyused > tmpfile 2>&1") != 0) {
		tst_brkm(TBROK, cleanup, "Failed to make swapfile");
	}

	/* turn on the swap file */
	if ((res = syscall(__NR_swapon, "alreadyused", 0)) != 0) {
		tst_resm(TWARN, "Failed swapon for file alreadyused"
			 " returned %d", res);
		return -1;
	}

	return 0;
}

/*
 * cleanup03() - clearing the turned on swap file
 */
int cleanup03()
{
	/* give swapoff to the test swap file */
	if (syscall(__NR_swapoff, "alreadyused") != 0) {
		tst_resm(TWARN, "Failed to turn off swap files. system"
			 " reboot after execution of LTP test"
			 " suite is recommended");
		return -1;
	}

	return 0;
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check whether we are root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}

	tst_tmpdir();

	if (tst_is_cwd_tmpfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a tmpfs filesystem");
	}

	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a nfs filesystem");
	}

	TEST_PAUSE;

}

/*
* cleanup() - Performs one time cleanup for this test at
* completion or premature exit
*/
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();
}