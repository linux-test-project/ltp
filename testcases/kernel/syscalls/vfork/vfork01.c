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
 * Name: vfork01
 *
 * Test Description:
 *  Fork a process using vfork() and verify that, the attribute values like
 *  euid, ruid, suid, egid, rgid, sgid, umask, inode and device number of
 *  root and current working directories are same as that of the parent
 *  process.
 * $
 * Expected Result:
 *  The attribute values like euid, ruid, suid, egid, rgid, sgid, umask, inode
 *  and device number of root and current working directory of the parent and
 *  child processes should be equal.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
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
 *
 * Usage:  <for command-line>
 *  vfork01 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,	-c n : Run n copies concurrently.
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
 *  None.
 *
 */

#define _GNU_SOURCE 1
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "test.h"

char *TCID = "vfork01";
int TST_TOTAL = 1;

/* Variables to hold parent/child eff/real/saved uid/gid values */
uid_t Peuid, Ceuid, Csuid, Psuid, Pruid, Cruid;
gid_t Pegid, Cegid, Psgid, Csgid, Prgid, Crgid;
mode_t Pumask, Cumask;

char *Pcwd, *Ccwd;		/*
				 * pathname of working directory of
				 * child/parent process.
				 */
/* stat structure to hold directory/inode information for parent/child */
struct stat StatPbuf;
struct stat StatCbuf;
struct stat Stat_cwd_Pbuf;
struct stat Stat_cwd_Cbuf;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;
	pid_t cpid;		/* process id of the child process */
	int exit_status;	/* exit status of child process */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call vfork(2) to create a child process without
		 * fully copying the address space of parent.
		 */
		TEST(vfork());

		if ((cpid = TEST_RETURN) == -1) {
			tst_resm(TFAIL, "vfork() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else if (cpid == 0) {	/* Child process */
			/*
			 * Get the euid, ruid, egid, rgid, umask value
			 * and the current working directory of the
			 * child process
			 */
			if (getresuid(&Cruid, &Ceuid, &Csuid) < 0) {
				tst_resm(TFAIL, "getresuid() fails to "
					 "get real/eff./saved uid of "
					 "child process");
				_exit(1);
			}

			if (getresgid(&Crgid, &Cegid, &Csgid) < 0) {
				tst_resm(TFAIL, "getresgid() fails to "
					 "get real/eff./saved gid of "
					 "child process");
				_exit(1);
			}

			/*
			 * Get the file mode creation mask value of
			 * child process by setting value zero and
			 * restore the previous mask value.
			 */
			Cumask = umask(0);

			/*
			 * Restore the process mask of child to
			 * previous value.
			 */
			umask(Cumask);

			/*
			 * Get the pathname of current working
			 * directory for the child process.
			 */
			if ((Ccwd = (char *)getcwd(NULL,
						   BUFSIZ)) == NULL) {
				tst_resm(TFAIL, "getcwd failed for the "
					 "child process");
				_exit(1);
			}

			/*
			 * Get the device number and the inode
			 * number of "/" directory for the child
			 * process.
			 */
			if (stat("/", &StatCbuf) < 0) {
				tst_resm(TFAIL, "stat(2) failed to get "
					 "info. of'/' in the child "
					 "process");
				_exit(1);
			}

			/*
			 * Get the device/inode number of "."
			 * (working directory) for the child process.
			 */
			if (stat(Ccwd, &Stat_cwd_Cbuf) < 0) {
				tst_resm(TFAIL, "stat(2) failed to get "
					 "info. of working irectory in "
					 "the child");
				_exit(1);
			}

			/* Now, do the actual comparision */
			if (Peuid != Ceuid || Pegid != Cegid ||
			    Psuid != Csuid || Psgid != Csgid ||
			    Pruid != Cruid || Prgid != Crgid ||
			    Pumask != Cumask) {
				tst_resm(TFAIL, "Attribute values of "
					 "parent and child don't match");
				_exit(1);
			} else {
				tst_resm(TINFO, "Attribute values of "
					 "parent and child match");
			}

			/* Check for the same working directories */
			if (strcmp(Pcwd, Ccwd) != 0) {
				tst_resm(TFAIL, "Working directories "
					 "of parent and child don't "
					 "match");
				_exit(1);
			} else {
				tst_resm(TINFO, "Working directories "
					 "of parent and child match");
			}

			/*
			 * Check for the same device/inode number of
			 * '/' directory.
			 */
			if ((StatPbuf.st_ino != StatCbuf.st_ino) ||
			    (StatPbuf.st_dev != StatCbuf.st_dev)) {
				tst_resm(TFAIL, "Device/inode number "
					 "of parent and childs '/' "
					 " don't match");
				_exit(1);
			} else {
				tst_resm(TINFO, "Device/inode number "
					 "of parent and childs '/' "
					 "match");
			}

			/*
			 * Check for the same device and inode number
			 *  of "." (current working directory.
			 */
			if ((Stat_cwd_Pbuf.st_ino !=
			     Stat_cwd_Cbuf.st_ino) ||
			    (Stat_cwd_Pbuf.st_dev !=
			     Stat_cwd_Cbuf.st_dev)) {
				tst_resm(TFAIL, "Device/inode number "
					 "of parent and childs '.' "
					 "don't match");
				_exit(1);
			} else {
				tst_resm(TINFO, "Device/inode number "
					 "of parent and childs '.' "
					 "don't match");
			}

			/*
			 * Exit with normal exit code if everything
			 * fine
			 */
			_exit(0);

		} else {	/* parent process */
			/*
			 * Let the parent process wait till child completes
			 * its execution.
			 */
			wait(&exit_status);

			/* Check for the exit status of child process */
			if (WEXITSTATUS(exit_status) == 0) {
				tst_resm(TPASS, "Call of vfork() successful");
			} else if (WEXITSTATUS(exit_status) == 1) {
				tst_resm(TFAIL,
					 "Child process exited abnormally");
			}
		}
		tst_count++;	/* incr. TEST_LOOP counter */
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  This function gets real/effective/saved uid/gid, umask, the device/inode
 *  number of '/' and current working directory for the parent process.
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/*
	 * Get the euid, ruid, egid, rgid, umask value
	 * and the current working directory of the parent process.
	 */
	if (getresuid(&Pruid, &Peuid, &Psuid) < 0) {
		tst_brkm(TFAIL, cleanup, "getresuid() fails to get "
			 "real/eff./saved uid of parent");
	}

	if (getresgid(&Prgid, &Pegid, &Psgid) < 0) {
		tst_brkm(TFAIL, cleanup, "getresgid() fails to get "
			 "real/eff./saved gid of parent");
	}

	/* Get the process file mode creation mask by setting value 0 */
	Pumask = umask(0);
	umask(Pumask);		/*
				 * Restore the mask value of the
				 * process.
				 */
	/*
	 * Get the pathname of current working directory of the parent
	 * process.
	 */
	if ((Pcwd = (char *)getcwd(NULL, BUFSIZ)) == NULL) {
		tst_brkm(TFAIL, cleanup,
			 "getcwd failed for the parent process");
	}

	/*
	 * Get the device and inode number of root directory for the
	 * parent process.
	 */
	if (stat("/", &StatPbuf) == -1) {
		tst_brkm(TFAIL, cleanup, "stat(2) failed to get info. of '/' "
			 "in parent process");
	}

	/*
	 * Get the device number and the inode number of "." (current-
	 * working directory) for the parent process.
	 */
	if (stat(Pcwd, &Stat_cwd_Pbuf) < 0) {
		tst_brkm(TFAIL, cleanup, "stat(2) failed to get info. of "
			 "working directory in parent process");
	}
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup(void)
{

}
