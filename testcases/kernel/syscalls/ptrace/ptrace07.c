/*
 * Copyright (c) 2017 Google, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, if not, see <http://www.gnu.org/licenses/>.
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

#ifndef PTRACE_GETREGSET
# define PTRACE_GETREGSET 0x4204
#endif

#ifndef PTRACE_SETREGSET
# define PTRACE_SETREGSET 0x4205
#endif

#ifndef NT_X86_XSTATE
# define NT_X86_XSTATE 0x202
#endif

#ifdef __x86_64__
static void check_regs_loop(uint32_t initval)
{
	const unsigned long num_iters = 1000000000;
	uint32_t xmm0[4] = { initval, initval, initval, initval };
	int status = 1;

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
	uint64_t xstate[512];
	struct iovec iov = { .iov_base = xstate, .iov_len = sizeof(xstate) };
	int status;
	bool okay;

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
	if (TST_RET != 0)
		tst_brk(TBROK | TTERRNO, "PTRACE_ATTACH failed");

	SAFE_WAITPID(pid, NULL, 0);
	TEST(ptrace(PTRACE_GETREGSET, pid, NT_X86_XSTATE, &iov));
	if (TST_RET != 0) {
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
		tst_brk(TBROK | TTERRNO,
			"PTRACE_SETREGSET failed with unexpected error");
	}

	TEST(ptrace(PTRACE_CONT, pid, 0, 0));
	if (TST_RET != 0)
		tst_brk(TBROK | TTERRNO, "PTRACE_CONT failed");

	okay = true;
	for (i = 0; i < num_cpus + 1; i++) {
		SAFE_WAIT(&status);
		okay &= (WIFEXITED(status) && WEXITSTATUS(status) == 0);
	}
	if (okay)
		tst_res(TPASS, "wasn't able to set invalid FPU state");
}

static struct tst_test test = {
	.test_all = do_test,
	.forks_child = 1,
	.needs_checkpoints = 1,
};

#else /* !__x86_64__ */
	TST_TEST_TCONF("this test is only supported on x86_64");
#endif /* __x86_64__ */
