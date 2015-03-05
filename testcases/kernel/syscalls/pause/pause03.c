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
#include <wait.h>

#include "test.h"

static volatile int cflag;
static pid_t cpid;

char *TCID = "pause03";
int TST_TOTAL = 1;

static void do_child(void);
static void setup(void);
static void cleanup(void);
static void sig_handle(int sig);

int main(int ac, char **av)
{
	int lc;
	const char *msg;
	int status;
	int ret_val;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		cflag = 0;

		if ((cpid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (cpid == 0) {
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0)
				tst_brkm(TBROK, cleanup, "self_exec failed");
#else
			do_child();
#endif
		}

		/* sleep to ensure the child executes */
		sleep(1);

		/* Check for the value of cflag */
		if (cflag == 1) {
			/*
			 * Indicates that child terminated
			 * before receipt of SIGKILL signal.
			 */
			tst_brkm(TFAIL, cleanup,
				 "Child exited before SIGKILL signal");
		}

		/* Send the SIGKILL signal now */
		kill(cpid, SIGKILL);

		/* sleep to ensure the signal sent is effected */
		sleep(1);

		/* Verify again the value of cflag */
		if (cflag == 0) {
			/* Child still exists */
			tst_resm(TFAIL, "Child still exists, "
				 "pause() still active");
			cleanup();
		}

		ret_val = wait(&status);

		/*
		 * Verify that wait returned after child process termination
		 * due to receipt of SIGKILL signal.
		 */
		if (WTERMSIG(status) == SIGKILL) {
			ret_val = wait(&status);
			if ((ret_val == -1) && (errno == ECHILD)) {
				/*
				 * Child is no longer accessible and pause()
				 * functionality is successful.
				 */
				tst_resm(TPASS, "Functionality of "
					 "pause() is correct");
			} else {
				tst_resm(TFAIL, "wait() failed due to "
					 "unkown reason, ret_val=%d, "
					 "errno=%d", ret_val, errno);
			}
		} else {
			tst_resm(TFAIL, "Child terminated not due to "
				 "SIGKILL, errno = %d", errno);
		}

	}

	cleanup();
	tst_exit();

}

void do_child(void)
{
	TEST(pause());

	tst_resm(TFAIL, "Unexpected return from pause()");

	while (1) ;
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	signal(SIGCLD, sig_handle);
}

void sig_handle(int sig)
{
	cflag = 1;
}

void cleanup(void)
{
	/* Cleanup the child if still active */
	kill(cpid, SIGKILL);
}
