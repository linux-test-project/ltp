// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *     Author: Ulrich Drepper / Nate Straz , Red Hat
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that LDT is propagated correctly from parent process to
 * the child process.
 *
 * On Friday, May 2, 2003 at 09:47:00AM MST, Ulrich Drepper wrote:
 *
 *  Robert Williamson wrote:
 *
 *  I'm getting a SIGSEGV with one of our tests, fork05.c, that apparently
 *  you wrote (attached below).  The test passes on my 2.5.68 machine running
 *  SuSE 8.0 (glibc 2.2.5 and Linuxthreads), however it segmentation faults on
 *  RedHat 9 running 2.5.68.  The test seems to "break" when it attempts to run
 *  the assembly code....could you take a look at it?
 *
 *  There is no need to look at it, I know it cannot work anymore on recent
 *  systems.  Either change all uses of %gs to %fs or skip the entire patch
 *  if %gs has a nonzero value.
 *
 * On Sat, Aug 12, 2000 at 12:47:31PM -0700, Ulrich Drepper wrote:
 *
 *  Ever since the %gs handling was fixed in the 2.3.99 series the
 *  appended test program worked.  Now with 2.4.0-test6 it's not working
 *  again.  Looking briefly over the patch from test5 to test6 I haven't
 *  seen an immediate candidate for the breakage.  It could be missing
 *  propagation of the LDT to the new process (and therefore an invalid
 *  segment descriptor) or simply clearing %gs.
 *
 *  Anyway, this is what you should see and what you get with test5:
 *
 *  a = 42
 *  %gs = 0x0007
 *  %gs = 0x0007
 *  a = 99
 *
 *  This is what you get with test6:
 *
 *  a = 42
 *  %gs = 0x0007
 *  %gs = 0x0000
 *  <SEGFAULT>
 *
 *  If somebody is actually creating a test suite for the kernel, please
 *  add this program.  It's mostly self-contained.  The correct handling
 *  of %gs is really important since glibc 2.2 will make heavy use of it.
 */

#include "tst_test.h"

#if defined(__i386__)

#include "lapi/syscalls.h"
#include <asm/ldt.h>

static void run(void)
{
	struct user_desc ldt0;
	int base_addr = 42;
	int status;
	pid_t pid;
	int lo;

	ldt0.entry_number = 0;
	ldt0.base_addr = (long)&base_addr;
	ldt0.limit = 4;
	ldt0.seg_32bit = 1;
	ldt0.contents = 0;
	ldt0.read_exec_only = 0;
	ldt0.limit_in_pages = 0;
	ldt0.seg_not_present = 0;
	ldt0.useable = 1;

	tst_syscall(__NR_modify_ldt, 1, &ldt0, sizeof(ldt0));

	asm volatile ("movw %w0, %%fs"::"q" (7));
	asm volatile ("movl %%fs:0, %0":"=r" (lo));
	tst_res(TINFO, "a = %d", lo);

	asm volatile ("pushl %%fs; popl %0":"=q" (lo));
	tst_res(TINFO, "%%fs = %#06hx", lo);

	asm volatile ("movl %0, %%fs:0"::"r" (99));

	pid = SAFE_FORK();
	if (!pid) {
		asm volatile ("pushl %%fs; popl %0":"=q" (lo));
		tst_res(TINFO, "%%fs = %#06hx", lo);

		asm volatile ("movl %%fs:0, %0":"=r" (lo));
		tst_res(TINFO, "a = %d", lo);

		TST_EXP_EQ_LI(lo, 99);

		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFEXITED(status) && !WEXITSTATUS(status))
		tst_res(TPASS, "Child did exit with 0");
	else
		tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};

#else
	TST_TEST_TCONF("Test only supports Intel 32 bits");
#endif
