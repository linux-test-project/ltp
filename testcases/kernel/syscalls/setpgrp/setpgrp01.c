/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include "test.h"

void setup();
void cleanup();

char *TCID = "setpgrp01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * TEST CASE:
		 *  Call the setpgrp system call
		 */
		TEST(setpgrp());

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL,
				 "setpgrp -  Call the setpgrp system call failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS,
				 "setpgrp -  Call the setpgrp system call returned %ld",
				 TEST_RETURN);
		}

	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	int pid, status;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/*
	 * Make sure current process is NOT a session or pgrp leader
	 */
	if (getpgrp() == getpid()) {
		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup,
				 "fork() in setup() failed - errno %d", errno);
		}

		if (pid != 0) {	/* parent - sits and waits */
			wait(&status);
			exit(WEXITSTATUS(status));
		}

	}
}

void cleanup(void)
{
}
