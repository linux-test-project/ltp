// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <rpalethorpe@suse.com>
 */
/*\
 *
 * [Description]
 *
 * This will reproduce an information leak in the set_mempolicy 32-bit
 * compat syscall. The catch is that the 32-bit compat syscall is not
 * used in x86_64 upstream. So at the time of writing, 32-bit programs
 * on large x86_64 numa systems will be broken if they use
 * set_mempolicy. OTOH they could not have been exploited either.
 *
 * On other architectures the compat syscall is connected. Including
 * PowerPC which has also been included as well. It is possible some
 * vendors connected the x86_64 compat call in their kernel branch.
 *
 * The kernel allocates memory from the user's stack as a temporary
 * work area. Allowing it to copy the node array of 32-bit fields to
 * 64-bit fields. It uses user memory so that it can share the
 * non-compatability syscall functions which use copy_from_user()
 * internally.
 *
 * Originally the compat call would copy a chunk of the
 * uninitialized kernel stack to the user stack before checking the
 * validation result. This meant when the user passed in an invalid
 * node_mask_ptr. They would get kernel stack data somewhere below
 * their stack pointer.
 *
 * So we allocate and set an array on the stack (larger than any
 * redzone). Then move the stack pointer to the beginning of the
 * array. Then move it back after the syscall. We can then check to
 * see if the array has been modified.
 */

#include "config.h"
#include "tst_test.h"

#include <string.h>

static unsigned int i;
static int sys_ret;
static volatile char *stack_ptr;

static void run(void)
{
#ifdef __powerpc__
	register long sys_num __asm__("r0");
	register long mode __asm__("r3");
	register long node_mask_ptr __asm__("r4");
	register long node_mask_sz __asm__("r5");
#else
	const int sys_num = 276;
	const int mode;
	const int node_mask_ptr = UINT_MAX;
	const int node_mask_sz = UINT_MAX;
#endif
	char stack_pattern[0x400];

	stack_ptr = stack_pattern;
	memset(stack_pattern, 0xA5, sizeof(stack_pattern));
	tst_res(TINFO, "stack pattern is in %p-%p", stack_ptr, stack_ptr + 0x400);

#ifdef __powerpc__
	sys_num = 261;
	mode = 0;
	node_mask_ptr = ~0UL;
	node_mask_sz = ~0UL;
	asm volatile (
		"addi 1,1,1024\n\t"
		"sc\n\t"
		"addi 1,1,-1024\n\t" :
		"+r"(sys_num), "+r"(mode), "+r"(node_mask_ptr), "+r"(node_mask_sz) :
		:
		"memory", "cr0", "r6", "r7", "r8", "r9", "r10", "r11", "r12");
	sys_ret = mode;
#endif
#ifdef __i386__
	asm volatile (
		"add $0x400, %%esp\n\t"
		"int $0x80\n\t"
		"sub $0x400, %%esp\n\t" :
		"=a"(sys_ret) :
		"a"(sys_num), "b"(mode), "c"(node_mask_ptr), "d"(node_mask_sz) :
		"memory");
	sys_ret = -sys_ret;
#endif

	for (i = 0; i < sizeof(stack_pattern); i++) {
		if (stack_ptr[i] != (char)0xA5) {
			tst_brk(TFAIL,
				"User stack was overwritten with something at %d", i);
		}
	}

	switch (sys_ret) {
	case EFAULT:
		tst_res(TPASS,
			"set_mempolicy returned EFAULT (compat assumed)");
		break;
	case EINVAL:
		tst_res(TCONF,
			"set_mempolicy returned EINVAL (non compat assumed)");
		break;
	default:
		tst_res(TFAIL,
			"set_mempolicy should fail with EFAULT or EINVAL, instead returned %ld",
			(long)sys_ret);
	}
}

static struct tst_test test = {
	.test_all = run,
	.supported_archs = (const char *const []) {
		"x86",
		"ppc",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "cf01fb9985e8"},
		{"CVE", "CVE-2017-7616"},
		{}
	}
};
