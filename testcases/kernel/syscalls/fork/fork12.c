/*
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *
 * NAME
 *	fork12.c
 *
 * DESCRIPTION
 *	Check that all children inherit parent's file descriptor
 *
 * ALGORITHM
 *	Parent forks processes until -1 is returned.$
 *
 * USAGE
 *	fork12
 *	** CAUTION ** Can hang your machine, esp prior to 2.4.19
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	07/2002 Split from fork07 as a test case to exhaust available pids.
 *
 * RESTRICTIONS
 *	Should be run as root to avoid resource limits.$
 *	Should not be run with other test programs because it tries to
 *	  use all available pids.
 */

#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "fork12";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);
static void fork12_sigs(int signum);

int main(int ac, char **av)
{
	int forks, pid1, fork_errno, waitstatus;
	int ret, status;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		tst_resm(TINFO, "Forking as many kids as possible");
		forks = 0;
		while ((pid1 = fork()) != -1) {
			if (pid1 == 0) {	/* child */
				/*
				 * Taunt the OOM killer so that it doesn't
				 * kill system processes
				 */
				SAFE_FILE_PRINTF(NULL,
					"/proc/self/oom_score_adj", "500");
				pause();
				exit(0);
			}
			forks++;
			ret = SAFE_WAITPID(cleanup, -1, &status, WNOHANG);
			if (ret > 0) {
				/* a child may be killed by OOM killer */
				if (WTERMSIG(status) == SIGKILL)
					break;
				tst_brkm(TBROK, cleanup,
					 "child exit with error code %d or signal %d",
					 WEXITSTATUS(status), WTERMSIG(status));
			}
		}
		fork_errno = errno;

		/* parent */
		tst_resm(TINFO, "Number of processes forked is %d", forks);
		tst_resm(TPASS, "fork() eventually failed with %d: %s",
			 fork_errno, strerror(fork_errno));
		/* collect our kids */
		/*
		 * Introducing a sleep(3) to make sure all children are
		 * at pause() when SIGQUIT is sent to them
		 */
		sleep(3);
		kill(0, SIGQUIT);
		while (wait(&waitstatus) > 0) ;

	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, fork12_sigs, cleanup);
	TEST_PAUSE;
}

static void cleanup(void)
{
	int waitstatus;

	/* collect our kids */
	kill(0, SIGQUIT);
	while (wait(&waitstatus) > 0) ;
}

static void fork12_sigs(int signum)
{
	if (signum == SIGQUIT) {
		/* Children will continue, parent will ignore */
	} else {
		tst_brkm(TBROK, cleanup,
			 "Unexpected signal %d received.", signum);
	}
}
