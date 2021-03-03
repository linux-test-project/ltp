// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (c) International Business Machines Corp., 2001
 *
 * Ported to LTP: Wayne Boyer
 *  21/04/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 */

/*
 * NAME
 *	execve05.c
 *
 * DESCRIPTION
 *	This testcase tests the basic functionality of the execve(2) system
 *	call.
 *
 * ALGORITHM
 *	This tests the functionality of the execve(2) system call by spawning
 *	a few children, each of which would execute "execve_child" simultaneously,
 *	and finally the parent ensures that they terminated correctly.
 *
 * USAGE
 *	execve05 -i 5 -n 20
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>

#include "tst_test.h"

#define TEST_APP "execve_child"

static int nchild = 8;

static char *opt_nchild;

static void do_child(void)
{
	char *argv[3] = {TEST_APP, "canary", NULL};

	TST_CHECKPOINT_WAIT(0);

	TEST(execve(TEST_APP, argv, environ));
	tst_res(TFAIL | TTERRNO, "execve() returned unexpected errno");
}

static void verify_execve(void)
{
	int i;

	for (i = 0; i < nchild; i++) {
		if (SAFE_FORK() == 0)
			do_child();
	}

	TST_CHECKPOINT_WAKE2(0, nchild);
}

static void setup(void)
{
	if (opt_nchild)
		nchild = SAFE_STRTOL(opt_nchild, 1, INT_MAX);
}

static struct tst_test test = {
	.test_all = verify_execve,
	.options = (struct tst_option[]) {
		{"n:", &opt_nchild, "Numbers of children"},
		{}
	},
	.forks_child = 1,
	.child_needs_reinit = 1,
	.needs_checkpoints = 1,
	.resource_files = (const char *const []) {
		TEST_APP,
		NULL
	},
	.setup = setup,
};
