// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Google, Inc.
 */

/*
 * Regression test for commit 814fb7bb7db5 ("x86/fpu: Don't let userspace set
 * bogus xcomp_bv"), or CVE-2017-15537.  This bug allowed ptrace(pid,
 * PTRACE_SETREGSET, NT_X86_XSTATE, &iov) to assign a task an invalid FPU state
 * --- specifically, by setting reserved bits in xstate_header.xcomp_bv.  This
 * made restoring the FPU registers fail when switching to the task, causing the
 * FPU registers to take on the values from other tasks.
 *
 * To detect the bug, we have a subprocess run a loop checking its xmm0 register
 * for corruption.  This detects the case where the FPU state became invalid and
 * the kernel is not restoring the process's registers.  Note that we have to
 * set the expected value of xmm0 to all 0's since it is acceptable behavior for
 * the kernel to simply reinitialize the FPU state upon seeing that it is
 * invalid.  To increase the chance of detecting the problem, we also create
 * additional subprocesses that spin with different xmm0 contents.
 *
 * Thus bug affected the x86 architecture only.  Other architectures could have
 * similar bugs as well, but this test has to be x86-specific because it has to
 * know about the architecture-dependent FPU state.
 */

#include <errno.h>
#include <inttypes.h>
#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include "config.h"
#include "ptrace.h"
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/cpuid.h"

#ifndef PTRACE_GETREGSET
# define PTRACE_GETREGSET 0x4204
#endif

#ifndef PTRACE_SETREGSET
# define PTRACE_SETREGSET 0x4205
#endif

#ifndef NT_X86_XSTATE
# define NT_X86_XSTATE 0x202
#endif

#ifndef CPUID_LEAF_XSTATE
# define CPUID_LEAF_XSTATE 0xd
#endif

static void check_regs_loop(uint32_t initval)
{
	const unsigned long num_iters = 1000000000;
	uint32_t xmm0[4] = { initval, initval, initval, initval };
	int status = 1;

#ifdef __x86_64__
	asm volatile("   movdqu %0, %%xmm0\n"
		     "   mov %0, %%rbx\n"
		     "1: dec %2\n"
		     "   jz 2f\n"
		     "   movdqu %%xmm0, %0\n"
		     "   mov %0, %%rax\n"
		     "   cmp %%rax, %%rbx\n"
		     "   je 1b\n"
		     "   jmp 3f\n"
		     "2: mov $0, %1\n"
		     "3:\n"
		     : "+m" (xmm0), "+r" (status)
		     : "r" (num_iters) : "rax", "rbx", "xmm0");
#endif

	if (status) {
		tst_res(TFAIL,
			"xmm registers corrupted!  initval=%08X, xmm0=%08X%08X%08X%08X\n",
			initval, xmm0[0], xmm0[1], xmm0[2], xmm0[3]);
	}
	exit(status);
}

static void do_test(void)
{
	int i;
	int num_cpus = tst_ncpus();
	pid_t pid;
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	uint64_t *xstate;
	/*
	 * CPUID.(EAX=0DH, ECX=0H):EBX: maximum size (bytes, from the beginning
	 * of the XSAVE/XRSTOR save area) required by enabled features in XCR0.
	 */
	__cpuid_count(CPUID_LEAF_XSTATE, ecx, eax, ebx, ecx, edx);
	xstate = SAFE_MEMALIGN(64, ebx);
	struct iovec iov = { .iov_base = xstate, .iov_len = ebx };
	int status;
	bool okay;

	tst_res(TINFO, "CPUID.(EAX=%u, ECX=0):EAX=%u, EBX=%u, ECX=%u, EDX=%u",
		CPUID_LEAF_XSTATE, eax, ebx, ecx, edx);
	pid = SAFE_FORK();
	if (pid == 0) {
		TST_CHECKPOINT_WAKE(0);
		check_regs_loop(0x00000000);
	}
	for (i = 0; i < num_cpus; i++) {
		if (SAFE_FORK() == 0)
			check_regs_loop(0xDEADBEEF);
	}

	TST_CHECKPOINT_WAIT(0);
	sched_yield();

	TEST(ptrace(PTRACE_ATTACH, pid, 0, 0));
	if (TST_RET != 0) {
		free(xstate);
		tst_brk(TBROK | TTERRNO, "PTRACE_ATTACH failed");
	}

	SAFE_WAITPID(pid, NULL, 0);
	TEST(ptrace(PTRACE_GETREGSET, pid, NT_X86_XSTATE, &iov));
	if (TST_RET != 0) {
		free(xstate);
		if (TST_ERR == EIO)
			tst_brk(TCONF, "GETREGSET/SETREGSET is unsupported");

		if (TST_ERR == EINVAL)
			tst_brk(TCONF, "NT_X86_XSTATE is unsupported");

		if (TST_ERR == ENODEV)
			tst_brk(TCONF, "CPU doesn't support XSAVE instruction");

		tst_brk(TBROK | TTERRNO,
			"PTRACE_GETREGSET failed with unexpected error");
	}

	xstate[65] = -1; /* sets all bits in xstate_header.xcomp_bv */

	/*
	 * Old kernels simply masked out all the reserved bits in the xstate
	 * header (causing the PTRACE_SETREGSET command here to succeed), while
	 * new kernels will reject them (causing the PTRACE_SETREGSET command
	 * here to fail with EINVAL).  We accept either behavior, as neither
	 * behavior reliably tells us whether the real bug (which we test for
	 * below in either case) is present.
	 */
	TEST(ptrace(PTRACE_SETREGSET, pid, NT_X86_XSTATE, &iov));
	if (TST_RET == 0) {
		tst_res(TINFO, "PTRACE_SETREGSET with reserved bits succeeded");
	} else if (TST_ERR == EINVAL) {
		tst_res(TINFO,
			"PTRACE_SETREGSET with reserved bits failed with EINVAL");
	} else {
		free(xstate);
		tst_brk(TBROK | TTERRNO,
			"PTRACE_SETREGSET failed with unexpected error");
	}

	/*
	 * It is possible for test child 'pid' to crash on AMD
	 * systems (e.g. AMD Opteron(TM) Processor 6234) with
	 * older kernels. This causes tracee to stop and sleep
	 * in ptrace_stop(). Without resuming the tracee, the
	 * test hangs at do_test()->tst_reap_children() called
	 * by the library. Use detach here, so we don't need to
	 * worry about potential stops after this point.
	 */
	TEST(ptrace(PTRACE_DETACH, pid, 0, 0));
	if (TST_RET != 0) {
		free(xstate);
		tst_brk(TBROK | TTERRNO, "PTRACE_DETACH failed");
	}

	/* If child 'pid' crashes, only report it as info. */
	SAFE_WAITPID(pid, &status, 0);
	if (WIFEXITED(status)) {
		tst_res(TINFO, "test child %d exited, retcode: %d",
			pid, WEXITSTATUS(status));
	}
	if (WIFSIGNALED(status)) {
		tst_res(TINFO, "test child %d exited, termsig: %d",
			pid, WTERMSIG(status));
	}

	okay = true;
	for (i = 0; i < num_cpus; i++) {
		SAFE_WAIT(&status);
		okay &= (WIFEXITED(status) && WEXITSTATUS(status) == 0);
	}
	if (okay)
		tst_res(TPASS, "wasn't able to set invalid FPU state");
	free(xstate);
}

static struct tst_test test = {
	.test_all = do_test,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.supported_archs = (const char *const []) {
		"x86_64",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "814fb7bb7db5"},
		{"CVE", "2017-15537"},
		{}
	}

};
