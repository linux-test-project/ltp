/* Copyright (c) 2014 Red Hat, Inc. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 * File: pidns03.c
 *
 * Description:
 * Clones a new child process with CLONE_NEWPID flag - the new child
 * process mounts procfs to a "proc" directory and checks if it belongs
 * to a new pid namespace by:
 * 1. reading value of "proc/self", which is symlink
 *    to directory named after current pid number
 * 2. comparing read value (PID) with "1"
 */

#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "pidns_helper.h"
#include "test.h"
#include "safe_macros.h"

#define PROCDIR "proc"
char *TCID = "pidns03";
int TST_TOTAL	= 1;


static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
	tst_tmpdir();
	SAFE_MKDIR(cleanup, PROCDIR, 0555);
}

int child_func(void *arg)
{
	ssize_t r;
	char buf[10];

	if (mount("none", PROCDIR, "proc", MS_RDONLY, NULL) == -1) {
		perror("mount");
		return 1;
	}

	/* self is symlink to directory named after current pid number */
	r = readlink(PROCDIR"/self", buf, sizeof(buf)-1);
	if (r == -1) {
		perror("readlink");
		umount(PROCDIR);
		return 1;
	}

	buf[r] = '\0';

	umount(PROCDIR);

	/* child should have PID 1 in a new pid namespace - if true
	 * procfs belongs to the new pid namespace */
	if (strcmp(buf, "1")) {
		fprintf(stderr, "%s contains: %s\n", PROCDIR"/self", buf);
		return 1;
	}

	return 0;
}

static void test(void)
{
	int status;

	if (do_clone_tests(CLONE_NEWPID, child_func, NULL, NULL, NULL) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");

	SAFE_WAIT(cleanup, &status);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		tst_resm(TPASS, "mounting procfs in a new namespace");
		return;
	}

	if (WIFSIGNALED(status)) {
		tst_resm(TFAIL, "child was killed with signal %s",
			 tst_strsig(WTERMSIG(status)));
		return;
	}

	tst_resm(TFAIL, "mounting procfs in a new namespace");
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		test();

	cleanup();
	tst_exit();
}
