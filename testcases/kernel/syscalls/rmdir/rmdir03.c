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
 * NAME
 *	rmdir03
 *
 * DESCRIPTION
 *      check rmdir() fails with EPERM or EACCES
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Pause for SIGUSR1 if option specified.
 *		Create temporary directory.
 *
 *	Test:
 *		Loop if the proper options are given.
 *              1. create a directory tstdir1 and set the sticky bit, then
 *                 create directory tstdir2 under tstdir1. Fork a
 *                 child , set to be user nobody. Pass tstdir2 to rmdir(2).
 *                 Verify the return value is not 0 and the errno is EPERM
 *                 or EACCES.
 *              2. Fork a child, set to be user nobody. Create a directory
 *                 tstdir1 and only give write permission to nobody.
 *                 Create directory tstdir2 under tstdir1. Fork the second
 *                 child , set to be user nobody. Pass tstdir2 to rmdir(2).
 *                 Verify the return value is not 0 and the errno is EACCES.
 *
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 *
 * USAGE
 *	rmdir03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	Test must be run as root.
 *
 */
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"

void dochild1();
void dochild2();
void setup();
void cleanup();

#define PERMS		0777

static uid_t nobody_uid;

char *TCID = "rmdir03";
int TST_TOTAL = 1;

char tstdir1[255];
char tstdir2[255];
char tstdir3[255];
char tstdir4[255];

int main(int ac, char **av)
{
	int lc;
	pid_t pid;
	struct stat buf1;
	int e_code, status, status2;

	/*
	 * parse standard options
	 */
	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&dochild1, "ns", 1, tstdir2);
	maybe_run_child(&dochild2, "ns", 2, tstdir4);
#endif

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

//test1:       $
		/*
		 * attempt to rmdir a file whose parent directory has
		 * the sticky bit set without the root right
		 * or effective uid
		 */

		if (stat(tstdir1, &buf1) != -1) {
			tst_brkm(TBROK, cleanup,
				 "tmp directory %s found!", tstdir1);
		}
		/* create a directory */
		if (mkdir(tstdir1, PERMS) == -1) {
			tst_brkm(TBROK, cleanup,
				 "Couldnot create directory %s", tstdir1);
		}
		if (stat(tstdir1, &buf1) == -1) {
			perror("stat");
			tst_brkm(TBROK, cleanup, "failed to stat directory %s "
				 "in rmdir()", tstdir1);

		}
		/* set the sticky bit */
		if (chmod(tstdir1, buf1.st_mode | S_ISVTX) != 0) {
			perror("chmod");
			tst_brkm(TBROK, cleanup,
				 "failed to set the S_ISVTX bit");

		}
		/* create a sub directory under tstdir1 */
		if (mkdir(tstdir2, PERMS) == -1) {
			tst_brkm(TBROK, cleanup,
				 "Could not create directory %s", tstdir2);
		}

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (pid == 0) {	/* first child */
#ifdef UCLINUX
			if (self_exec(av[0], "ns", 1, tstdir2) < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			dochild1();
#endif
		}
		/* Parent */

//test2:       $
		/* create the a directory with 0700 permits */
		if (mkdir(tstdir3, 0700) == -1) {
			tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) Failed",
				 tstdir3, PERMS);
		}
		/* create the a directory with 0700 permits */
		if (mkdir(tstdir4, 0777) == -1) {
			tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) Failed",
				 tstdir4, PERMS);
		}

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (pid == 0) {	/* child */
#ifdef UCLINUX
			if (self_exec(av[0], "ns", 2, tstdir4) < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			dochild2();
#endif
		} else {	/* parent */
			/* wait for the child to finish */
			wait(&status);
			wait(&status2);
			/* make sure the child returned a good exit status */
			e_code = status >> 8;
			if (e_code != 0) {
				tst_resm(TFAIL, "Failures reported above");
			} else {
				/* No error in the 1st one, check the 2nd */
				e_code = status2 >> 8;
				if (e_code != 0) {
					tst_resm(TFAIL,
						 "Failures reported above");
				}
			}
		}

		/* clean up things in case we are looping */

		(void)rmdir(tstdir2);
		(void)rmdir(tstdir1);
		(void)rmdir(tstdir4);
		(void)rmdir(tstdir3);

	}

	/*
	 * cleanup and exit
	 */
	cleanup();
	tst_exit();

}

/*
 * dochild1()
 */
void dochild1(void)
{
	int retval = 0;

	/* set to nobody */
	if (seteuid(nobody_uid) == -1) {
		retval = 1;
		tst_brkm(TBROK, cleanup, "setreuid failed to "
			 "set effective uid to %d", nobody_uid);
	}

	/* rmdir tstdir2 */
	TEST(rmdir(tstdir2));

	if (TEST_ERRNO) {
	}

	if (TEST_RETURN != -1) {
		retval = 1;
		tst_resm(TFAIL, "call succeeded unexpectedly");
	} else if ((TEST_ERRNO != EPERM) && (TEST_ERRNO != EACCES)) {
		retval = 1;
		tst_resm(TFAIL, "Expected EPERM or EACCES, got %d", TEST_ERRNO);
	} else {
		tst_resm(TPASS, "rmdir() produced EPERM or EACCES");
	}

	if (seteuid(0) == -1) {
		retval = 1;
		tst_brkm(TBROK, cleanup, "seteuid(0) failed");
	}
	exit(retval);
	/* END of child 1 (test1) */
}

/*
 * dochild1()
 */
void dochild2(void)
{
	int retval = 0;

	/* set to nobody */
	if (seteuid(nobody_uid) == -1) {
		retval = 1;
		tst_brkm(TBROK, cleanup, "setreuid failed to "
			 "set effective uid to %d", nobody_uid);
	}

	/* rmdir tstdir4 */
	TEST(rmdir(tstdir4));

	if (TEST_ERRNO) {
	}

	if (TEST_RETURN != -1) {
		retval = 1;
		tst_resm(TFAIL, "call succeeded unexpectedly");
	} else if (TEST_ERRNO != EACCES) {
		retval = 1;
		tst_resm(TFAIL, "Expected EACCES got %d", TEST_ERRNO);
	} else {
		tst_resm(TPASS, "rmdir() produced EACCES");
	}

	if (seteuid(0) == -1) {
		retval = 1;
		tst_brkm(TBROK, cleanup, "seteuid(0) failed");
	}
	exit(retval);
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	struct passwd *pw;

	tst_require_root();

	pw = SAFE_GETPWNAM(NULL, "nobody");
	nobody_uid = pw->pw_uid;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	umask(0);

	sprintf(tstdir1, "./tstdir1_%d", getpid());
	sprintf(tstdir2, "%s/tstdir2_%d", tstdir1, getpid());
	sprintf(tstdir3, "./tstdir3_%d", getpid());
	sprintf(tstdir4, "%s/tstdir3_%d", tstdir3, getpid());
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 */
void cleanup(void)
{

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

	/*
	 * Exit with return code appropriate for results.
	 */

}
