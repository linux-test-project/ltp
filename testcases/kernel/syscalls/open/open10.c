/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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
 *	open10.c - Verifies that the group ID and setgid bit are
 *		   set correctly when a new file is created using open.
 *		  (ported from SPIE, section2/iosuite/open5.c, by
 *		   Airong Zhang)
 *
 * CALLS
 *	open
 *
 * ALGORITHM
 *	Create two directories, one with the group ID of this process
 *	and the setgid bit not set, and the other with a group ID
 *	other than that of this process and with the setgid bit set.
 *	In each directory, create a file with and without the setgid
 *	bit set in the creation modes. Verify that the modes and group
 *	ID are correct on each of the 4 files.
 *	As root, create a file with the setgid bit on in the
 *	directory with the setgid bit.
 *	This tests the SVID3 create group semantics.
 *
 * USAGE
 *	open10
 *
 * RESTRICTIONS
 *
 */

#include <stdio.h>		/* needed by testhead.h         */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include "usctest.h"
#include "test.h"

char *TCID = "open10";
int TST_TOTAL = 1;
int local_flag;

#define PASSED 1
#define FAILED 0

int local_flag;

#define MODE_RWX        S_IRWXU | S_IRWXG | S_IRWXO
#define MODE_SGID       S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO
#define DIR_A_TEMP	"open10.testdir.A.%d"
#define DIR_B_TEMP	"open10.testdir.B.%d"
#define SETGID		"setgid"
#define NOSETGID	"nosetgid"
#define ROOT_SETGID	"root_setgid"
#define	MSGSIZE		150

void cleanup(void);

char progname[] = "open10()";

int issu()
{

	int uid;

	uid = (-1);
	uid = geteuid();

	if (uid == (-1)) {
		fprintf(stderr, "geteuid: failed in issu()");
		return (-1);
	}

	if (uid == 0) {
		return 0;
	} else {
		fprintf(stderr, "*** NOT SUPERUSER must be root  %s\n",
			progname);
		return (uid);
	}
	return 0;
}

/*--------------------------------------------------------------*/
int main(int ac, char *av[])
{
	int ret;
	struct stat buf;
	struct group *group;
	struct passwd *user1;
	char DIR_A[MSGSIZE], DIR_B[MSGSIZE];
	char setgid_A[MSGSIZE], nosetgid_A[MSGSIZE];
	char setgid_B[MSGSIZE], nosetgid_B[MSGSIZE], root_setgid_B[MSGSIZE];
	gid_t group1_gid, group2_gid, mygid;
	uid_t save_myuid, user1_uid;
	pid_t mypid;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int fail_count = 0;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	 }

	tst_tmpdir();

	if (issu() != 0) {
		tst_resm(TINFO, "Must be root to run this test.");
		cleanup();
	}
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		local_flag = PASSED;

		save_myuid = getuid();
		mypid = getpid();
		sprintf(DIR_A, DIR_A_TEMP, mypid);
		sprintf(DIR_B, DIR_B_TEMP, mypid);
		sprintf(setgid_A, "%s/%s", DIR_A, SETGID);
		sprintf(nosetgid_A, "%s/%s", DIR_A, NOSETGID);
		sprintf(setgid_B, "%s/%s", DIR_B, SETGID);
		sprintf(nosetgid_B, "%s/%s", DIR_B, NOSETGID);
		sprintf(root_setgid_B, "%s/%s", DIR_B, ROOT_SETGID);

		/* Get the uid of user1 */
		if ((user1 = getpwnam("nobody")) == NULL) {
			tst_brkm(TBROK, cleanup, "nobody not in /etc/passwd");
		}
		user1_uid = user1->pw_uid;

		/*
		 * Get the group IDs of group1 and group2.
		 */
		if ((group = getgrnam("nobody")) == NULL) {
			if ((group = getgrnam("nogroup")) == NULL) {
				tst_brkm(TBROK, cleanup,
					 "nobody/nogroup not in /etc/group");
			}
		}
		group1_gid = group->gr_gid;
		if ((group = getgrnam("bin")) == NULL) {
			tst_brkm(TBROK, cleanup, "bin not in /etc/group");
		}
		group2_gid = group->gr_gid;

		/*--------------------------------------------------------------*/
		/* Block0: Set up the parent directories                        */
		/*--------------------------------------------------------------*/
		//block0:

		/*
		 * Create a directory with group id the same as this process
		 * and with no setgid bit.
		 */
		if ((ret = mkdir(DIR_A, MODE_RWX)) < 0) {
			tst_resm(TFAIL|TERRNO, "mkdir(%s) failed", DIR_A);
			local_flag = FAILED;
		}

		if ((ret = chown(DIR_A, user1_uid, group2_gid)) < 0) {
			tst_resm(TFAIL|TERRNO, "chown(%s) failed", DIR_A);
			local_flag = FAILED;
		}

		if ((ret = stat(DIR_A, &buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "stat(%s) failed", DIR_A);
			local_flag = FAILED;
		}

		/* Verify modes */
		if (buf.st_mode & S_ISGID) {
			tst_resm(TFAIL, "%s: Incorrect modes, setgid bit set",
				 DIR_A);
			local_flag = FAILED;
		}

		/* Verify group ID */
		if (buf.st_gid != group2_gid) {
			tst_resm(TFAIL, "%s: Incorrect group (got %d and %d)",
				 DIR_A, buf.st_gid, group2_gid);
			local_flag = FAILED;
		}

		/*
		 * Create a directory with group id different from that of
		 * this process and with the setgid bit set.
		 */
		if ((ret = mkdir(DIR_B, MODE_RWX)) < 0) {
			tst_resm(TFAIL|TERRNO, "mkdir(%s) failed", DIR_B);
			local_flag = FAILED;
		}

		if ((ret = chown(DIR_B, user1_uid, group2_gid)) < 0) {
			tst_resm(TFAIL|TERRNO, "chown(%s) failed", DIR_B);
			local_flag = FAILED;
		}

		if ((ret = chmod(DIR_B, MODE_SGID)) < 0) {
			tst_resm(TFAIL|TERRNO, "chmod(%s) failed", DIR_B);
			local_flag = FAILED;
		}

		if ((ret = stat(DIR_B, &buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "stat(%s) failed", DIR_B);
			local_flag = FAILED;
		}

		/* Verify modes */
		if (!(buf.st_mode & S_ISGID)) {
			tst_resm(TFAIL,
				 "%s: Incorrect modes, setgid bit not set",
				 DIR_B);
			local_flag = FAILED;
		}

		/* Verify group ID */
		if (buf.st_gid != group2_gid) {
			tst_resm(TFAIL, "%s: Incorrect group (got %d and %d)",
				 DIR_B, buf.st_gid, group2_gid);
			local_flag = FAILED;
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block0.");
		} else {
			tst_resm(TFAIL, "Test failed in block0.");
			fail_count++;
		}

		local_flag = PASSED;

	/*--------------------------------------------------------------*/
		/* Block1: Create two files in testdir.A, one with the setgid   */
		/*         bit set in the creation modes and the other without. */
		/*         Both should inherit the group ID of the process and  */
		/*         maintain the setgid bit as specified in the creation */
		/*         modes.                                               */
	/*--------------------------------------------------------------*/
		//block1:

		/*
		 * Now become user1, group1
		 */
		if ((ret = setgid(group1_gid)) < 0) {
			tst_resm(TINFO, "Unable to set process group ID to group1");
		}

		if ((ret = setreuid(-1, user1_uid)) < 0) {
			tst_resm(TINFO, "Unable to set process uid to user1");
		}
		mygid = getgid();

		/*
		 * Create the file with setgid not set
		 */
		ret = open(nosetgid_A, O_CREAT | O_EXCL | O_RDWR, MODE_RWX);
		if (ret < 0) {
			tst_resm(TFAIL|TERRNO, "open(%s) failed", nosetgid_A);
			local_flag = FAILED;
		}
		close(ret);

		if ((ret = stat(nosetgid_A, &buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "stat(%s) failed", nosetgid_A);
			local_flag = FAILED;
		}

		/* Verify modes */
		if (buf.st_mode & S_ISGID) {
			tst_resm(TFAIL, "%s: Incorrect modes, setgid bit set",
				 nosetgid_A);
			local_flag = FAILED;
		}

		/* Verify group ID */
		if (buf.st_gid != mygid) {
			tst_resm(TFAIL, "%s: Incorrect group (got %d and %d)",
				 nosetgid_A, buf.st_gid, mygid);
			local_flag = FAILED;
		}

		/*
		 * Create the file with setgid set
		 */
		ret = open(setgid_A, O_CREAT | O_EXCL | O_RDWR, MODE_SGID);
		if (ret < 0) {
			tst_resm(TFAIL|TERRNO, "open(%s) failed", setgid_A);
			local_flag = FAILED;
		}
		close(ret);

		if ((ret = stat(setgid_A, &buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "stat(%s) failed", setgid_A);
			local_flag = FAILED;
		}

		/* Verify modes */
		if (!(buf.st_mode & S_ISGID)) {
			tst_resm(TFAIL,
				 "%s: Incorrect modes, setgid bit not set",
				 setgid_A);
			local_flag = FAILED;
		}

		/* Verify group ID */
		if (buf.st_gid != mygid) {
			tst_resm(TFAIL, "%s: Incorrect group (%d and %d)",
				 setgid_A, buf.st_gid, mygid);
			local_flag = FAILED;
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block1.");
		} else {
			tst_resm(TFAIL, "Test failed in block1.");
			fail_count++;
		}

		local_flag = PASSED;

	/*--------------------------------------------------------------*/
		/* Block2: Create two files in testdir.B, one with the setgid   */
		/*         bit set in the creation modes and the other without. */
		/*         Both should inherit the group ID of the parent       */
		/*         directory, group2. Either file should have the segid */
		/*         bit set in the modes.                                */
	/*--------------------------------------------------------------*/
		//block2:

		/*
		 * Create the file with setgid not set
		 */
		ret = open(nosetgid_B, O_CREAT | O_EXCL | O_RDWR, MODE_RWX);
		if (ret < 0) {
			tst_resm(TFAIL|TERRNO, "open(%s) failed", nosetgid_B);
			local_flag = FAILED;
		}
		close(ret);

		if ((ret = stat(nosetgid_B, &buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "stat(%s) failed", nosetgid_B);
			local_flag = FAILED;
		}

		/* Verify modes */
		if (buf.st_mode & S_ISGID) {
			tst_resm(TFAIL,
				 "%s: Incorrect modes, setgid bit should be set",
				 nosetgid_B);
			local_flag = FAILED;
		}

		/* Verify group ID */
		if (buf.st_gid != group2_gid) {
			tst_resm(TFAIL, "%s: Incorrect group (got %d and %d)",
				 nosetgid_B, buf.st_gid, group2_gid);
			local_flag = FAILED;
		}

		/*
		 * Create the file with setgid set
		 */
		ret = open(setgid_B, O_CREAT | O_EXCL | O_RDWR, MODE_SGID);
		if (ret < 0) {
			tst_resm(TFAIL|TERRNO, "open(%s) failed", setgid_B);
			local_flag = FAILED;
		}
		close(ret);

		if ((ret = stat(setgid_B, &buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "stat(%s) failed", setgid_B);
			local_flag = FAILED;
		}

		/* Verify group ID */
		if (buf.st_gid != group2_gid) {
			tst_resm(TFAIL, "%s: Incorrect group (got %d and %d)",
				 setgid_B, buf.st_gid, group2_gid);
			local_flag = FAILED;
		}

		/* Verify modes */
		if (!(buf.st_mode & S_ISGID)) {
			tst_resm(TFAIL,
				 "%s: Incorrect modes, setgid bit not set",
				 setgid_B);
			local_flag = FAILED;
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block2.");
		} else {
			tst_resm(TFAIL, "Test failed in block2.");
			fail_count++;
		}

		local_flag = PASSED;

	/*--------------------------------------------------------------*/
		/* Block3: Create a file in testdir.B, with the setgid bit set  */
		/*         in the creation modes and do so as root. The file    */
		/*         should inherit the group ID of the parent directory, */
		/*         group2 and should have the setgid bit set.           */
	/*--------------------------------------------------------------*/
		//block3:

		/* Become root again */
		if ((ret = setreuid(-1, save_myuid)) < 0) {
			tst_resm(TFAIL|TERRNO, "Changing back to root failed");
			local_flag = FAILED;
		}

		/* Create the file with setgid set */
		if ((ret = open(root_setgid_B, O_CREAT | O_EXCL | O_RDWR,
				MODE_SGID)) < 0) {
			tst_resm(TFAIL|TERRNO, "open(%s) failed", root_setgid_B);
			local_flag = FAILED;
		}
		close(ret);

		if ((ret = stat(root_setgid_B, &buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "stat(%s) failed", root_setgid_B);
			local_flag = FAILED;
		}

		/* Verify modes */
		if (!(buf.st_mode & S_ISGID)) {
			tst_resm(TFAIL,
				 "%s: Incorrect modes, setgid bit not set",
				 root_setgid_B);
			local_flag = FAILED;
		}

		/* Verify group ID */
		if (buf.st_gid != group2_gid) {
			tst_resm(TFAIL, "%s: Incorrect group (got %d and %d)",
				 root_setgid_B, buf.st_gid, group2_gid);
			local_flag = FAILED;
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block3.");
		} else {
			tst_resm(TFAIL, "Test failed in block3.");
			fail_count++;
		}

	/*--------------------------------------------------------------*/
		/* Clean up any files created by test before call to anyfail.   */
		/* Remove the directories.                                      */
	/*--------------------------------------------------------------*/
		if ((ret = unlink(setgid_A)) < 0)
			tst_resm(TWARN|TERRNO, "unlink(%s) failed", setgid_A);
		if ((ret = unlink(nosetgid_A)) < 0)
			tst_resm(TWARN|TERRNO, "unlink(%s) failed", nosetgid_A);
		if ((ret = rmdir(DIR_A)) < 0)
			tst_resm(TWARN|TERRNO, "rmdir(%s) failed", DIR_A);

		if ((ret = unlink(setgid_B)) < 0)
			tst_resm(TWARN|TERRNO, "unlink(%s) failed", setgid_B);
		if ((ret = unlink(root_setgid_B)) < 0)
			tst_resm(TWARN|TERRNO, "unlink(%s) failed", root_setgid_B);
		if ((ret = unlink(nosetgid_B)) < 0)
			tst_resm(TWARN|TERRNO, "unlink(%s) failed", nosetgid_B);
		if ((ret = rmdir(DIR_B)) < 0)
			tst_resm(TWARN|TERRNO, "rmdir(%s) failed", DIR_B);

		if (fail_count == 0) {
			tst_resm(TPASS, "Test passed.");
		} else {
			tst_resm(TFAIL,
				 "Test failed because of above failures.");
		}

	}
	cleanup();
	tst_exit();
}

void cleanup(void)
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	tst_rmdir();

}
