// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
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
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"

#define TIMEOUT	5

static void do_child(void);
static void sigproc(int sig);
static volatile sig_atomic_t end;
static char init_cwd[PATH_MAX];

static void verify_getcwd(void)
{
	int status;
	char cur_cwd[PATH_MAX];
	pid_t child;

	child = SAFE_FORK();
	if (child == 0)
		do_child();

	 while (1) {
		SAFE_GETCWD(cur_cwd, PATH_MAX);
		if (strncmp(init_cwd, cur_cwd, PATH_MAX)) {
			tst_res(TFAIL, "initial current work directory is "
				 "%s, now is %s. Bug is reproduced!",
				 init_cwd, cur_cwd);
			break;
		}

		if (end) {
			tst_res(TPASS, "Bug is not reproduced!");
			break;
		}
	}

	SAFE_KILL(child, SIGKILL);
	SAFE_WAITPID(child, &status, 0);
}

static void setup(void)
{
	if (tst_ncpus() == 1)
		tst_brk(TCONF, "This test needs two cpus at least");

	SAFE_SIGNAL(SIGALRM, sigproc);

	alarm(TIMEOUT);

	SAFE_GETCWD(init_cwd, PATH_MAX);
}

static void sigproc(int sig)
{
	end = sig;
}

static void do_child(void)
{
	unsigned int i = 0;
	char c_name[PATH_MAX] = "testfile", n_name[PATH_MAX];

	SAFE_TOUCH(c_name, 0644, NULL);

	while (1) {
		snprintf(n_name, PATH_MAX, "testfile%u", i++);
		SAFE_RENAME(c_name, n_name);
		strncpy(c_name, n_name, PATH_MAX);
	}
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_getcwd,
	.needs_tmpdir = 1,
	.forks_child = 1
};
