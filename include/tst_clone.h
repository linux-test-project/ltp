/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef TST_CLONE_H__
#define TST_CLONE_H__

/* Functions from lib/cloner.c */
int ltp_clone(unsigned long flags, int (*fn)(void *arg), void *arg,
		size_t stack_size, void *stack);
int ltp_clone7(unsigned long flags, int (*fn)(void *arg), void *arg,
		size_t stack_size, void *stack, ...);
int ltp_clone_alloc(unsigned long clone_flags, int (*fn)(void *arg),
		void *arg, size_t stacksize);
int ltp_clone_quick(unsigned long clone_flags, int (*fn)(void *arg),
		void *arg);
void *ltp_alloc_stack(size_t size);

#define clone(...) (use_the_ltp_clone_functions__do_not_use_clone)

#endif	/* TST_CLONE_H__ */
