/*
 *   Copyright (c) 2014 Fujitsu Ltd.
 *   Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
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
 */

/*
 * Note: this test has already been in xfstests generic/028 test case,
 * I just port it to LTP.
 *
 * Kernel commit '232d2d60aa5469bb097f55728f65146bd49c1d25' introduced a race
 * condition that causes getcwd(2) to return "/" instead of correct path.
 *     232d2d6 dcache: Translating dentry into pathname without
 *             taking rename_lock
 *
 * And these two kernel commits fixed the bug:
 *   ede4cebce16f5643c61aedd6d88d9070a1d23a68
 *	prepend_path() needs to reinitialize dentry/vfsmount/mnt on restarts
 *   f6500801522c61782d4990fa1ad96154cb397cd4
 *	f650080 __dentry_path() fixes
 *
 * This test is to check whether this bug exists in the running kernel,
 * or whether this bug has been fixed.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"

#define TIMEOUT	5

static void setup(void);
static void cleanup(void);
static void do_child(void);
static void sigproc(int sig);
static volatile sig_atomic_t end;
static char init_cwd[PATH_MAX];

char *TCID = "getcwd04";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int status;
	char cur_cwd[PATH_MAX];
	pid_t child;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	child = tst_fork();
	if (child < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fork failed");

	if (child == 0)
		do_child();

	 while (1) {
		SAFE_GETCWD(cleanup, cur_cwd, PATH_MAX);
		if (strncmp(init_cwd, cur_cwd, PATH_MAX)) {
			tst_resm(TFAIL, "initial current work directory is "
				 "%s, now is %s. Bug is reproduced!",
				 init_cwd, cur_cwd);
			break;
		}

		if (end) {
			tst_resm(TPASS, "Bug is not reproduced!");
			break;
		}
	}

	SAFE_KILL(cleanup, child, SIGKILL);
	SAFE_WAITPID(cleanup, child, &status, 0);

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	if (tst_ncpus() == 1)
		tst_brkm(TCONF, NULL, "This test needs two cpus at least");

	tst_tmpdir();

	if (signal(SIGALRM, sigproc) == SIG_ERR)
		tst_brkm(TBROK | TERRNO, cleanup, "signal(SIGALRM) failed");

	alarm(TIMEOUT);

	SAFE_GETCWD(cleanup, init_cwd, PATH_MAX);
}

static void sigproc(int sig)
{
	end = sig;
}

static void do_child(void)
{
	unsigned int i = 0;
	char c_name[PATH_MAX] = "testfile", n_name[PATH_MAX];

	SAFE_TOUCH(NULL, c_name, 0644, NULL);

	while (1) {
		snprintf(n_name, PATH_MAX, "testfile%u", i++);
		SAFE_RENAME(NULL, c_name, n_name);
		strncpy(c_name, n_name, PATH_MAX);
	}
}

static void cleanup(void)
{
	tst_rmdir();
}
