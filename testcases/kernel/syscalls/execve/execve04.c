/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 *  07/2001 Ported by Wayne Boyer
 *  04/2008 Roy Lee <roylee@andestech.com>
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
 * Attempt to execve(2) a file which is being opened by another process for
 * writing fails with ETXTBSY.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>

#include "test.h"
#include "safe_macros.h"

#define TEST_APP "execve_child"

char *TCID = "execve04";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);
static void do_child(void);

int main(int ac, char **av)
{
	int lc;
	pid_t pid;
	char *argv[2] = {TEST_APP, NULL};
	char *env[1] = {NULL};

	tst_parse_opts(ac, av, NULL, NULL);

#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork failed");
		} else if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0)
				tst_brkm(TBROK, cleanup, "self_exec failed");
#else
			do_child();
#endif
		}

		TST_SAFE_CHECKPOINT_WAIT(cleanup, 0);

		TEST(execve(TEST_APP, argv, env));

		if (TEST_ERRNO != ETXTBSY)
			tst_resm(TFAIL | TTERRNO, "execve succeeded, expected failure");
		else
			tst_resm(TPASS | TTERRNO, "execve failed as expected");

		TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);
		SAFE_WAIT(cleanup, NULL);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	char path[PATH_MAX];

	if (tst_get_path(TEST_APP, path, sizeof(path))) {
		tst_brkm(TBROK, NULL,
		         "Couldn't found "TEST_APP" binary in $PATH");
	}

	tst_tmpdir();

	TST_CHECKPOINT_INIT(tst_rmdir);

	SAFE_CP(tst_rmdir, path, ".");
}

static void cleanup(void)
{
	tst_rmdir();
}

static void do_child(void)
{
	int fd;

#ifdef UCLINUX
	TST_CHECKPOINT_INIT(NULL);
#endif

	if ((fd = open(TEST_APP, O_WRONLY)) == -1) {
		perror("open failed");
		exit(1);
	}

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	exit(0);
}
