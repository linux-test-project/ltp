/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef TST_CLONE_H__
#define TST_CLONE_H__

#include <sched.h>

#ifdef TST_TEST_H__

/**
 * struct tst_clone_args - Arguments for tst_clone().
 * @flags: Clone flags (e.g. CLONE_NEWNS, CLONE_NEWPID).
 * @pidfd: Pointer (cast to u64) where the kernel stores the pidfd when
 *         CLONE_PIDFD is set.
 * @exit_signal: Signal sent to the parent when the child exits.
 * @cgroup: Target cgroup fd (requires CLONE_INTO_CGROUP).
 */
struct tst_clone_args {
	uint64_t flags;
	uint64_t pidfd;
	uint64_t exit_signal;
	uint64_t cgroup;
};

/**
 * tst_clone() - Create a child process via clone3 with clone fallback.
 * @args: Clone arguments.
 *
 * Without CLONE_VM this acts like fork(); set tst_test.forks_child
 * accordingly (safe_clone requires it). Set exit_signal to SIGCHLD
 * for tst_reap_children.
 *
 * Return: Child PID in the parent, 0 in the child, -1 on clone3 failure
 *         (except ENOSYS), -2 on clone failure.
 */
pid_t tst_clone(const struct tst_clone_args *args);

pid_t safe_clone(const char *file, const int lineno,
		 const struct tst_clone_args *args);

/* "Safe" version of tst_clone */
#define SAFE_CLONE(args) safe_clone(__FILE__, __LINE__, args)

#endif	/* TST_TEST_H__ */

/* Functions from lib/cloner.c */
int ltp_clone(unsigned long flags, int (*fn)(void *arg), void *arg,
		size_t stack_size, void *stack);
int ltp_clone7(unsigned long flags, int (*fn)(void *arg), void *arg,
		size_t stack_size, void *stack, ...);
void *ltp_alloc_stack(size_t size);

#define clone(...) (use_the_ltp_clone_functions__do_not_use_clone)

#endif	/* TST_CLONE_H__ */
