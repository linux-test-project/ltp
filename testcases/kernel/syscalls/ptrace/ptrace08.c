// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Andrew Lutomirski
 * Copyright (C) 2020 SUSE LLC <mdoucha@suse.cz>
 *
 * CVE-2018-1000199
 *
 * Test error handling when ptrace(POKEUSER) modifies debug registers.
 * Even if the call returns error, it may create breakpoint in kernel code.
 * Kernel crash partially fixed in:
 *
 *  commit f67b15037a7a50c57f72e69a6d59941ad90a0f0f
 *  Author: Linus Torvalds <torvalds@linux-foundation.org>
 *  Date:   Mon Mar 26 15:39:07 2018 -1000
 *
 *  perf/hwbp: Simplify the perf-hwbp code, fix documentation
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <signal.h>
#include "tst_test.h"
#include "tst_safe_stdio.h"

#if defined(__i386__) || defined(__x86_64__)
#define SYMNAME_SIZE 256
#define KERNEL_SYM "do_debug"

static unsigned long break_addr;
static pid_t child_pid;

static void setup(void)
{
	int fcount;
	char endl, symname[256];
	FILE *fr = SAFE_FOPEN("/proc/kallsyms", "r");

	/* Find address of do_debug() in /proc/kallsyms */
	do {
		fcount = fscanf(fr, "%lx %*c %255s%c", &break_addr, symname,
			&endl);

		if (fcount <= 0 && feof(fr))
			break;

		if (fcount < 2) {
			fclose(fr);
			tst_brk(TBROK, "Unexpected data in /proc/kallsyms %d", fcount);
		}

		if (fcount >= 3 && endl != '\n')
			while (!feof(fr) && fgetc(fr) != '\n');
	} while (!feof(fr) && strcmp(symname, KERNEL_SYM));

	SAFE_FCLOSE(fr);

	if (strcmp(symname, KERNEL_SYM))
		tst_brk(TBROK, "Cannot find address of kernel symbol \"%s\"",
			KERNEL_SYM);

	if (!break_addr)
		tst_brk(TCONF, "Addresses in /proc/kallsyms are hidden");

	tst_res(TINFO, "Kernel symbol \"%s\" found at 0x%lx", KERNEL_SYM,
		break_addr);
}

static void debug_trap(void)
{
	/* x86 instruction INT1 */
	asm volatile (".byte 0xf1");
}

static void child_main(void)
{
	raise(SIGSTOP);
	/* wait for SIGCONT from parent */
	debug_trap();
	exit(0);
}

static void run(void)
{
	int status;
	pid_t child;

	child = child_pid = SAFE_FORK();

	if (!child_pid) {
		child_main();
	}

	if (SAFE_WAITPID(child_pid, &status, WUNTRACED) != child_pid)
		tst_brk(TBROK, "Received event from unexpected PID");

	SAFE_PTRACE(PTRACE_ATTACH, child_pid, NULL, NULL);
	SAFE_PTRACE(PTRACE_POKEUSER, child_pid,
		(void *)offsetof(struct user, u_debugreg[0]), (void *)1);
	SAFE_PTRACE(PTRACE_POKEUSER, child_pid,
		(void *)offsetof(struct user, u_debugreg[7]), (void *)1);

	/* Return value intentionally ignored here */
	ptrace(PTRACE_POKEUSER, child_pid,
		(void *)offsetof(struct user, u_debugreg[0]),
		(void *)break_addr);

	SAFE_PTRACE(PTRACE_DETACH, child_pid, NULL, NULL);
	SAFE_KILL(child_pid, SIGCONT);
	child_pid = 0;

	if (SAFE_WAITPID(child, &status, 0) != child)
		tst_brk(TBROK, "Received event from unexpected PID");

	if (!WIFSIGNALED(status))
		tst_brk(TBROK, "Received unexpected event from child");

	tst_res(TPASS, "Child killed by %s", tst_strsig(WTERMSIG(status)));
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
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f67b15037a7a"},
		{"CVE", "2018-1000199"},
		{}
	}
};
#else
TST_TEST_TCONF("This test is only supported on x86 systems");
#endif
