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
 *	creat07.c
 *
 * DESCRIPTION
 *	Testcase to check creat(2) sets the following errnos correctly:
 *	1.	ETXTBSY
 *
 * ALGORITHM
 *	1.	Attempt to creat(2) an executable which is currently running,
 *		and test for ETXTBSY
 *
 * USAGE:  <for command-line>
 *  creat07 -F test1 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *	       -F <test file> : Specify the test executable to launch
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	test must be run with the -F option
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

char *TCID = "creat07";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);
void help(void);

int exp_enos[] = {ETXTBSY, 0};

int Fflag = 0;
char *fname;

/* for test specific parse_opts options - in this case "-F" */
option_t options[] = {
	{"F:", &Fflag, &fname},
	{NULL, NULL, NULL}
};

int
main(int ac, char **av)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */
	int retval=0, status, e_code;
	pid_t pid, pid2;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	if (!Fflag) {
		tst_resm(TWARN, "You must specifiy the test executable 'test1' "
			 "with the -F option");
		tst_resm(TWARN, "Run '%s -h' for option information.", TCID);
		tst_exit();
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * test whether creat(2) is setting ETXTBSY if an attempt is
		 * made to creat() an executable which is running.
		 */

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() #1 failed");
		}

		if (pid == 0) {		/* first child */
			/* start up the test1 executable */
			(void)execve(fname, NULL, NULL);
			tst_resm(TFAIL, "execve() failed");
			exit(1);
		}

		if ((pid2 = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() #2 failed");
		}

		if (pid2 == 0) {		/* second child */
			sleep(10);		/* let first child start */

			TEST(creat(fname, O_WRONLY));

			if (TEST_RETURN != -1) {
				retval=1;
				tst_resm(TFAIL, "creat(2) succeeded on "
					 "expected fail");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO != ETXTBSY) {
				retval=1;
				tst_resm(TFAIL, "expected ETXTBSY, instead "
					 "received %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS, "received ETXTBSY");
			}

			/* kill off the dummy test program */
			if (kill(pid, SIGKILL) == -1) {
				retval=1;
				tst_brkm(TBROK, cleanup, "kill failed");
			}
			exit(retval);
		} else {
                       /* wait for the child to finish */
                        wait(&status);
                        /* make sure the child returned a good exit status */
                        e_code = status >> 8;
                        if (e_code != 0) {
                                tst_resm(TFAIL, "Failures reported above");
                        }
		}
	}
	cleanup();

	return 0;
	/*NOTREACHED*/
}

/*
 * help() - Prints out the help message for the -F option defined
 *          by this test.
 */
void
help()
{
	printf("  -F <test file> : in this case the file is 'test1'\n");
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void
setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}


/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
