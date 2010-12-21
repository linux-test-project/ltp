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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/* 10/30/2002	Port to LTP	dbarrera@us.ibm.com */

/*
 * NAME
 *	mkdir.c - Stress test of mkdir call.
 *
 * CALLS
 *	mkdir, rmdir
 *
 * ALGORITHM
 *	Create multiple processes which create subdirectories in the
 *	same directory multiple times. On exit of all child processes,
 *	make sure all subdirectories can be removed.
 *
 *      USAGE: mkdir09 -c # -t # -d #
 *              -c = number of children groups
 *              -t = number of seconds to run test
 *              -d = number of directories created in test directory
 *
 * RESTRICTIONS
 *
 */

#include <stdio.h>		/* needed by testhead.h         */
#include <wait.h>		/* needed by testhead.h         */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include "test.h"
#include "usctest.h"

#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#define NCHILD		3

#define MODE_RWX	07770
#define DIR_NAME	"./X.%d"

/* used by getopt */
extern char *optarg;
extern int optind, opterr;
char *goodopts = "c:t:d:";
int errflg;

/*
 *  * These globals must be defined in the test.
 *   */

char *TCID = "mkdir09";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int exp_enos[] = { EFAULT, 0 };	/* List must end with 0 */

int child_groups, test_time, nfiles;
char testdir[MAXPATHLEN];
int parent_pid, sigchld, sigterm, jump;
void term();
void chld();
int *pidlist, child_count;
jmp_buf env_buf;

int getchild(int group, int child, int children);
int dochild1();
int dochild2();
int dochild3(int group);
int massmurder();
int runtest();
void setup();
void cleanup();

#ifdef UCLINUX
static char *argv0;
void dochild1_uclinux();
void dochild2_uclinux();
void dochild3_uclinux();
static int group_uclinux;
#endif

/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
int main(argc, argv)
int argc;
char *argv[];
{
	int c;

#ifdef UCLINUX
	char *msg;

	/* parse standard options */
	if ((msg =
	     parse_opts(argc, argv, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	argv0 = argv[0];
	maybe_run_child(&dochild1_uclinux, "nd", 1, &nfiles);
	maybe_run_child(&dochild2_uclinux, "n", 2);
	maybe_run_child(&dochild3_uclinux, "nd", 3, &group_uclinux);
#endif

	setup();

	/* Set up to catch SIGTERM signal */
	if (signal(SIGTERM, term) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error setting up SIGTERM signal, ERRNO = %d", errno);

	}

	/* Set up to catch SIGCLD signal */
	if (signal(SIGCLD, chld) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error setting up SIGCLD signal, ERRNO = %d", errno);

	}

	/* Default argument settings. */

	child_groups = 2;
	test_time = 5;		/* 0 = run forever or till signal */
	nfiles = 5;

	/* Get command line options */

	while ((c = getopt(argc, argv, goodopts)) != EOF) {
		switch (c) {
		case 'c':
			child_groups = atoi(optarg);
			break;
		case 't':
			test_time = atoi(optarg);
			break;
		case 'd':
			nfiles = atoi(optarg);
			break;
		case '?':
			errflg++;
			break;
		default:
			break;
		}
	}
	if (errflg) {
		tst_resm(TINFO,
			 "USAGE : mkdir09 -c #child_groups -t#test_time -d#directories");
		tst_resm(TINFO, "Bad argument count.");

	}

	runtest();
	cleanup();
	tst_exit();

}

/*--------------------------------------------------------------*/

int runtest()
{
	int i, j;
	int count, child, status;
	char tmpdir[MAXPATHLEN];

	/* Create permanent directories with holes in directory structure */

	for (j = 0; j < nfiles; j++) {
		sprintf(tmpdir, DIR_NAME, j);
		TEST(mkdir(tmpdir, MODE_RWX));

		if (TEST_RETURN < 0) {
			tst_brkm(TFAIL, cleanup,
				 "Error creating permanent directories, ERRNO = %d",
				 TEST_ERRNO);
			tst_exit();
		}
		if ((j % NCHILD) != 0) {
			if (rmdir(tmpdir) < 0) {
				tst_brkm(TFAIL, cleanup,
					 "Error removing directory, ERRNO = %d",
					 errno);
				tst_exit();
			}
		}
	}

	parent_pid = getpid();

	/* allocate space for list of child pid's */

	if ((pidlist =
	     (int *)malloc((child_groups * NCHILD) * sizeof(int))) ==
	    (int *)0) {
		tst_resm(TWARN, "\tMalloc failed (may be OK if under stress)");
		tst_exit();
	}

	child_count = 0;
	for (j = 0; j < child_groups; j++) {
		for (i = 0; i < NCHILD; i++) {
			getchild(j, i, child_count);
			child_count++;
		}
	}

	/* If signal already received, skip to cleanup */

	if (!sigchld && !sigterm) {
		if (test_time) {
			/* To get out of sleep if signal caught */
			if (!setjmp(env_buf)) {
				jump++;
				sleep(test_time);
			}
		} else {
			pause();
		}
	}

	/* Reset signals since we are about to clean-up and to avoid
	 * problem with wait call *               $
	 * */

	if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error resetting SIGTERM signal, ERRNO = %d", errno);
		tst_exit();
	}
	if (signal(SIGCLD, SIG_DFL) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error resetting SIGCLD signal, ERRNO = %d", errno);
		tst_exit();
	}

	if (test_time) {
		sleep(test_time);
	}

	/* Clean up children */
	massmurder();
	/*
	 * Watch children finish and show returns.
	 */

	count = 0;
	while (1) {
		if ((child = wait(&status)) > 0) {
			if (status != 0) {
				tst_resm(TWARN,
					 "\tChild{%d} exited status = %0x",
					 child, status);
				tst_exit();
			}
			count++;
		} else {
			if (errno != EINTR) {
				break;
			}
			tst_resm(TINFO, "\tSignal detected during wait");
		}
	}

	/*
	 * Make sure correct number of children exited.
	 */

	if (count != child_count) {
		tst_resm(TWARN, "\tWrong number of children waited on!");
		tst_resm(TWARN, "\tSaw %d, expected %d", count, NCHILD);
		tst_exit();
	}

	/* Check for core file in test directory. */

	if (access("core", 0) == 0) {
		tst_resm(TWARN, "\tCore file found in test directory.");
		tst_exit();
	}

	/* Remove expected files */

	for (j = 0; j < nfiles; j += NCHILD) {
		sprintf(tmpdir, DIR_NAME, j);
		if (rmdir(tmpdir) < 0) {
			tst_resm(TWARN,
				 "\tError removing expected directory, ERRNO = %d",
				 errno);
			tst_exit();
		}
	}

	tst_resm(TPASS, "PASS");

	return 0;
}

int getchild(group, child, children)
int group, child, children;
{
	int pid;

	pid = FORK_OR_VFORK();

	if (pid < 0) {

		massmurder();	/* kill the kids */
		tst_brkm(TBROK, cleanup,
			 "\tFork failed (may be OK if under stress)");
		tst_exit();
	} else if (pid == 0) {	/* child does this */
		switch (children % NCHILD) {
		case 0:
#ifdef UCLINUX
			if (self_exec(argv0, "nd", 1, nfiles) < 0) {
				massmurder();
				tst_brkm(TBROK, cleanup, "\tself_exec failed");
				tst_exit();
			}
#else
			dochild1();	/* create existing directories */
#endif
			break;	/* so lint won't complain */
		case 1:
#ifdef UCLINUX
			if (self_exec(argv0, "n", 2) < 0) {
				massmurder();
				tst_brkm(TBROK, cleanup, "\tself_exec failed");
				tst_exit();
			}
#else
			dochild2();	/* remove nonexistant directories */
#endif
			break;
		case 2:
#ifdef UCLINUX
			if (self_exec(argv0, "nd", 3, group) < 0) {
				massmurder();
				tst_brkm(TBROK, cleanup, "\tself_exec failed");
				tst_exit();
			}
#else
			dochild3(group);	/* create/delete directories */
#endif
			break;
		default:
			tst_brkm(TFAIL, cleanup,
				 "Test not inplemented for child %d", child);
			exit(1);
			break;
		}
		exit(1);	/* If child gets here, something wrong */
	}
	pidlist[children] = pid;
	return 0;
}

void term()
{
	/* Routine to handle SIGTERM signal. */

	if (parent_pid == getpid()) {
		tst_resm(TWARN, "\tsignal SIGTERM received by parent.");
		tst_exit();
	}
	sigterm++;
	if (jump) {
		longjmp(env_buf, 1);
	}
}

void chld()
{
	/* Routine to handle SIGCLD signal. */

	sigchld++;
	if (jump) {
		longjmp(env_buf, 1);
	}
}

int dochild1()
{
	/* Child routine which attempts to create directories in the test
	 * directory that already exist. Runs until a SIGTERM signal is
	 * received. Will exit with an error if it is able to create the
	 * directory or if the expected error is not received.
	 */

	int j;
	char tmpdir[MAXPATHLEN];

	while (!sigterm) {
		for (j = 0; j < nfiles; j += NCHILD) {
			sprintf(tmpdir, DIR_NAME, j);
			TEST(mkdir(tmpdir, MODE_RWX));

			if (TEST_RETURN < 0) {

				if (TEST_ERRNO != EEXIST) {
					tst_brkm(TFAIL, cleanup,
						 "MKDIR %s, errno = %d; Wrong error detected.",
						 tmpdir, TEST_ERRNO);
					exit(1);
				}
			} else {
				tst_brkm(TFAIL, cleanup,
					 "MKDIR %s succeded when it shoud have failed.",
					 tmpdir);
				exit(1);
			}
		}
	}
	exit(0);
}

#ifdef UCLINUX
void dochild1_uclinux()
{
	/* Set up to catch SIGTERM signal */
	if (signal(SIGTERM, term) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error setting up SIGTERM signal, ERRNO = %d", errno);
		tst_exit();
	}

	dochild1();
}
#endif

int dochild2()
{
	/* Child routine which attempts to remove directories from the
	 * test directory which do not exist. Runs until a SIGTERM
	 * signal is received. Exits with an error if the proper
	 * error is not detected or if the remove operation is
	 * successful.
	 */

	int j;
	char tmpdir[MAXPATHLEN];

	while (!sigterm) {
		for (j = 1; j < nfiles; j += NCHILD) {
			sprintf(tmpdir, DIR_NAME, j);
			if (rmdir(tmpdir) < 0) {
				if (errno != ENOENT) {
					tst_brkm(TFAIL, cleanup,
						 "RMDIR %s, errno = %d; Wrong error detected.",
						 tmpdir, errno);
					exit(1);
				}
			} else {
				tst_brkm(TFAIL, cleanup,
					 "RMDIR %s succeded when it should have failed.",
					 tmpdir);
				exit(1);
			}
		}
	}
	exit(0);
	return 0;
}

#ifdef UCLINUX
void dochild2_uclinux()
{
	/* Set up to catch SIGTERM signal */
	if (signal(SIGTERM, term) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error setting up SIGTERM signal, ERRNO = %d", errno);
		tst_exit();
	}

	dochild2();
}
#endif

int dochild3(group)
int group;
{
	/* Child routine which creates and deletes directories in the
	 * test directory. Runs until a SIGTERM signal is received, then
	 * cleans up and exits. Detects error if the expected condition
	 * is not encountered.
	 */

	int j;

	char tmpdir[MAXPATHLEN];
	char tmp[MAXPATHLEN];

	while (!sigterm) {
		for (j = 2; j < nfiles; j += NCHILD) {
			strcpy(tmp, DIR_NAME);
			strcat(tmp, ".%d");
			sprintf(tmpdir, tmp, j, group);

			TEST(mkdir(tmpdir, MODE_RWX));

			if (TEST_RETURN < 0) {
				tst_brkm(TFAIL, cleanup,
					 "MKDIR %s, errno = %d; Wrong error detected.",
					 tmpdir, TEST_ERRNO);
				exit(1);
			}
		}
		for (j = 2; j < nfiles; j += NCHILD) {
			strcpy(tmp, DIR_NAME);
			strcat(tmp, ".%d");
			sprintf(tmpdir, tmp, j, group);
			if (rmdir(tmpdir) < 0) {
				tst_brkm(TFAIL, cleanup,
					 "RMDIR %s, errno = %d; Wrong error detected.",
					 tmpdir, errno);
				exit(1);
			}
		}
	}
	exit(0);
}

#ifdef UCLINUX
void dochild3_uclinux()
{
	/* Set up to catch SIGTERM signal */
	if (signal(SIGTERM, term) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error setting up SIGTERM signal, ERRNO = %d", errno);
		tst_exit();
	}

	dochild3(group_uclinux);
}
#endif

int massmurder()
{
	register int j;
	for (j = 0; j < child_count; j++) {
		if (pidlist[j] > 0) {
			if (kill(pidlist[j], SIGTERM) < 0) {
				tst_brkm(TFAIL, cleanup,
					 "Error killing child %d, ERRNO = %d",
					 j, errno);
				tst_exit();
			}
		}
	}
	return 0;
}

/***************************************************************
 *  * setup() - performs all ONE TIME setup for this test.
 *   ***************************************************************/
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

}

/***************************************************************
 *  * cleanup() - performs all ONE TIME cleanup for this test at
 *   *              completion or premature exit.
 *    ***************************************************************/
void cleanup()
{
	/*
	 *      * print timing stats if that option was specified.
	 *           * print errno log if that option was specified.
	 *                */
	TEST_CLEANUP;

	/*
	 *      * Remove the temporary directory.
	 *           */
	tst_rmdir();

	/*
	 *      * Exit with return code appropriate for results.
	 *           */

}
