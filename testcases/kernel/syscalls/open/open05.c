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
 *	open05.c
 *
 * DESCRIPTION
 *	Testcase to check open(2) sets errno to EACCES correctly.
 *
 * ALGORITHM
 *	Create a file owned by root with no read permission for other users.
 *	Attempt to open it as ltpuser(1). The attempt should fail with EACCES.
 *
 * USAGE:  <for command-line>
 *  open05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * RESTRICTION
 *	Must run test as root.
 *
 */
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

char user1name[] = "nobody";

char *TCID = "open05";
int TST_TOTAL = 1;
extern int Tst_count;

extern struct passwd *my_getpwnam(char *);
char fname[20];
struct passwd *nobody;
int fd;

int exp_enos[] = { EACCES, 0 };

void cleanup(void);
void setup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int e_code, status, retval = 0;
	pid_t pid;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		 /*NOTREACHED*/}

		if (pid == 0) {	/* child */
			if (seteuid(nobody->pw_uid) == -1) {
				tst_resm(TWARN, "seteuid() failed, errno: %d",
					 errno);
			}

			TEST(open(fname, O_RDWR));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "open succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO != EACCES) {
				retval = 1;
				tst_resm(TFAIL, "Expected EACCES got %d",
					 TEST_ERRNO);
			} else {
				tst_resm(TPASS, "open returned expected "
					 "EACCES error");
			}

			/* set the id back to root */
			if (seteuid(0) == -1) {
				tst_resm(TWARN, "seteuid(0) failed");
			}
			exit(retval);

		} else {	/* parent */
			/* wait for the child to finish */
			wait(&status);
			/* make sure the child returned a good exit status */
			e_code = status >> 8;
			if ((e_code != 0) || (retval != 0)) {
				tst_resm(TFAIL, "Failures reported above");
			}

			close(fd);
			cleanup();

		}
	}

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* test must be run as root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Must run test as root");
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	sprintf(fname, "file.%d", getpid());

	nobody = my_getpwnam(user1name);

	if ((fd = open(fname, O_RDWR | O_CREAT, 0700)) == -1) {
		tst_brkm(TBROK, cleanup, "open() failed, errno: %d", errno);
	 /*NOTREACHED*/}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	unlink(fname);

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
