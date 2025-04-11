// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2020
 *	Email: code@zilogic.com
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify mmap() syscall using MAP_GROWSDOWN flag.
 *
 * [Algorithm]
 *
 * **Test 1**
 *
 * We assign the memory region partially allocated with MAP_GROWSDOWN flag to
 * a thread as a stack and expect the mapping to grow when we touch the
 * guard page by calling a recusive function in the thread that uses the
 * growable mapping as a stack.
 *
 * The kernel only grows the memory region when the stack pointer is within
 * guard page when the guard page is touched so simply faulting the guard
 * page will not cause the mapping to grow.
 *
 * Newer kernels does not allow a MAP_GROWSDOWN mapping to grow closer than
 * 'stack_guard_gap' pages to an existing mapping. So when we map the stack we
 * make sure there is enough of free address space before the lowest stack
 * address.
 *
 * Kernel default `stack_guard_gap` size is `256 * getpagesize()`.
 *
 * The stack memory map would look like::
 *
 *   |  -  -  -   reserved  size   -  -  -  |
 *
 *   +-- - - - --+------------+-------------+
 *   | 256 pages |  unmapped  |   mapped    |
 *   +-- - - - --+------------+-------------+
 *                            | mapped size |
 *   ^           |  -  -  stack size  -  -  |
 *   start
 *               ^                          ^
 *               stack bottom       stack top
 *
 * **Test 2**
 *
 * We allocate stack as we do in the first test but we mmap a page in the
 * space the stack is supposed to grow into and we expect the thread to
 * segfault when the guard page is faulted.
 */

#include <pthread.h>
#include "tst_test.h"
#include "tst_safe_pthread.h"

static long page_size;

static bool __attribute__((noinline)) check_stackgrow_up(void)
{
	char local_var;
	static char *addr;

	if (!addr) {
		addr = &local_var;
		return check_stackgrow_up();
	}

	return (addr < &local_var);
}

static void setup(void)
{
	if (check_stackgrow_up())
		tst_brk(TCONF, "Test can't be performed with stack grows up architecture");

	page_size = getpagesize();
}

/*
 * Returns stack lowest address. Note that the address is not mapped and will
 * be mapped on page fault when we grow the stack to the lowest address possible.
 */
static void *allocate_stack(size_t stack_size, size_t mapped_size)
{
	void *start, *stack_top, *stack_bottom;

	long reserved_size = 256 * page_size + stack_size;

	start = SAFE_MMAP(NULL, reserved_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_MUNMAP(start, reserved_size);

	SAFE_MMAP((start + reserved_size - mapped_size), mapped_size, PROT_READ | PROT_WRITE,
		  MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN,
		  -1, 0);

	stack_top = start + reserved_size;
	stack_bottom = start + reserved_size - stack_size;

	tst_res(TINFO, "start = %p, stack_top = %p, stack bottom = %p",
		start, stack_top, stack_bottom);
	tst_res(TINFO, "mapped pages %zu, stack pages %zu",
		mapped_size/page_size, stack_size/page_size);

	return stack_bottom;
}

static __attribute__((noinline)) void *check_depth_recursive(void *limit)
{
	if ((off_t) &limit < (off_t) limit) {
		tst_res(TINFO, "&limit = %p, limit = %p", &limit, limit);
		return NULL;
	}

	return check_depth_recursive(limit);
}

/*
 * We set the limit one page above the stack bottom to make sure that the stack
 * frame will not overflow to the next page, which would potentially cause
 * segfault if we are unlucky and there is a mapping right after the guard gap.
 *
 * Generally the stack frame would be much smaller than page_size so moving the
 * pointer by a few bytes would probably be enough, but we do not want to take
 * any chances.
 */
static void grow_stack(void *stack, size_t size)
{
	pthread_t test_thread;
	pthread_attr_t attr;
	int ret;
	void *limit = stack + page_size;

	ret = pthread_attr_init(&attr);
	if (ret)
		tst_brk(TBROK, "pthread_attr_init failed during setup");

	ret = pthread_attr_setstack(&attr, stack, size);
	if (ret)
		tst_brk(TBROK, "pthread_attr_setstack failed during setup");

	SAFE_PTHREAD_CREATE(&test_thread, &attr, check_depth_recursive, limit);
	SAFE_PTHREAD_JOIN(test_thread, NULL);

	exit(0);
}

static void grow_stack_success(size_t stack_size, size_t mapped_size)
{
	pid_t child_pid;
	int wstatus;
	void *stack;

	child_pid = SAFE_FORK();
	if (!child_pid) {
		stack = allocate_stack(stack_size, mapped_size);
		grow_stack(stack, stack_size);
	}

	SAFE_WAIT(&wstatus);
	if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 0)
		tst_res(TPASS, "Stack grows in unmapped region");
	else
		tst_res(TFAIL, "Child: %s", tst_strstatus(wstatus));

}

/*
 * We map a page at the bottom of the stack which will cause the thread to be
 * killed with SIGSEGV on faulting the guard page.
 */
static void grow_stack_fail(size_t stack_size, size_t mapped_size)
{
	pid_t child_pid;
	int wstatus;
	void *stack;

	child_pid = SAFE_FORK();
	if (!child_pid) {
		tst_no_corefile(0);
		stack = allocate_stack(stack_size, mapped_size);

		SAFE_MMAP(stack, page_size, PROT_READ | PROT_WRITE,
			  MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		tst_res(TINFO, "mapped page at %p", stack);

		grow_stack(stack, stack_size);
	}

	SAFE_WAIT(&wstatus);
	if (WIFSIGNALED(wstatus) && WTERMSIG(wstatus) == SIGSEGV)
		tst_res(TPASS, "Child killed by %s as expected", tst_strsig(SIGSEGV));
	else
		tst_res(TFAIL, "Child: %s", tst_strstatus(wstatus));
}

static void run_test(void)
{
	size_t pthread_stack = LTP_ALIGN(PTHREAD_STACK_MIN, getpagesize());
	size_t stack_size = 8 * pthread_stack;

	grow_stack_success(stack_size, pthread_stack);
	grow_stack_success(stack_size, stack_size/2);
	grow_stack_fail(stack_size, pthread_stack);
	grow_stack_fail(stack_size, stack_size/2);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run_test,
	.forks_child = 1,
};
