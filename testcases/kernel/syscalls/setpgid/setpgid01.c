/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
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
/* $Id: setpgid01.c,v 1.7 2009/11/02 13:57:18 subrata_modak Exp $ */

/*
 * Description:
 * Verify that:
 *   1. Basic functionality test for setpgid(2).
 *   2. Check functioning of setpgid(2) with pid = 0 and pgid = 0.
 */

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "test.h"

static void setup(void);
static void cleanup(void);

char *TCID = "setpgid01";

static void setpgid_test1(void);
static void setpgid_test2(void);
static void (*testfunc[])(void) = { setpgid_test1, setpgid_test2};
int TST_TOTAL = ARRAY_SIZE(testfunc);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
}

static void setpgid_test1(void)
{
	pid_t pgid, pid;

	pgid = getpgrp();
	pid = getpid();

	TEST(setpgid(pid, pgid));
	if (TEST_RETURN == -1 || getpgrp() != pgid) {
		tst_resm(TFAIL | TTERRNO, "test setpgid(%d, %d) fail",
			 pid, pgid);
	} else {
		tst_resm(TPASS, "test setpgid(%d, %d) success", pid, pgid);
	}
}

static int wait4child(pid_t child)
{
	int status;

	if (waitpid(child, &status, 0) == -1)
		tst_resm(TBROK|TERRNO, "waitpid");
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	else
		return status;
}

static void setpgid_test2(void)
{
	int ret;
	pid_t pgid, pid;

	pid = FORK_OR_VFORK();
	if (pid == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "fork()");

	if (pid != 0) {
		ret = wait4child(pid);
	} else {
		pid = getpid();
		TEST(setpgid(0, 0));
		pgid = getpgrp();
		if (TEST_RETURN == -1) {
			fprintf(stderr, "setpgid(0, 0) fails in "
				"child process: %s\n", strerror(TEST_ERRNO));
			exit(1);
		} else if (pgid != pid) {
			fprintf(stderr, "setpgid(0, 0) fails to make PGID"
				"equal to PID\n");
			exit(1);
		} else {
			exit(0);
		}
	}

	if (ret == 0)
		tst_resm(TPASS, "test setpgid(0, 0) success");
	else
		tst_resm(TFAIL, "test setpgid(0, 0) fail");
}


static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
