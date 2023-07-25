/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
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
 * Verify that, pause() returns -1 and sets errno to EINTR after receipt of a
 * signal which is caught by the calling process. Also, verify that the calling
 * process will resume execution from the point of suspension.
 */

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "test.h"

char *TCID = "pause02";
int TST_TOTAL = 1;
static pid_t cpid;

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

		cpid = tst_fork();
		switch (cpid) {
		case -1:
			tst_brkm(TBROK, cleanup, "fork() failed");
		break;
		case 0:
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0)
				tst_brkm(TBROK, cleanup, "self_exec failed");
#else
			do_child();
#endif
		break;
		default:
		break;
		}

		/*
		 * Wait for child to enter pause().
		 */
		TST_PROCESS_STATE_WAIT(cleanup, cpid, 'S');

		/*
		 * Send the SIGINT signal now, so that child
		 * returns from pause and resumes execution.
		 */
		kill(cpid, SIGINT);

		wait(&status);

		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0)
				tst_resm(TPASS, "pause was interrupted correctly");
			else
				tst_resm(TFAIL, "pause was interrupted but the "
				                "retval and/or errno was wrong");
			continue;
		}

		if (WIFSIGNALED(status)) {
			switch (WTERMSIG(status)) {
			case SIGALRM:
				tst_resm(TFAIL, "Timeout: SIGINT wasn't received by child");
			break;
			default:
				tst_resm(TFAIL, "Child killed by signal");
			}

			continue;
		}

		tst_resm(TFAIL, "Pause was not interrupted");
	}

	cleanup();
	tst_exit();
}

static void sig_handle(int sig)
{
}

static void do_child(void)
{
	/* avoid LTP framework to do whatever it likes */
	signal(SIGALRM, SIG_DFL);

	if (signal(SIGINT, sig_handle) == SIG_ERR) {
		fprintf(stderr, "Child: Failed to setup signal handler\n");
		exit(1);
	}

	/* Commit suicide after 10 seconds */
	alarm(10);

	TEST(pause());

	if (TEST_RETURN == -1) {
		if (TEST_ERRNO == EINTR)
			exit(0);

		fprintf(stderr, "Child: Pause returned -1 but errno is %d (%s)\n",
		        TEST_ERRNO, strerror(TEST_ERRNO));
		exit(1);
	}

	fprintf(stderr, "Child: Pause returned %ld\n", TEST_RETURN);
	exit(1);
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
