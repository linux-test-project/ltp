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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/* 10/30/2002	Port to LTP	dbarrera@us.ibm.com */

/*
 * Stress test of mkdir call.
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
 */

#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include "test.h"

#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#define NCHILD		3

#define MODE_RWX	07770
#define DIR_NAME	"./X.%d"

char *TCID = "mkdir09";
int TST_TOTAL = 1;

char testdir[MAXPATHLEN];
int parent_pid, sigchld, sigterm, jump;
void term(int sig);
void chld(int sig);
int *pidlist, child_count;
jmp_buf env_buf;

int getchild(int group, int child, int children);
int dochild1(void);
int dochild2(void);
int dochild3(int group);
int massmurder(void);
int runtest(void);
void setup(void);
void cleanup(void);

static int child_groups = 2;
static int test_time = 5;
static int nfiles = 5;

static char *opt_child_groups;
static char *opt_test_time;
static char *opt_nfiles;

static option_t options[] = {
	{"c:", NULL, &opt_child_groups},
	{"t:", NULL, &opt_test_time},
	{"d:", NULL, &opt_nfiles},
	{NULL, NULL, NULL}
};

static void usage(void)
{
	printf("  -c      Child groups\n");
	printf("  -t      Test runtime\n");
	printf("  -d      Directories\n");
}

int main(int argc, char *argv[])
{
	tst_parse_opts(argc, argv, options, usage);

	if (opt_child_groups)
		child_groups = atoi(opt_child_groups);

	if (opt_test_time)
		test_time = atoi(opt_test_time);

	if (opt_nfiles)
		nfiles = atoi(opt_nfiles);

	setup();

	if (signal(SIGTERM, term) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error setting up SIGTERM signal, ERRNO = %d", errno);

	}

	if (signal(SIGCHLD, chld) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error setting up SIGCHLD signal, ERRNO = %d", errno);

	}

	runtest();
	cleanup();
	tst_exit();
}

int runtest(void)
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
		}
		if ((j % NCHILD) != 0) {
			if (rmdir(tmpdir) < 0) {
				tst_brkm(TFAIL, cleanup,
					 "Error removing directory, ERRNO = %d",
					 errno);
			}
		}
	}

	parent_pid = getpid();

	/* allocate space for list of child pid's */

	if ((pidlist = malloc((child_groups * NCHILD) * sizeof(int))) ==
	    NULL) {
		tst_brkm(TWARN, NULL,
			 "\tMalloc failed (may be OK if under stress)");
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
	}
	if (signal(SIGCHLD, SIG_DFL) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "Error resetting SIGCHLD signal, ERRNO = %d", errno);
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
				tst_brkm(TWARN,
					 NULL,
					 "\tChild{%d} exited status = %0x",
					 child, status);
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
		tst_brkm(TWARN, NULL, "\tSaw %d, expected %d", count,
			 NCHILD);
	}

	/* Check for core file in test directory. */

	if (access("core", 0) == 0) {
		tst_brkm(TWARN, NULL, "\tCore file found in test directory.");
	}

	/* Remove expected files */

	for (j = 0; j < nfiles; j += NCHILD) {
		sprintf(tmpdir, DIR_NAME, j);
		if (rmdir(tmpdir) < 0) {
			tst_brkm(TWARN,
				 NULL,
				 "\tError removing expected directory, ERRNO = %d",
				 errno);
		}
	}

	tst_resm(TPASS, "PASS");

	return 0;
}

int getchild(int group, int child, int children)
{
	int pid;

	pid = FORK_OR_VFORK();

	if (pid < 0) {

		massmurder();	/* kill the kids */
		tst_brkm(TBROK, cleanup,
			 "\tFork failed (may be OK if under stress)");
	} else if (pid == 0) {	/* child does this */
		switch (children % NCHILD) {
		case 0:
			dochild1();	/* create existing directories */
			break;	/* so lint won't complain */
		case 1:
			dochild2();	/* remove nonexistant directories */
			break;
		case 2:
			dochild3(group);	/* create/delete directories */
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

void term(int sig)
{
	/* Routine to handle SIGTERM signal. */

	if (parent_pid == getpid()) {
		tst_brkm(TWARN, NULL, "\tsignal SIGTERM received by parent.");
	}
	sigterm++;
	if (jump) {
		longjmp(env_buf, 1);
	}
}

void chld(int sig)
{
	/* Routine to handle SIGCHLD signal. */

	sigchld++;
	if (jump) {
		longjmp(env_buf, 1);
	}
}

int dochild1(void)
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

int dochild2(void)
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

int dochild3(int group)
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

int massmurder(void)
{
	register int j;
	for (j = 0; j < child_count; j++) {
		if (pidlist[j] > 0) {
			if (kill(pidlist[j], SIGTERM) < 0) {
				tst_brkm(TFAIL, cleanup,
					 "Error killing child %d, ERRNO = %d",
					 j, errno);
			}
		}
	}
	return 0;
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
}

void cleanup(void)
{
	tst_rmdir();
}
