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
 * Test Name: mknod05
 *
 * Test Description:
 *  Verify that mknod(2) succeeds when used by root to create a filesystem
 *  node with set group-ID bit set on a directory with set group-ID bit set.
 *  The node created should have set group-ID bit set and its gid should be
 *  equal to that of its parent directory.
 *
 * Expected Result:
 *  mknod() should return value 0 on success and node created should have
 *  set group-ID bit set and its gid should be equal to that of its parent
 *  directory.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  mknod05 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  This test should be run by 'super-user' (root) only.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"

#define LTPUSER		"nobody"
#define MODE_RWX	S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO
#define MODE_SGID       S_IFIFO | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO
#define DIR_TEMP	"testdir_5"
#define TNODE		"tnode_%d"

struct stat buf;		/* struct. to hold stat(2) o/p contents */
struct passwd *user1;		/* struct. to hold getpwnam(3) o/p contents */

char *TCID = "mknod05";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
char node_name[PATH_MAX];	/* buffer to hold node name created */

gid_t group1_gid, group2_gid, mygid;	/* user and process group id's */
uid_t save_myuid, user1_uid;	/* user and process user id's */
pid_t mypid;			/* process id */

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	int fflag;		/* functionality flag variable */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 *  Attempt to create a filesystem node with group id (sgid)
		 *  bit set on a directory with group id (sgid) bit set
		 *  such that, the node created by mknod(2) should have
		 *  group id (sgid) bit set and node's gid should be equal
		 *  to that of effective gid of the process.
		 */
		TEST(mknod(node_name, MODE_SGID, 0));

		/* Check return code from mknod(2) */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "mknod(%s, %#o, 0)  failed, errno=%d : "
				 "%s", node_name, MODE_SGID, TEST_ERRNO,
				 strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Perform functional verification if test executed
		 * without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/* Set the functionality flag */
			fflag = 1;

			/* Check for node's creation */
			if (stat(node_name, &buf) < 0) {
				tst_resm(TFAIL, "stat() of %s failed, errno:%d",
					 node_name, TEST_ERRNO);
				/* unset functionality flag */
				fflag = 0;
			}

			/* Verify mode permissions of node */
			if (!(buf.st_mode & S_ISGID)) {
				tst_resm(TFAIL, "%s: Incorrect modes, "
					 "setgid bit not set", node_name);
				/* unset flag as functionality fails */
				fflag = 0;
			}

			/* Verify group ID */
			if (buf.st_gid != group2_gid) {
				tst_resm(TFAIL, "%s: Incorrect group",
					 node_name);
				/* unset flag as functionality fails */
				fflag = 0;
			}
			if (fflag) {
				tst_resm(TPASS, "Functionality of mknod(%s, "
					 "%#o, 0) successful",
					 node_name, MODE_SGID);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}

		/* Remove the node for the next go `round */
		if (unlink(node_name) == -1) {
			tst_resm(TWARN, "unlink(%s) failed, errno:%d %s",
				 node_name, errno, strerror(errno));
		}
	}

	/* change the directory back to temporary directory */
	chdir("..");

	/*
	 * Invoke cleanup() to delete the test directories created
	 * in the setup() and exit main().
	 */
	cleanup();

	tst_exit();
}

/*
 * setup(void) - performs all ONE TIME setup for this test.
 * 	Exit the test program on receipt of unexpected signals.
 *	Create a temporary directory used to hold test directories created
 *	and change the directory to it.
 *	Verify that pid of process executing the test is root.
 *	Create a test directory on temporary directory and set the ownership
 *	of test directory to guest user and process, change mode permissions
 *	to set group id bit on it.
 */
void setup()
{

	/* Capture unexpected signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be super/root for this test!");
		tst_exit();
	}

	TEST_PAUSE;

	/* Make a temp dir and cd to it */
	tst_tmpdir();

	/* Save the real user id of the current test process */
	save_myuid = getuid();

	/* Save the process id of the current test process */
	mypid = getpid();

	/* Get the node name to be created in the test */
	sprintf(node_name, TNODE, mypid);

	/* Get the uid/gid of ltp user */
	if ((user1 = getpwnam(LTPUSER)) == NULL) {
		tst_brkm(TBROK, cleanup, "%s not in /etc/passwd", LTPUSER);
	}
	user1_uid = user1->pw_uid;
	group1_gid = user1->pw_gid;

	/* Get the effective group id of the test process */
	group2_gid = getegid();

	/*
	 * Create a test directory under temporary directory with the
	 * specified mode permissions, with uid/gid set to that of guest
	 * user and the test process.
	 */
	if (mkdir(DIR_TEMP, MODE_RWX) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(2) of %s failed", DIR_TEMP);
	}
	if (chown(DIR_TEMP, user1_uid, group2_gid) < 0) {
		tst_brkm(TBROK, cleanup, "chown(2) of %s failed", DIR_TEMP);
	}
	if (chmod(DIR_TEMP, MODE_SGID) < 0) {
		tst_brkm(TBROK, cleanup, "chmod(2) of %s failed", DIR_TEMP);
	}

	/*
	 * Verify that test directory created with expected permission modes
	 * and ownerships.
	 */
	if (stat(DIR_TEMP, &buf) < 0) {
		tst_brkm(TBROK, cleanup, "stat(2) of %s failed", DIR_TEMP);
	}
	/* Verify modes of test directory */
	if (!(buf.st_mode & S_ISGID)) {
		tst_brkm(TBROK, cleanup,
			 "%s: Incorrect modes, setgid bit not set", DIR_TEMP);
	}

	/* Verify group ID of test directory */
	if (buf.st_gid != group2_gid) {
		tst_brkm(TBROK, cleanup, "%s: Incorrect group", DIR_TEMP);
	}

	/* Change directory to DIR_TEMP */
	if (chdir(DIR_TEMP) < 0) {
		tst_brkm(TBROK, cleanup,
			 "Unable to change to %s directory", DIR_TEMP);
	}
}

/*
 * cleanup() - Performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	Print test timing stats and errno log if test executed with options.
 *	Remove temporary directory and sub-directories/files under it
 *	created during setup().
 *	Exit the test program with normal exit code.
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
