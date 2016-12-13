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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "test.h"
#include "safe_macros.h"

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	S_IRWXU | S_IRGRP | S_IWGRP| S_IROTH | S_IWOTH
#define LTPUSER1	"nobody"
#define LTPUSER2	"bin"

char *TCID = "utime03";
int TST_TOTAL = 1;
time_t curr_time;		/* current time in seconds */

struct passwd *ltpuser;		/* password struct for ltpusers */
uid_t user_uid;			/* user id of ltpuser */
gid_t group_gid;		/* group id of ltpuser */
int status;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* struct buffer to hold file info. */
	int lc;
	long type;
	time_t modf_time, access_time;
	time_t pres_time;	/* file modification/access/present time */
	pid_t pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	switch ((type = tst_fs_type(cleanup, "."))) {
	case TST_NFS_MAGIC:
		if (tst_kvercmp(2, 6, 18) < 0)
			tst_brkm(TCONF, cleanup, "Cannot do utime on a file"
				" on %s filesystem before 2.6.18",
				 tst_fs_type_name(type));
		break;
	case TST_V9FS_MAGIC:
		tst_brkm(TCONF, cleanup,
			 "Cannot do utime on a file on %s filesystem",
			 tst_fs_type_name(type));
		break;
	}

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

			tst_count = 0;

			/*
			 * Invoke utime(2) to set TEMP_FILE access and
			 * modification times to the current time.
			 */
			TEST(utime(TEMP_FILE, NULL));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL|TTERRNO,
					 "utime(%s) failed", TEMP_FILE);
			} else {
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
				pres_time = time(NULL);

				/*
				 * Get the modification and access
				 * times of temporary file using
				 * stat(2).
				 */
				SAFE_STAT(cleanup, TEMP_FILE, &stat_buf);
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
			}
			tst_count++;	/* incr. TEST_LOOP counter */
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
void setup(void)
{
	int fildes;		/* file handle for temp file */
	char *tmpd = NULL;

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	tst_tmpdir();

	/* get the name of the temporary directory */
	tmpd = SAFE_GETCWD(NULL, tmpd, 0);

	/* Creat a temporary file under above directory */
	fildes = SAFE_CREAT(cleanup, TEMP_FILE, FILE_MODE);

	/* Close the temporary file created */
	SAFE_CLOSE(cleanup, fildes);

	/*
	 * Make sure that specified Mode permissions set as
	 * umask value may be different.
	 */
	SAFE_CHMOD(cleanup, TEMP_FILE, FILE_MODE);
	SAFE_CHMOD(cleanup, tmpd, 0711);

	ltpuser = SAFE_GETPWNAM(cleanup, LTPUSER2);

	/* get uid/gid of user accordingly */
	user_uid = ltpuser->pw_uid;
	group_gid = ltpuser->pw_gid;

	/*
	 * Change the ownership of test directory/file specified by
	 * pathname to that of user_uid and group_gid.
	 */
	SAFE_CHOWN(cleanup, TEMP_FILE, user_uid, group_gid);

	/* Get the current time */
	curr_time = time(NULL);

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
void cleanup(void)
{
	seteuid(0);

	tst_rmdir();

}
