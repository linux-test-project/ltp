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
 *  creat07 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	test must be run with the -F option
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include "test.h"
#include "usctest.h"

#define test_app "test1"

char *TCID = "creat07";
int TST_TOTAL = 1;

void setup(char *);
void cleanup(void);

int exp_enos[] = { ETXTBSY, 0 };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int retval = 0, status;
	pid_t pid, pid2;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup(av[0]);

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "fork #1 failed");

		if (pid == 0) {
			char *av[1];
			av[0] = basename(test_app);
			(void)execve(test_app, av, NULL);
			perror("execve failed");
			exit(1);
		}

		if ((pid2 = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK, cleanup, "fork #2 failed");

		if (pid2 == 0) {
			sleep(10);

			TEST(creat(test_app, O_WRONLY));

			if (TEST_RETURN != -1) {
				retval = 1;
				printf("creat didn't fail as expected\n");
			} else if (TEST_ERRNO == ETXTBSY)
				printf("received ETXTBSY\n");
			else {
				retval = 1;
				perror("creat failed unexpectedly");
			}

			if (kill(pid, SIGKILL) == -1) {
				retval = 1;
				perror("kill failed");
			}
			exit(retval);
		}
		if (wait(&status) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "wait failed");
		if (WIFEXITED(status) || WEXITSTATUS(status) == 0)
			tst_resm(TPASS, "creat functionality correct");
		else
			tst_resm(TFAIL, "creat functionality incorrect");
	}
	cleanup();

	tst_exit();
}

void setup(char *app)
{
	char *cmd, *pwd = NULL;
	char test_path[MAXPATHLEN];

	if (test_app[0] == '/')
		strncpy(test_path, test_app, sizeof(test_app));
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
		tst_brkm(TBROK, cleanup, "Cannot copy file %s", test_path);
	free(cmd);

	TEST_PAUSE;
}

void cleanup()
{
	TEST_CLEANUP;

	tst_rmdir();
}
