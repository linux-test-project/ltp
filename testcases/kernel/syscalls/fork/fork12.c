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
 * 	fork12.c
 *
 * DESCRIPTION
 *	Check that all children inherit parent's file descriptor
 *
 * ALGORITHM
 * 	Parent forks processes until -1 is returned.$
 *$
 * USAGE
 * 	fork12
 * 	** CAUTION ** Can hang your machine, esp prior to 2.4.19
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	07/2002 Split from fork07 as a test case to exhaust available pids.
 *
 * RESTRICTIONS
 * 	Should be run as root to avoid resource limits.$
 * 	Should not be run with other test programs because it tries to
 * 	  use all available pids.
 */

#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

char *TCID = "fork12";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);
void fork12_sigs(int signum);

char pbuf[10];

int main(int ac, char **av)
{
	int forks, pid1, fork_errno, waitstatus;
	int ret, status;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	 }

	/*
	 * perform global setup for the test
	 */
	setup();

	/*
	 * check looping state if -i option is given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/*
		 * reset Tst_count in case we are looping.
		 */
		Tst_count = 0;

		tst_resm(TINFO, "Forking as many kids as possible");
		forks = 0;
		while ((pid1 = fork()) != -1) {
			if (pid1 == 0) {	/* child */
				pause();
				exit(0);
			}
			forks++;
			ret = waitpid(-1, &status, WNOHANG);
			if (ret < 0)
				tst_brkm(TBROK, cleanup,
					 "waitpid failed %d: %s\n", errno,
					 strerror(errno));
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
		sleep(3);	//Introducing a sleep(3) to make sure all children are at pause() when SIGQUIT is sent to them
		kill(0, SIGQUIT);
		while (wait(&waitstatus) > 0) ;

	}
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{
	/*
	 * capture signals
	 */
	tst_sig(FORK, fork12_sigs, cleanup);

	/*
	 * Pause if that option was specified
	 */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit
 */
void cleanup()
{
	int waitstatus;

	/* collect our kids */
	kill(0, SIGQUIT);
	while (wait(&waitstatus) > 0) ;
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}

void fork12_sigs(int signum)
{
	if (signum == SIGQUIT) {
		/* Children will continue, parent will ignore */
	} else
		tst_brkm(TBROK, cleanup,
		    "Unexpected signal %d received.", signum);
}
