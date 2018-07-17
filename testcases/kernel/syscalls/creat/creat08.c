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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	creat08.c - Verifies that the group ID and setgid bit are
 *		   set correctly when a new file is created.
 *		   (ported from SPIE, section2/iosuite/creat5.c,
 *		    by Airong Zhang <zhanga@us.ibm.com>)
 * CALLS
 *	creat
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
 *	creat08
 *
 * RESTRICTIONS
 *
 */

#include <stdio.h>		/* needed by testhead.h         */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "creat08";
int TST_TOTAL = 1;
int local_flag;

#define PASSED 1
#define FAILED 0

#define MODE_RWX        (S_IRWXU|S_IRWXG|S_IRWXO)
#define MODE_SGID       (S_ISGID|S_IRWXU|S_IRWXG|S_IRWXO)
#define DIR_A_TEMP	"testdir.A.%d"
#define DIR_B_TEMP	"testdir.B.%d"
#define SETGID		"setgid"
#define NOSETGID	"nosetgid"
#define ROOT_SETGID	"root_setgid"
#define	MSGSIZE		150

static void tst_cleanup(void);
static void cleanup(void);
static void setup(void);

static char DIR_A[MSGSIZE], DIR_B[MSGSIZE];
static char setgid_A[MSGSIZE], nosetgid_A[MSGSIZE];
static char setgid_B[MSGSIZE], nosetgid_B[MSGSIZE], root_setgid_B[MSGSIZE];

int main(int ac, char **av)
{
	struct stat buf;
	struct group *group;
	struct passwd *user1;
	gid_t group1_gid, group2_gid, mygid;
	uid_t save_myuid, user1_uid;
	pid_t mypid;

	int fd;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

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
			tst_brkm(TBROK | TERRNO, NULL,
				 "getpwnam(\"nobody\") failed");
		}

		user1_uid = user1->pw_uid;

		/*
		 * Get the group IDs of group1 and group2.
		 */
		if ((group = getgrnam("nobody")) == NULL) {
			if ((group = getgrnam("nogroup")) == NULL) {
				tst_brkm(TBROK | TERRNO, cleanup,
					 "getgrnam(\"nobody\") and "
					 "getgrnam(\"nogroup\") failed");
			}
		}
		group1_gid = group->gr_gid;
		if ((group = getgrnam("bin")) == NULL) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "getgrnam(\"bin\") failed");
		}
		group2_gid = group->gr_gid;

/*--------------------------------------------------------------*/
/* Block0: Set up the parent directories			*/
/*--------------------------------------------------------------*/
		/*
		 * Create a directory with group id the same as this process
		 * and with no setgid bit.
		 */
		if (mkdir(DIR_A, MODE_RWX) == -1) {
			tst_resm(TFAIL, "Creation of %s failed", DIR_A);
			local_flag = FAILED;
		}

		if (chown(DIR_A, user1_uid, group2_gid) == -1) {
			tst_resm(TFAIL, "Chown of %s failed", DIR_A);
			local_flag = FAILED;
		}

		if (stat(DIR_A, &buf) == -1) {
			tst_resm(TFAIL, "Stat of %s failed", DIR_A);
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
			tst_resm(TFAIL, "%s: Incorrect group", DIR_A);
			tst_resm(TINFO, "got %u and %u", buf.st_gid,
				 group2_gid);
			local_flag = FAILED;
		}

		/*
		 * Create a directory with group id different from that of
		 * this process and with the setgid bit set.
		 */
		if (mkdir(DIR_B, MODE_RWX) == -1) {
			tst_resm(TFAIL, "Creation of %s failed", DIR_B);
			local_flag = FAILED;
		}

		if (chown(DIR_B, user1_uid, group2_gid) == -1) {
			tst_resm(TFAIL, "Chown of %s failed", DIR_B);
			local_flag = FAILED;
		}

		if (chmod(DIR_B, MODE_SGID) == -1) {
			tst_resm(TFAIL, "Chmod of %s failed", DIR_B);
			local_flag = FAILED;
		}

		if (stat(DIR_B, &buf) == -1) {
			tst_resm(TFAIL, "Stat of %s failed", DIR_B);
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
			tst_resm(TFAIL, "%s: Incorrect group", DIR_B);
			tst_resm(TINFO, "got %u and %u", buf.st_gid,
				 group2_gid);
			local_flag = FAILED;
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block0.");
		} else {
			tst_resm(TFAIL, "Test failed in block0.");
		}

		local_flag = PASSED;

/*--------------------------------------------------------------*/
/* Block1: Create two files in testdir.A, one with the setgid   */
/*         bit set in the creation modes and the other without. */
/*	   Both should inherit the group ID of the process and  */
/*	   maintain the setgid bit as specified in the creation */
/*	   modes.                                               */
/*--------------------------------------------------------------*/
		/*
		 * Now become user1, group1
		 */
		if (setgid(group1_gid) == -1) {
			tst_resm(TINFO,
				 "Unable to set process group ID to group1");
		}

		if (setreuid(-1, user1_uid) == -1) {
			tst_resm(TINFO, "Unable to set process uid to user1");
		}
		mygid = getgid();

		/*
		 * Create the file with setgid not set
		 */
		fd = open(nosetgid_A, O_CREAT | O_EXCL | O_RDWR, MODE_RWX);
		if (fd == -1) {
			tst_resm(TFAIL, "Creation of %s failed", nosetgid_A);
			local_flag = FAILED;
		}

		if (stat(nosetgid_A, &buf) == -1) {
			tst_resm(TFAIL, "Stat of %s failed", nosetgid_A);
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
			tst_resm(TFAIL, "%s: Incorrect group", nosetgid_A);
			local_flag = FAILED;
		}
		close(fd);

		/*
		 * Create the file with setgid set
		 */
		fd = open(setgid_A, O_CREAT | O_EXCL | O_RDWR, MODE_SGID);
		if (fd == -1) {
			tst_resm(TFAIL, "Creation of %s failed", setgid_A);
			local_flag = FAILED;
		}

		if (stat(setgid_A, &buf) == -1) {
			tst_resm(TFAIL, "Stat of %s failed", setgid_A);
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
			tst_resm(TFAIL, "%s: Incorrect group", setgid_A);
			tst_resm(TINFO, "got %u and %u", buf.st_gid, mygid);
			local_flag = FAILED;
		}
		close(fd);

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block1.");
		} else {
			tst_resm(TFAIL, "Test failed in block1.");
		}

		local_flag = PASSED;

/*--------------------------------------------------------------*/
/* Block2: Create two files in testdir.B, one with the setgid   */
/*         bit set in the creation modes and the other without. */
/*	   Both should inherit the group ID of the parent       */
/*	   directory, group2.                                   */
/*--------------------------------------------------------------*/
		/*
		 * Create the file with setgid not set
		 */
		fd = creat(nosetgid_B, MODE_RWX);
		if (fd == -1) {
			tst_resm(TFAIL, "Creation of %s failed", nosetgid_B);
			local_flag = FAILED;
		}

		if (stat(nosetgid_B, &buf) == -1) {
			tst_resm(TFAIL, "Stat of %s failed", nosetgid_B);
			local_flag = FAILED;
		}

		/* Verify modes */
		if (buf.st_mode & S_ISGID) {
			tst_resm(TFAIL,
				 "%s: Incorrect modes, setgid bit should not be set",
				 nosetgid_B);
			local_flag = FAILED;
		}

		/* Verify group ID */
		if (buf.st_gid != group2_gid) {
			tst_resm(TFAIL, "%s: Incorrect group", nosetgid_B);
			local_flag = FAILED;
		}
		close(fd);

		/*
		 * Create the file with setgid set
		 */
		fd = creat(setgid_B, MODE_SGID);
		if (fd == -1) {
			tst_resm(TFAIL, "Creation of %s failed", setgid_B);
			local_flag = FAILED;
		}

		if (stat(setgid_B, &buf) == -1) {
			tst_resm(TFAIL, "Stat of %s failed", setgid_B);
			local_flag = FAILED;
		}

		/* Verify group ID */
		if (buf.st_gid != group2_gid) {
			tst_resm(TFAIL, "%s: Incorrect group", setgid_B);
			tst_resm(TFAIL, "got %u and %u", buf.st_gid,
				 group2_gid);
			local_flag = FAILED;
		}

		/*
		 * Skip S_ISGID check
		 * 0fa3ecd87848 ("Fix up non-directory creation in SGID directories")
		 * clears S_ISGID for files created by non-group members
		 */

		close(fd);

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block2.");
		} else {
			tst_resm(TFAIL, "Test failed in block2.");
		}

		local_flag = PASSED;
/*--------------------------------------------------------------*/
/* Block3: Create a file in testdir.B, with the setgid bit set  */
/*	   in the creation modes and do so as root. The file    */
/*	   should inherit the group ID of the parent directory, */
/*	   group2 and should have the setgid bit set.		*/
/*--------------------------------------------------------------*/
		/* Become root again */
		if (setreuid(-1, save_myuid) == -1) {
			tst_resm(TFAIL | TERRNO,
				 "Changing back to root failed");
			local_flag = FAILED;
		}

		/* Create the file with setgid set */
		fd = creat(root_setgid_B, MODE_SGID);
		if (fd == -1) {
			tst_resm(TFAIL, "Creation of %s failed", root_setgid_B);
			local_flag = FAILED;
		}

		if (stat(root_setgid_B, &buf) == -1) {
			tst_resm(TFAIL, "Stat of %s failed", root_setgid_B);
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
			tst_resm(TFAIL, "%s: Incorrect group", root_setgid_B);
			tst_resm(TINFO, "got %u and %u", buf.st_gid,
				 group2_gid);
			local_flag = FAILED;
		}
		close(fd);

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block3");
		} else {
			tst_resm(TFAIL, "Test failed in block3");
		}
		tst_cleanup();
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();
	tst_tmpdir();
}

static void tst_cleanup(void)
{
	if (unlink(setgid_A) == -1) {
		tst_resm(TBROK, "%s failed", setgid_A);
	}
	if (unlink(nosetgid_A) == -1) {
		tst_resm(TBROK, "unlink %s failed", nosetgid_A);
	}
	SAFE_RMDIR(NULL, DIR_A);
	SAFE_UNLINK(NULL, setgid_B);
	SAFE_UNLINK(NULL, root_setgid_B);
	SAFE_UNLINK(NULL, nosetgid_B);
	SAFE_RMDIR(NULL, DIR_B);
}

static void cleanup(void)
{
	tst_rmdir();
}
