/******************************************************************************/
/* Copyright (c) Kerlabs 2008.                                                */
/* Copyright (c) International Business Machines  Corp., 2008                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/*
 */
/*
 * NAME
 * 	setuid04.c
 *
 * DESCRIPTION
 * 	Check if setuid behaves correctly with file permissions.
 *      The test creates a file as ROOT with permissions 0644, does a setuid
 *      and then tries to open the file with RDWR permissions.
 *      The same test is done in a fork to check if new UIDs are correctly
 *      passed to the son.
 *
 * USAGE:  <for command-line>
 *  setuid04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Created by Renaud Lottiaux
 *
 * RESTRICTIONS
 * 	Must be run as root.
 */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"
#include <pwd.h>

char *TCID = "setuid04";
int TST_TOTAL = 1;
extern int Tst_count;
char nobody_uid[] = "nobody";
char testfile[256] = "";
struct passwd *ltpuser;

int exp_enos[] = { EACCES, 0 };
int fd = -1;

void setup(void);
void cleanup(void);
void do_master_child();

int main(int ac, char **av)
{
	pid_t pid;
	char *msg;		/* message returned from parse_opts */
	int status;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	/*
	 * perform global setup for the test
	 */
	setup();

	TEST_EXP_ENOS(exp_enos);

	pid = FORK_OR_VFORK();
	if (pid < 0)
		tst_brkm(TBROK, cleanup, "Fork failed");

	if (pid == 0) {
		do_master_child();
	} else {
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0))
			tst_resm(WEXITSTATUS(status),
				 "son process exits with error");
	}

	cleanup();
	 /*NOTREACHED*/ return 0;
}

/*
 * do_master_child()
 */
void do_master_child()
{
	int lc;			/* loop counter */
	int pid;
	int status;

	if (setuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK, tst_exit,
			 "setuid failed to set the effective uid to %d",
			 ltpuser->pw_uid);
	}

	/* Check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int tst_fd;

		/* Reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(tst_fd = open(testfile, O_RDWR));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			close(tst_fd);
		}

		if (TEST_ERRNO == EACCES) {
			tst_resm(TPASS, "open returned errno EACCES");
		} else {
			tst_resm(TFAIL, "open returned unexpected errno - %d",
				 TEST_ERRNO);
			continue;
		}

		pid = FORK_OR_VFORK();
		if (pid < 0)
			tst_brkm(TBROK, tst_exit, "Fork failed");

		if (pid == 0) {
			int tst_fd2;

			/* Test to open the file in son process */
			TEST(tst_fd2 = open(testfile, O_RDWR));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				close(tst_fd2);
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == EACCES) {
				tst_resm(TPASS, "open returned errno EACCES");
			} else {
				tst_resm(TFAIL,
					 "open returned unexpected errno - %d",
					 TEST_ERRNO);
			}
			tst_exit();
		} else {
			/* Wait for son completion */
			waitpid(pid, &status, 0);
			if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0))
				exit(WEXITSTATUS(status));
		}
	}
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	ltpuser = getpwnam(nobody_uid);

	sprintf(testfile, "setuid04file%d.tst", getpid());

	/* Create test file */
	fd = open(testfile, O_CREAT | O_RDWR, 0644);
	if (fd < 0)
		tst_brkm(TBROK, cleanup, "cannot creat test file");

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit
 */
void cleanup(void)
{
	close(fd);
	unlink(testfile);

	/*
	 * print timing status if that option was specified
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
