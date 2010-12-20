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
 *	execve04.c
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
 *  execve04 -F <test file> [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	04/2008 Roy Lee <roylee@andestech.com>
 *              - Fix a synchronization issue.
 *                On a loaded system, the 'execving' child can get access
 *                to the file before the 'opening' child does, hence results
 *                in an unexpected opening fail.
 *
 * RESTRICTIONS
 *	must be run with -F <test file> option
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>

#include "test.h"
#include "usctest.h"
#include "libtestsuite.h"

char *test_app;
char *TCID = "execve04";
int TST_TOTAL = 1;

void setup(char *);
void cleanup(void);
void help(void);
void do_child_1(void);
void do_child_2(void);

int exp_enos[] = { ETXTBSY, 0 };
int start_sync_pipes[2];
int end_sync_pipes[2];

int Fflag = 0;

#ifdef UCLINUX
#define PIPE_NAME_START		"execve04_start"
#define PIPE_NAME_END		"execve04_end"
#else
#define PIPE_NAME_START		NULL
#define PIPE_NAME_END		NULL
#endif

option_t options[] = {
	{"F:", &Fflag, &test_app},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t pid, pid1;
	int retval = 3, status;
	char *argv[1], *env[1];

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
#ifdef UCLINUX
	maybe_run_child(&do_child_1, "nS", 1, &test_app);
#endif

	if (!Fflag)
		tst_brkm(TBROK, NULL,
		    "You must specify an executable file with the -F option.");

	setup(*av);

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		if (sync_pipe_create(start_sync_pipes, PIPE_NAME_START) == -1)
			tst_brkm(TBROK, cleanup, "sync_pipe_create failed");
		if (sync_pipe_create(end_sync_pipes, PIPE_NAME_END) == -1)
			tst_brkm(TBROK, cleanup, "sync_pipe_create failed");

		/*
		 * to test whether execve(2) sets ETXTBSY when a second
		 * child process attempts to execve the executable opened
		 * by the first child process
		 */
		if ((pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK, cleanup, "fork #1 failed");
		else if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(av[0], "nS", 1, test_app) < 0)
				tst_brkm(TBROK, cleanup, "self_exec failed");
#else
			do_child_1();
#endif
		}

		if (sync_pipe_wait(start_sync_pipes) == -1)
			tst_brkm(TBROK, cleanup, "sync_pipe_wait failed");

		if (sync_pipe_close(start_sync_pipes, PIPE_NAME_START) == -1)
			tst_brkm(TBROK, cleanup, "sync_pipe_close failed");

		if ((pid1 = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK, cleanup, "fork #2 failed");

		if (pid1 == 0) {

			retval = 3;

			argv[0] = 0;
			env[0] = 0;

			/* do not interfere with end synchronization of first
			 * child */
			sync_pipe_close(end_sync_pipes, PIPE_NAME_END);

			TEST(execve(test_app, argv, env));

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO != ETXTBSY) {
				retval = 1;
				perror("didn't get ETXTBSY\n");
			} else
				printf("execve failed with ETXTBSY as "
				    "expected\n");
			exit(retval);
		}
		/* wait for the child to finish */
		if (waitpid(pid1, &status, 0) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid failed");
		if (WIFEXITED(status) && WEXITSTATUS(status) == 3)
			tst_resm(TPASS, "execve failed as expected");
		else
			tst_resm(TFAIL, "execve succeeded, expected failure");

		/*  terminate first child */
		sync_pipe_notify(end_sync_pipes);
		(void) waitpid(pid, NULL, 0);
	}
	cleanup();

	tst_exit();
}

void help()
{
	printf("  -F <test name> : for example, 'execve04 -F test3'\n");
}

void setup(char *argv0)
{
	char *cmd, *pwd = NULL;
	char test_path[MAXPATHLEN];

	if (test_app[0] == '/')
		strncpy(test_path, test_app, sizeof(test_path));
	else {
		if ((pwd = get_current_dir_name()) == NULL)
			tst_brkm(TBROK|TERRNO, NULL, "getcwd failed");

		snprintf(test_path, sizeof(test_path), "%s/%s",
		    pwd, basename(test_app));

		free(pwd);
	}

	cmd = malloc(strlen(test_path) + strlen("cp -p \"") + strlen("\" .") +
	    1);
	if (cmd == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "Cannot alloc command string");

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	sprintf(cmd, "cp -p \"%s\" .", test_path);
	if (system(cmd) != 0)
		tst_brkm(TBROK, NULL, "command failed: %s", cmd);
	free(cmd);
}

void cleanup()
{
	TEST_CLEANUP;

	tst_rmdir();

}

void do_child_1()
{
	int fildes;

#ifdef UCLINUX
	if (sync_pipe_create(start_sync_pipes, PIPE_NAME_START) == -1)
		tst_brkm(TBROK, cleanup, "sync_pipe_create failed");
	if (sync_pipe_create(end_sync_pipes, PIPE_NAME_END) == -1)
		tst_brkm(TBROK, cleanup, "sync_pipe_create failed");
#endif

	if ((fildes = open(test_app, O_WRONLY)) == -1) {
		printf("%s\n", test_app);
		perror("open failed");
		exit(1);
	}

	if (sync_pipe_notify(start_sync_pipes) == -1) {
		perror("sync_pipe_notify failed");
		exit(1);
	}

	if (sync_pipe_close(start_sync_pipes, PIPE_NAME_START) == -1) {
		perror("sync_pipe_close failed");
		exit(1);
	}

	if (sync_pipe_wait(end_sync_pipes) == -1) {
		perror("sync_pipe_wait failed");
		exit(1);
	}
	exit(0);
}
