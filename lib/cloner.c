/*
 * Copyright (c) International Business Machines Corp., 2009
 * Some wrappers for clone functionality.  Thrown together by Serge Hallyn
 * <serue@us.ibm.com> based on existing clone usage in ltp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h> /* fork, getpid, sleep */
#include <string.h>
#include <stdlib.h> /* exit */
#include <sched.h> /* clone */
#include "test.h"

#undef clone /* we want to use clone() */

/* copied from several other files under ltp */
#if defined (__s390__) || (__s390x__)
#define clone __clone
extern int __clone(int(void*),void*,int,void*);
#elif defined(__ia64__)
#define clone2 __clone2
/* Prototype provided by David Mosberger				*/
extern int  __clone2(int (*fn) (void *arg), void *child_stack_base,
		size_t child_stack_size, int flags, void *arg,
		pid_t *parent_tid, void *tls, pid_t *child_tid);
#endif

/***********************************************************************
 * ltp_clone: wrapper for clone to hide the architecture dependencies.
 *   1. hppa takes bottom of stack and no stacksize (stack grows up)
 *   2. __ia64__ takes bottom of stack and uses clone2
 *   3. all others take top of stack (stack grows down)
 ***********************************************************************/
int
ltp_clone(unsigned long clone_flags, int (*fn)(void *arg), void *arg,
		size_t stack_size, void *stack)
{
	int ret;

#if defined(__hppa__)
	ret = clone(fn, stack, clone_flags, arg);
#elif defined(__ia64__)
	ret = clone2(fn, stack, stack_size, clone_flags, arg, NULL, NULL, NULL);
#elif defined(__arm__)
	/*
	 * Stack size should be a multiple of 32 bit words
	 * & stack limit must be aligned to a 32 bit boundary
	 */
	ret = clone(fn, (stack ? stack + stack_size : NULL),
			clone_flags, arg);
#else
	ret = clone(fn, (stack ? stack + stack_size - 1 : NULL),
			clone_flags, arg);
#endif

	return ret;
}

/***********************************************************************
 * ltp_clone_malloc: also does the memory allocation for clone with a
 * caller-specified size.
 ***********************************************************************/
int
ltp_clone_malloc(unsigned long clone_flags, int (*fn)(void *arg), void *arg,
		size_t stack_size)
{
	int ret;
	void *stack = malloc(stack_size);
	int saved_errno;

	if (!stack)
		return -1;

	ret = ltp_clone(clone_flags, fn, arg, stack_size, stack);

	if (ret == -1) {
		saved_errno = errno;
		free(stack);
		errno = saved_errno;
	}

	return ret;
}

/***********************************************************************
 * ltp_clone_quick: calls ltp_clone_malloc with predetermined stack size.
 * Experience thus far suggests that one page is often insufficient,
 * while 4*getpagesize() seems adequate.
 ***********************************************************************/
int
ltp_clone_quick(unsigned long clone_flags, int (*fn)(void *arg), void *arg)
{
	size_t stack_size = getpagesize() * 4;

	return ltp_clone_malloc(clone_flags, fn, arg, stack_size);
}
