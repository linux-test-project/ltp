// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <chrbis@suse.cz>
 *
 * After fix for CVE-2018-1000199 (see ptrace08.c) subsequent calls to POKEUSER
 * for x86 debug registers were ignored silently.
 *
 * This is a regression test for commit:
 *
 * commit bd14406b78e6daa1ea3c1673bda1ffc9efdeead0
 * Author: Jiri Olsa <jolsa@kernel.org>
 * Date:   Mon Aug 27 11:12:25 2018 +0200
 *
 *     perf/hw_breakpoint: Modify breakpoint even if the new attr has disabled set
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <signal.h>
#include "tst_test.h"

static pid_t child_pid;

static void child_main(void)
{
	raise(SIGSTOP);
	exit(0);
}

static void run(void)
{
	int status;
	unsigned long addr;

	child_pid = SAFE_FORK();

	if (!child_pid)
		child_main();

	if (SAFE_WAITPID(child_pid, &status, WUNTRACED) != child_pid)
		tst_brk(TBROK, "Received event from unexpected PID");

#if defined(__i386__) || defined(__x86_64__)
	SAFE_PTRACE(PTRACE_ATTACH, child_pid, NULL, NULL);
	SAFE_PTRACE(PTRACE_POKEUSER, child_pid,
		(void *)offsetof(struct user, u_debugreg[0]), (void *)1);
	SAFE_PTRACE(PTRACE_POKEUSER, child_pid,
		(void *)offsetof(struct user, u_debugreg[0]), (void *)2);

	addr = ptrace(PTRACE_PEEKUSER, child_pid,
	              (void*)offsetof(struct user, u_debugreg[0]), NULL);
#endif

	if (addr == 2)
		tst_res(TPASS, "The rd0 was set on second PTRACE_POKEUSR");
	else
		tst_res(TFAIL, "The rd0 wasn't set on second PTRACE_POKEUSER");

	SAFE_PTRACE(PTRACE_DETACH, child_pid, NULL, NULL);
	SAFE_KILL(child_pid, SIGCONT);
	child_pid = 0;
	tst_reap_children();
}

static void cleanup(void)
{
	/* Main process terminated by tst_brk() with child still paused */
	if (child_pid)
		SAFE_KILL(child_pid, SIGKILL);
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = cleanup,
	.forks_child = 1,
	.supported_archs = (const char *const []) {
		"x86",
		"x86_64",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "bd14406b78e6"},
		{}
	}
};
