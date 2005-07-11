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
 *	execve05.c
 *
 * DESCRIPTION
 *	Testcase to check execve sets the following errnos correctly:
 *	1.	ETXTBSY
 * 
 * ALGORITHM
 *	1.	Attempt to execve(2) a file which is being opened by another
 *		process for writing fails with ETXTBSY.
 *
 * USAGE:  <for command-line>
 *  execve05 -F <test file> [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	must be run with -F <test file> option
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

char *TCID = "execve05";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);
void help(void);
void do_child_1(void);
void do_child_2(void);

int exp_enos[] = {ETXTBSY, 0};

int Fflag = 0;
char *test_name;

/* for test specific parse_opts options - in this case "-F" */
option_t options[] = {
	{"F:", &Fflag, &test_name},
	{NULL, NULL, NULL}
};

int
main(int ac, char **av)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */
	pid_t pid, pid1;
	int e_code, status, retval=0;
	char *argv[1], *env[1];

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != (char *)NULL){
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

#ifdef UCLINUX
	maybe_run_child(&do_child_1, "nS", 1, &test_name);
#endif

	if (!Fflag) {
		tst_resm(TWARN, "You must specify an executable file with "
			 "the -F option.");
		tst_resm(TWARN, "Run '%s -h' for option information.", TCID);
		cleanup();
        }

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * to test whether execve(2) sets ETXTBSY when a second
		 * child process attempts to execve the executable opened
		 * by the first child process
		 */
		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork #1 failed");
		}

		if (pid == 0) {		/* first child */
#ifdef UCLINUX
			if (self_exec(av[0], "nS", 1, test_name) < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			do_child_1();
#endif
		}

		if ((pid1 = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork #2 failed");
		}

		if (pid1 == 0) {		/* second child */
			argv[0] = 0;
			env[0] = 0;

			TEST(execve(test_name, argv, env));

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO != ETXTBSY) {
				retval=1;
				tst_resm(TFAIL, "expected ETXTBSY, received "
					 "%d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS, "call generated expected "
					 "ETXTBSY error");
			}

			/* wait for the first child to exit */
			(void)waitpid(pid, NULL, 0);
			exit(retval);	
		} else {	/* parent */
			 /* wait for the child to finish */
			waitpid(pid,NULL,0);
                        waitpid(pid1,&status,0);
                        /* make sure the child returned a good exit status */
                        e_code = status >> 8;
                        if ((e_code != 0) || (retval != 0)) {
                          tst_resm(TFAIL, "Failures reported above");
                        }
			cleanup();
		}
	}

	/*NOTREACHED*/
	return(0);
}

/*
 * help() - Prints out the help message for the -D option defined
 *          by this test.
 */
void
help()
{
	printf("  -F <test name> : for example, 'execve05 -F test3'\n");
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

/*
 * do_child_1()
 */
void
do_child_1()
{
	int fildes;

	if ((fildes = open(test_name, O_WRONLY)) == -1) {
		tst_brkm(TBROK, NULL, "open(2) failed");
		exit(1);	
	}
	sleep(10);	/* let other child execve same file */
	exit(0);
}
