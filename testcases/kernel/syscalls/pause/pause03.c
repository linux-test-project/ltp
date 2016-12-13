/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 * Test Description:
 *  pause() does not return due to receipt of SIGKILL signal and specified
 *  process should be terminated.
 */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"

static pid_t cpid;

char *TCID = "pause03";
int TST_TOTAL = 1;

static void do_child(void);
static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	int status;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if ((cpid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK | TERRNO, NULL, "fork() failed");

		if (cpid == 0) {
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0)
				tst_brkm(TBROK, cleanup, "self_exec failed");
#else
			do_child();
#endif
		}

		TST_PROCESS_STATE_WAIT(cleanup, cpid, 'S');

		kill(cpid, SIGKILL);

		SAFE_WAIT(NULL, &status);

		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
			tst_resm(TPASS, "pause() did not return after SIGKILL");
			continue;
		}

		if (WIFSIGNALED(status)) {
			tst_resm(TFAIL, "child killed by %s unexpectedly",
			         tst_strsig(WTERMSIG(status)));
			continue;
		}

		tst_resm(TFAIL, "child exited with %i", WEXITSTATUS(status));
	}

	cleanup();
	tst_exit();

}

void do_child(void)
{
	TEST(pause());

	tst_resm(TFAIL, "Unexpected return from pause()");

	exit(0);
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}


void cleanup(void)
{
	kill(cpid, SIGKILL);
}
