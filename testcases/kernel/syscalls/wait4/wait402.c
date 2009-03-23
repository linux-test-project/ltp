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
 *	wait402.c
 *
 * DESCRIPTION
 *	wait402 - check for ECHILD errno when using an illegal pid value
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call with an illegal pid value
 *	check the errno value
 *	  issue a PASS message if we get ECHILD
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  wait402 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#define _USE_BSD
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

void cleanup(void);
void setup(void);

char *TCID = "wait402";
int TST_TOTAL = 1;
extern int Tst_count;

int exp_enos[] = { 10, 0 };

/* Get the max number of message queues allowed on system */
static long get_pid_max()
{
	FILE *fp;
	char buff[512];

	/* Get the max number of message queues allowed on system */
	fp = fopen("/proc/sys/kernel/pid_max", "r");
	if (fp == NULL)
		tst_brkm(TBROK, cleanup,
			 "Could not open /proc/sys/kernel/pid_max");
	if (!fgets(buff, sizeof(buff), fp))
		tst_brkm(TBROK, cleanup,
			 "Could not read /proc/sys/kernel/pid_max");
	fclose(fp);

	return atol(buff);
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t pid;
	pid_t epid = get_pid_max() + 1;

	int status = 1;
	struct rusage *rusage = NULL;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * Allocate some space for the rusage structure.
		 */

		if ((rusage = (struct rusage *)malloc(sizeof(struct rusage)))
		    == NULL) {
			tst_brkm(TBROK, cleanup, "malloc() failed");
		}

		pid = FORK_OR_VFORK();

		if (pid == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (pid == 0) {	/* this is the child */
			/*
			 * sleep for a moment to let us do the test
			 */
			sleep(2);
			exit(0);
		} else {	/* this is the parent */
			/*
			 * call wait4 with the TEST() macro.  epid is set
			 * to an illegal positive value.  This should give
			 * an ECHILD error.
			 */
			TEST(wait4(epid, &status, 0, rusage));
		}

		if (TEST_RETURN == 0) {
			tst_brkm(TFAIL, cleanup,
				 "call failed to produce expected error - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case ECHILD:
			tst_resm(TPASS,
				 "received expected failure - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_brkm(TFAIL, cleanup,
				 "call failed to produce expected "
				 "error - errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

		/*
		 * Clean up things in case we are looping.
		 */
		free(rusage);
		rusage = NULL;
	}

	cleanup();

	 /*NOTREACHED*/ return 0;

}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void cleanup(void)
{
	wait(NULL);
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
