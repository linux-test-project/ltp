/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 *
 * EPERM   -  The calling process, process specified by pid and the target
 *            process group must be in the same session.
 *
 * EACCESS -  Proccess cannot change process group ID of a child after child
 *            has performed exec()
 */

#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "test.h"

#define TEST_APP "setpgid03_child"

char *TCID = "setpgid03";
int TST_TOTAL = 1;

static void do_child(void);
static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int child_pid;
	int status;
	int rval;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* Child is in new session we are not alowed to change pgid */
		if ((child_pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK, cleanup, "fork() failed");

		if (child_pid == 0) {
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0)
				tst_brkm(TBROK, cleanup, "self_exec failed");
#else
			do_child();
#endif
		}

		TST_SAFE_CHECKPOINT_WAIT(cleanup, 0);
		rval = setpgid(child_pid, getppid());
		if (rval == -1 && errno == EPERM) {
			tst_resm(TPASS, "setpgid failed with EPERM");
		} else {
			tst_resm(TFAIL,
				"retval %d, errno %d, expected errno %d",
				rval, errno, EPERM);
		}
		TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);

		if (wait(&status) < 0)
			tst_resm(TFAIL | TERRNO, "wait() for child 1 failed");

		if (!(WIFEXITED(status)) || (WEXITSTATUS(status) != 0))
			tst_resm(TFAIL, "child 1 failed with status %d",
				WEXITSTATUS(status));

		/* Child after exec() we are no longer allowed to set pgid */
		if ((child_pid = FORK_OR_VFORK()) == -1)
			tst_resm(TFAIL, "Fork failed");

		if (child_pid == 0) {
			if (execlp(TEST_APP, TEST_APP, NULL) < 0)
				perror("exec failed");

			exit(127);
		}

		TST_SAFE_CHECKPOINT_WAIT(cleanup, 0);
		rval = setpgid(child_pid, getppid());
		if (rval == -1 && errno == EACCES) {
			tst_resm(TPASS, "setpgid failed with EACCES");
		} else {
			tst_resm(TFAIL,
				"retval %d, errno %d, expected errno %d",
				rval, errno, EACCES);
		}
		TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);

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
	if (setsid() < 0) {
		printf("CHILD: setsid() failed, errno: %d\n", errno);
		exit(2);
	}

	TST_SAFE_CHECKPOINT_WAKE(NULL, 0);

	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);

	exit(0);
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TST_CHECKPOINT_INIT(tst_rmdir);

	umask(0);

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}
