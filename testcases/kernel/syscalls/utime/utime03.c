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
 * Test Name: utime03
 *
 * Test Description:
 *  Verify that the system call utime() successfully sets the modification
 *  and access times of a file to the current time, under the following
 *  constraints,
 *	- The times argument is null.
 *	- The user ID of the process is not "root".
 *	- The file is not owned by the user ID of the process.
 *	- The user ID of the process has write access to the file.
 *
 * Expected Result:
 *  utime succeeds returning zero and sets the access and modificatio
 *  times of the file to the current time.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  utime03 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
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
 * Restrictions:
 *  This test should be run by root only.
 *  nobody and bin must be valid users.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "test.h"
#include "usctest.h"

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	S_IRWXU | S_IRGRP | S_IWGRP| S_IROTH | S_IWOTH
#define LTPUSER1	"nobody"
#define LTPUSER2	"bin"

char *TCID = "utime03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
time_t curr_time;		/* current time in seconds */
time_t tloc;			/* argument var. for time() */
int exp_enos[] = { 0 };

struct passwd *ltpuser;		/* password struct for ltpusers */
uid_t user_uid;			/* user id of ltpuser */
gid_t group_gid;		/* group id of ltpuser */
int status;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* struct buffer to hold file info. */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	time_t modf_time, access_time;
	time_t pres_time;	/* file modification/access/present time */
	pid_t pid;

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

	/*
	 * check if the current filesystem is nfs
	 */
	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do utime on a file located on an NFS filesystem");
	}

        if (tst_is_cwd_v9fs()) {
                tst_brkm(TCONF, cleanup,
                         "Cannot do utime on a file located on an 9P filesystem");
        }

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	pid = FORK_OR_VFORK();

	if (pid == -1) {
		tst_brkm(TBROK, cleanup, "fork() failed");
	} else if (pid == 0) {
		if ((ltpuser = getpwnam(LTPUSER1)) == NULL) {
			tst_brkm(TBROK, cleanup, "%s not found in /etc/passwd",
				 LTPUSER1);
		}

		/* get uid/gid of user accordingly */
		user_uid = ltpuser->pw_uid;

		seteuid(user_uid);

		for (lc = 0; TEST_LOOPING(lc); lc++) {

			Tst_count = 0;

			/*
			 * Invoke utime(2) to set TEMP_FILE access and
			 * modification times to the current time.
			 */
			TEST(utime(TEMP_FILE, NULL));

			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL,
					 "utime(%s) Failed, errno=%d : %s",
					 TEMP_FILE, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				/*
				 * Perform functional verification if test
				 * executed without (-f) option.
				 */
				if (STD_FUNCTIONAL_TEST) {
					/*
					 * Sleep for a second so that mod time
					 * and access times will be different
					 * from the current time.
					 */
					sleep(2);

					/*
					 * Get the current time now, after
					 * calling utime(2)
					 */
					if ((pres_time = time(&tloc)) < 0) {
						tst_brkm(TFAIL, cleanup,
							 "time() failed to get "
							 "present time after "
							 "utime, error=%d",
							 errno);
					 }

					/*
					 * Get the modification and access
					 * times of temporary file using
					 * stat(2).
					 */
					if (stat(TEMP_FILE, &stat_buf) < 0) {
						tst_brkm(TFAIL, cleanup,
							 "stat(2) of %s failed, "
							 "error:%d", TEMP_FILE,
							 TEST_ERRNO);
					 }
					modf_time = stat_buf.st_mtime;
					access_time = stat_buf.st_atime;

					/* Now do the actual verification */
					if (modf_time <= curr_time ||
					    modf_time >= pres_time ||
					    access_time <= curr_time ||
					    access_time >= pres_time) {
						tst_resm(TFAIL, "%s access and "
							 "modification times "
							 "not set", TEMP_FILE);
					} else {
						tst_resm(TPASS, "Functionality "
							 "of utime(%s, NULL) "
							 "successful",
							 TEMP_FILE);
					}
				} else {
					tst_resm(TPASS, "%s call succeeded",
						 TCID);
				}
			}
			Tst_count++;	/* incr. TEST_LOOP counter */
		}
	} else {
		waitpid(pid, &status, 0);
		_exit(0);	/*
				 * Exit here and let the child clean up.
				 * This allows the errno information set
				 * by the TEST_ERROR_LOG macro and the
				 * PASS/FAIL status to be preserved for
				 * use during cleanup.
				 */
	}

	cleanup();
	tst_exit();

}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory and close it
 *  Change the ownership of testfile to that of "bin" user.
 *  Record the current time.
 */
void setup()
{
	int fildes;		/* file handle for temp file */
	char *tmpd = NULL;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Check that the test process id is not super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be root for this test!");
		tst_exit();
	}

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	tst_tmpdir();

	/* get the name of the temporary directory */
	if ((tmpd = getcwd(tmpd, 0)) == NULL) {
		tst_brkm(TBROK, NULL, "getcwd failed");
	}

	/* Creat a temporary file under above directory */
	if ((fildes = creat(TEMP_FILE, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "creat(%s, %#o) Failed, errno=%d :%s",
			 TEMP_FILE, FILE_MODE, errno, strerror(errno));
	 }

	/* Close the temporary file created */
	if (close(fildes) < 0) {
		tst_brkm(TBROK, cleanup,
			 "close(%s) Failed, errno=%d : %s:",
			 TEMP_FILE, errno, strerror(errno));
	 }

	/*
	 * Make sure that specified Mode permissions set as
	 * umask value may be different.
	 */
	if (chmod(TEMP_FILE, FILE_MODE) < 0) {
		tst_brkm(TBROK, cleanup,
			 "chmod(%s) Failed, errno=%d : %s:",
			 TEMP_FILE, errno, strerror(errno));
	 }

	if (chmod(tmpd, 0711) != 0) {
		tst_brkm(TBROK, cleanup, "chmod() failed");
	}

	if ((ltpuser = getpwnam(LTPUSER2)) == NULL) {
		tst_brkm(TBROK, cleanup, "%s not found in /etc/passwd",
			 LTPUSER2);
	 }

	/* get uid/gid of user accordingly */
	user_uid = ltpuser->pw_uid;
	group_gid = ltpuser->pw_gid;

	/*
	 * Change the ownership of test directory/file specified by
	 * pathname to that of user_uid and group_gid.
	 */
	if (chown(TEMP_FILE, user_uid, group_gid) < 0) {
		tst_brkm(TBROK, cleanup, "chown() of %s failed, error %d",
			 TEMP_FILE, errno);
	 }

	/* Get the current time */
	if ((curr_time = time(&tloc)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "time() failed to get current time, errno=%d", errno);
	 }

	/*
	 * Sleep for a second so that mod time and access times will be
	 * different from the current time
	 */
	sleep(2);		/* sleep(1) on IA64 sometimes sleeps < 1 sec!! */

}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	seteuid(0);
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();

}
