// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Andrew Lutomirski
 * Copyright (C) 2020 SUSE LLC <mdoucha@suse.cz>
 *
 * CVE-2018-1000199
 *
 * Test error handling when ptrace(POKEUSER) modified x86 debug registers even
 * when the call returned error.
 *
 * When the bug was present we could create breakpoint in the kernel code,
 * which shoudn't be possible at all. The original CVE caused a kernel crash by
 * setting a breakpoint on do_debug kernel function which, when triggered,
 * caused an infinite loop. However we do not have to crash the kernel in order
 * to assert if kernel has been fixed or not.
 *
 * On newer kernels all we have to do is to try to set a breakpoint, on any
 * kernel address, then read it back and check if the value has been set or
 * not.
 *
 * The original fix to the CVE however disabled a breakpoint on address change
 * and the check was deffered to write dr7 that enabled the breakpoint again.
 * So on older kernels we have to write to dr7 which should fail instead.
 *
 * Kernel crash partially fixed in:
 *
 *  commit f67b15037a7a50c57f72e69a6d59941ad90a0f0f
 *  Author: Linus Torvalds <torvalds@linux-foundation.org>
 *  Date:   Mon Mar 26 15:39:07 2018 -1000
 *
 *  perf/hwbp: Simplify the perf-hwbp code, fix documentation
 *
 * On Centos7, this is also a regression test for
 * commit 27747f8bc355 ("perf/x86/hw_breakpoints: Fix check for kernel-space breakpoints").
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <signal.h>
#include "tst_test.h"
#include "tst_safe_stdio.h"

static pid_t child_pid;

#if defined(__i386__)
# define KERN_ADDR_MIN 0xc0000000
# define KERN_ADDR_MAX 0xffffffff
# define KERN_ADDR_BITS 32
#else
# define KERN_ADDR_MIN 0xffff800000000000
# define KERN_ADDR_MAX 0xffffffffffffffff
# define KERN_ADDR_BITS 64
#endif


static void child_main(void)
{
	raise(SIGSTOP);
	exit(0);
}

static void ptrace_try_kern_addr(unsigned long kern_addr)
{
	int status;
	unsigned long addr;

	tst_res(TINFO, "Trying address 0x%lx", kern_addr);

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
		(void *)offsetof(struct user, u_debugreg[7]), (void *)1);

	TEST(ptrace(PTRACE_POKEUSER, child_pid,
		(void *)offsetof(struct user, u_debugreg[0]),
		(void *)kern_addr));

	if (TST_RET == -1) {
		addr = ptrace(PTRACE_PEEKUSER, child_pid,
					  (void*)offsetof(struct user, u_debugreg[0]), NULL);
		if (addr == kern_addr) {
			TEST(ptrace(PTRACE_POKEUSER, child_pid,
				(void *)offsetof(struct user, u_debugreg[7]), (void *)1));
		}
	}

	if (TST_RET != -1) {
		tst_res(TFAIL, "ptrace() breakpoint with kernel addr succeeded");
	} else {
		if (TST_ERR == EINVAL) {
			tst_res(TPASS | TTERRNO,
				"ptrace() breakpoint with kernel addr failed");
		} else {
			tst_res(TFAIL | TTERRNO,
				"ptrace() breakpoint on kernel addr should return EINVAL, got");
		}
	}

#endif

	SAFE_PTRACE(PTRACE_DETACH, child_pid, NULL, NULL);
	SAFE_KILL(child_pid, SIGCONT);
	child_pid = 0;
	tst_reap_children();
}

static void run(void)
{
	ptrace_try_kern_addr(KERN_ADDR_MIN);
	ptrace_try_kern_addr(KERN_ADDR_MAX);
	ptrace_try_kern_addr(KERN_ADDR_MIN + (KERN_ADDR_MAX - KERN_ADDR_MIN)/2);
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
	/*
	 * When running in compat mode we can't pass 64 address to ptrace so we
	 * have to skip the test.
	 */
	.skip_in_compat = 1,
	.supported_archs = (const char *const []) {
		"x86",
		"x86_64",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f67b15037a7a"},
		{"CVE", "2018-1000199"},
		{"linux-git", "27747f8bc355"},
		{}
	}
};
