/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
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
 */

/*
 *	wait401 - check that a call to wait4() correctly waits for a child
 *		  process to exit
 */

#include "test.h"

#include <errno.h>
#define _USE_BSD
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>

char *TCID = "wait401";
int TST_TOTAL = 1;

static void cleanup(void);
static void setup(void);

int main(int ac, char **av)
{
	int lc;
	pid_t pid;
	int status = 1;
	struct rusage rusage;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		pid = FORK_OR_VFORK();

		switch (pid) {
		case -1:
			tst_brkm(TBROK, cleanup, "fork() failed");
			break;
		case 0:
			sleep(1);
			exit(0);
			break;
		default:
			TEST(wait4(pid, &status, 0, &rusage));
			break;
		}

		if (TEST_RETURN == -1) {
			tst_brkm(TFAIL, cleanup, "%s call failed - errno = %d "
				 ": %s", TCID, TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

		if (WIFEXITED(status) == 0) {
			tst_brkm(TFAIL, cleanup,
				 "%s call succeeded but "
				 "WIFEXITED() did not return expected value "
				 "- %d", TCID, WIFEXITED(status));
		} else if (TEST_RETURN != pid) {
			tst_resm(TFAIL, "%s did not return the "
				 "expected value (%d), actual: %ld",
				 TCID, pid, TEST_RETURN);
		} else {

			tst_resm(TPASS,
				 "Received child pid as expected.");
		}

		tst_resm(TPASS, "%s call succeeded", TCID);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
