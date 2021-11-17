// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 SUSE LLC <nstange@suse.de>
 * Copyright (C) 2020 SUSE LLC <mdoucha@suse.cz>
 *
 * CVE-2018-8897
 *
 * Test that the MOV SS instruction touching a ptrace watchpoint followed by
 * INT3 breakpoint is handled correctly by the kernel. Kernel crash fixed in:
 *
 *  commit d8ba61ba58c88d5207c1ba2f7d9a2280e7d03be9
 *  Author: Andy Lutomirski <luto@kernel.org>
 *  Date:   Thu Jul 23 15:37:48 2015 -0700
 *
 *  x86/entry/64: Don't use IST entry for #BP stack
 */

#include <stdlib.h>
#include <stddef.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <signal.h>
#include "tst_test.h"

static short watchpoint;
static pid_t child_pid;

static int child_main(void)
{
	SAFE_PTRACE(PTRACE_TRACEME, 0, NULL, NULL);
	raise(SIGSTOP);
	/* wait for SIGCONT from parent */

	asm volatile(
		"mov %%ss, %0\n"
		"mov %0, %%ss\n"
		"int $3\n"
		: "+m" (watchpoint)
	);

	return 0;
}

static void run(void)
{
	int status;

#if defined(__i386__) || defined(__x86_64__)
	child_pid = SAFE_FORK();

	if (!child_pid) {
		exit(child_main());
	}

	if (SAFE_WAITPID(child_pid, &status, 0) != child_pid)
		tst_brk(TBROK, "Received event from unexpected PID");

	SAFE_PTRACE(PTRACE_POKEUSER, child_pid,
		(void *)offsetof(struct user, u_debugreg[0]), &watchpoint);
	SAFE_PTRACE(PTRACE_POKEUSER, child_pid,
		(void *)offsetof(struct user, u_debugreg[7]), (void *)0x30001);
	SAFE_PTRACE(PTRACE_CONT, child_pid, NULL, NULL);
#endif

	while (1) {
		if (SAFE_WAITPID(child_pid, &status, 0) != child_pid)
			tst_brk(TBROK, "Received event from unexpected PID");

		if (WIFEXITED(status)) {
			child_pid = 0;
			break;
		}

		if (WIFSTOPPED(status)) {
			SAFE_PTRACE(PTRACE_CONT, child_pid, NULL, NULL);
			continue;
		}

		tst_brk(TBROK, "Unexpected event from child");
	}

	tst_res(TPASS, "We're still here. Nothing bad happened, probably.");
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
		{"linux-git", "d8ba61ba58c8"},
		{"CVE", "2018-8897"},
		{}
	}
};
