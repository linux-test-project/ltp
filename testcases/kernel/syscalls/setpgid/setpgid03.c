/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test to check the error and trivial conditions in setpgid system call
 */

#include <wait.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

char *TCID = "setpgid03";
int TST_TOTAL = 1;

static void do_child(void);
static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int pid;
	int rval;
	int status;

	int lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

//test1:
		/* sid of the calling process is not same */
		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (pid == 0) {	/* child */
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			do_child();
#endif
		}

		sleep(1);
		rval = setpgid(pid, getppid());
		if (errno == EPERM) {
			tst_resm(TPASS,
				"setpgid SUCCESS to set errno to EPERM");
		} else {
			tst_resm(TFAIL,
				"setpgid FAILED, expect %d, return %d",
				EPERM, errno);
		}
		sleep(1);

		if (wait(&status) < 0)
			tst_resm(TFAIL | TERRNO, "wait() for child 1 failed");

		if (!(WIFEXITED(status)) || (WEXITSTATUS(status) != 0))
			tst_resm(TFAIL, "child 1 failed with status %d",
				WEXITSTATUS(status));

//test2:
		/*
		 * Value of pid matches the pid of the child process and
		 * the child process has exec successfully. Error
		 */
		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_resm(TFAIL, "Fork failed");

		}
		if (pid == 0) {
			if (execlp("sleep", "sleep", "3", NULL) < 0) {
				perror("exec failed");
			}
			exit(127);
		}

		sleep(1);
		rval = setpgid(pid, getppid());
		if (errno == EACCES) {
			tst_resm(TPASS,
				"setpgid SUCCEEDED to set errno to EACCES");
		} else {
			tst_resm(TFAIL,
				"setpgid FAILED, expect EACCES got %d", errno);
		}

		if (wait(&status) < 0)
			tst_resm(TFAIL | TERRNO, "wait() for child 2 failed");

		if (!(WIFEXITED(status)) || (WEXITSTATUS(status) != 0))
			tst_resm(TFAIL, "child 2 failed with status %d",
				WEXITSTATUS(status));
	}

	cleanup();
	tst_exit();
}

static void do_child(void)
{
	int exno = 0;

	if (setsid() < 0) {
		printf("setsid() failed, errno: %d\n", errno);
		exno = 1;
	}
	sleep(2);
	exit(exno);
}

static void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;
}
