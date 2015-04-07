/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
 *    DATE STARTED	: 06/01/02
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
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
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

static void setup(void);

char *TCID = "execv01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;
	pid_t pid;
	char *const args[] = { "execv01_child", "canary", NULL};
	char path[2048];

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	if (tst_get_path("execv01_child", path, sizeof(path)))
		tst_brkm(TCONF, NULL, "Couldn't find execv01_child in $PATH");

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		switch (pid = FORK_OR_VFORK()) {
		case 0:
			execv(path, args);
			tst_brkm(TFAIL | TERRNO, NULL,
			         "Failed to execute execv01_child");
		case -1:
			tst_brkm(TBROK, NULL, "fork failed");
		default:
			tst_record_childstatus(NULL, pid);
		}
	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, NULL);

	TEST_PAUSE;
}
