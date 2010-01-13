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
 *	mkdir02
 *
 * DESCRIPTION
 *	This test will verify that new directory created
 *	by mkdir(2) inherites the group ID from the parent
 *      directory and S_ISGID bit, if the S_ISGID bit is set
 *	in the parent directory.
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Pause for SIGUSR1 if option specified.
 *		Create temporary directory.
 *              Give write permission on temporary directory for all users.
 *              set up umask
 *
 *	Test:
 *		Loop if the proper options are given.
 *              fork the first child as ltpuser1
 *                  create a directory tstdir1 with S_ISGID set
 *              fork the second child as ltpuser2
 *                  create a directtory tstdir2 user tstdir1
 *                  check tstdir2's group ID and the S_ISGID bit
 *                  if they are the same as tstdir1's
 *                       PASS
 *                  else FAIL
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.*
 * USAGE
 *	mkdir02 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None.
 */

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
extern struct passwd *my_getpwnam(char *);

#define PERMS		0777

char *TCID = "mkdir02";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char tstdir1[100];
char tstdir2[100];

char user1name[] = "nobody";
char user2name[] = "bin";

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct stat buf, buf1;
	pid_t pid, pid1;
	struct passwd *ltpuser1;
	struct passwd *ltpuser2;
	int rval, status;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* check the inherited group ID */

		/*
		 * first, fork the first child, set to ltpuser1's uid and gid,
		 * create a directory, with S_ISGID bit set
		 */

		ltpuser1 = my_getpwnam(user1name);
		sprintf(tstdir1, "tstdir1.%d", getpid());

		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TFAIL, cleanup, "fork() failed");
			/* NOTREACHED */
		}

		if (pid == 0) {	/* first child */
			rval = setregid(ltpuser1->pw_gid, ltpuser1->pw_gid);
			if (rval < 0) {
				perror("setregid");
				tst_resm(TFAIL, "setregid failed to "
					 "to set the real gid to %d and "
					 "effective gid to %d",
					 ltpuser1->pw_gid, ltpuser1->pw_gid);
				exit(1);
				/* NOTREACHED */
			}
			/* being ltupuser1 */
			rval = setreuid(ltpuser1->pw_uid, ltpuser1->pw_uid);
			if (rval < 0) {
				perror("setreuid");
				tst_resm(TFAIL, "setreuid failed to "
					 "to set the real uid to %d and "
					 "effective uid to %d",
					 ltpuser1->pw_uid, ltpuser1->pw_uid);
				exit(1);
				/* NOTREACHED */
			}

			/*
			 * create a direcoty with S_ISGID bit set
			 * and the group ID is ltpuser1
			 */
			if (mkdir(tstdir1, PERMS) != 0) {
				perror("mkdir");
				tst_resm(TFAIL, "mkdir() failed to create"
					 " a directory with Set "
					 " group ID turn on ");
				exit(1);
				/* NOTREACHED */
			}
			if (stat(tstdir1, &buf1) == -1) {
				perror("stat");
				tst_resm(TFAIL,
					 "failed to stat the new directory"
					 "in mkdir()");
				exit(1);
				/* NOTREACHED */
			}
			if (chmod(tstdir1, buf1.st_mode | S_ISGID) != 0) {
				perror("chmod");
				tst_resm(TFAIL, "failed to set S_ISGID bit");
				exit(1);
				/* NOTREACHED */
			}

			/* Successfully create the parent directory */
			exit(0);
			/* NOTREACHED */
		}
		wait(&status);
		if (WEXITSTATUS(status) != 0) {
			tst_brkm(TFAIL, cleanup,
				 "Test to attempt to make a directory"
				 " inherits group ID FAILED ");
		}
		/*
		 * fork the second child process, set to ltpuser2's uid and gid
		 * create a sub directory under the directory
		 * just created by child process 1
		 * check the group ID of the sub directory
		 * should inherit from parent directory
		 */

		ltpuser2 = my_getpwnam(user2name);
		sprintf(tstdir2, "%s/tstdir2.%d", tstdir1, getpid());
		if ((pid1 = FORK_OR_VFORK()) < 0) {
			perror("fork failed");
			tst_brkm(TFAIL, cleanup, "fork() failed");
			/* NOTREACHED */
		} else if (pid1 == 0) {	/* second child */

			/* being user ltpuser2 */
			rval = setregid(ltpuser2->pw_gid, ltpuser2->pw_gid);
			if (rval < 0) {
				tst_resm(TFAIL, "setregid failed to "
					 "to set the real gid to %d and "
					 "effective gid to %d",
					 ltpuser2->pw_gid, ltpuser2->pw_gid);
				perror("setregid");
				exit(1);
				/* NOTREACHED */
			}
			rval = setreuid(ltpuser2->pw_uid, ltpuser2->pw_uid);
			if (rval < 0) {
				tst_resm(TFAIL, "setreuid failed to "
					 "to set the real uid to %d and "
					 "effective uid to %d",
					 ltpuser2->pw_uid, ltpuser2->pw_uid);
				perror("setreuid");
				exit(1);
				/* NOTREACHED */

			}

			/*
			 * create a sub direcoty
			 * under the directory just created
			 * by ltpuser1
			 */
			if (mkdir(tstdir2, PERMS) != 0) {
				tst_resm(TFAIL, "mkdir() failed to create"
					 " a directory %s under %s ", tstdir2,
					 tstdir1);
				exit(1);
				/* NOTREACHED */
			}
			/*
			 * check the group ID
			 * should not be the same as the current processs's
			 * since parent directory is set with S_ISGID bit
			 */
			if (stat(tstdir2, &buf) == -1) {
				tst_resm(TFAIL,
					 "failed to stat the new directory"
					 "in mkdir()");
				exit(1);
				/* NOTREACHED */
			}
			if (stat(tstdir1, &buf1) == -1) {
				tst_resm(TFAIL,
					 "failed to stat the new directory"
					 "in mkdir()");
				exit(1);
				/* NOTREACHED */
			}
			if (buf.st_gid != buf1.st_gid) {
				tst_resm(TFAIL, "mkdir() FAILED to inherit "
					 " the group ID %d from parent "
					 " directory %d",
					 buf.st_gid, buf1.st_gid);
				exit(1);
				/* NOTREACHED */
			}

			/* check the S_ISGID  bit */
			if (!(buf.st_mode & S_ISGID)) {
				tst_resm(TFAIL, "mkdir() FAILED to inherit "
					 " the S_ISGID bit from parent "
					 " directory");
				exit(1);
				/* NOTREACHED */
			}
			/* PASS */
			exit(0);
			/* NOTREACHED */
		}

		waitpid(pid1, &status, 0);
		if (WEXITSTATUS(status) == 0) {
			tst_resm(TPASS, "Test to attempt to make a directory"
				 " inherits group ID SUCCEEDED ");
		} else {
			tst_resm(TFAIL, "Test to attempt to make a directory"
				 " inherits group ID FAILED");
			cleanup();
		 /*NOTREACHED*/}

	}			/* End for TEST_LOOPING */

	/*
	 * cleanup and exit
	 */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* must run as root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, cleanup, "Must run this as root");
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	umask(0);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

	/*
	 * Exit with return code appropriate for results.
	 */
	tst_exit();
}
