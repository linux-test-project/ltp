/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef TST_CLONE_H__
#define TST_CLONE_H__

#include <sched.h>

#ifdef TST_TEST_H__

/* The parts of clone3's clone_args we support */
struct tst_clone_args {
	uint64_t flags;
	uint64_t exit_signal;
	uint64_t cgroup;
};

/* clone3 with fallbacks to clone when possible. Be aware that it
 * returns -1 if clone3 fails (except ENOSYS), but -2 if clone fails.
 *
 * Without CLONE_VM this acts like fork so you may want to set
 * tst_test.forks_child (safe_clone requires this).
 *
 * You should set exit_signal to SIGCHLD for
 * tst_reap_children. Otherwise you must call wait with the
 * appropriate parameters.
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
