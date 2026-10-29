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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <stdarg.h>
#include "config.h"
#include "tst_clone.h"

#undef clone			/* we want to use clone() */

/*
 * The ia64 port has never included a prototype for __clone2(). It was updated
 * to take eight parameters in glibc commit:
 *
 * commit 625f22fc7f8e0d61e3e6cff2c65468b91dbad426
 * Author: Ulrich Drepper <drepper@redhat.com>
 * Date:   Mon Mar 3 19:53:27 2003 +0000
 *
 * The first release that contained this commit was glibc-2.3.3 which is old
 * enough to assume that __clone2() takes eight parameters.
 */
#if defined(__ia64__)
extern int __clone2(int (*fn) (void *arg), void *child_stack_base,
                    size_t child_stack_size, int flags, void *arg,
                    pid_t *parent_tid, void *tls, pid_t *child_tid);
#endif

/*
 * ltp_clone: wrapper for clone to hide the architecture dependencies.
 *   1. hppa takes bottom of stack and no stacksize (stack grows up)
 *   2. __ia64__ takes bottom of stack and uses clone2
 *   3. all others take top of stack (stack grows down)
 */
static int
ltp_clone_(unsigned long flags, int (*fn)(void *arg), void *arg,
	   size_t stack_size, void *stack, pid_t *ptid, void *tls, pid_t *ctid)
{
	int ret;

#if defined(__ia64__)
	ret = __clone2(fn, stack, stack_size, flags, arg, ptid, tls, ctid);
#else
# if defined(__hppa__) || defined(__metag__)
	/*
	 * These arches grow their stack up, so don't need to adjust the base.
	 * XXX: This should be made into a runtime test.
	 */
# else
	/*
	 * For archs where stack grows downwards, stack points to the topmost
	 * address of the memory space set up for the child stack.
	 */
	if (stack)
		stack += stack_size;
# endif

	ret = clone(fn, stack, flags, arg, ptid, tls, ctid);
#endif

	return ret;
}

int ltp_clone(unsigned long flags, int (*fn)(void *arg), void *arg,
              size_t stack_size, void *stack)
{
	return ltp_clone_(flags, fn, arg, stack_size, stack, NULL, NULL, NULL);
}

int ltp_clone7(unsigned long flags, int (*fn)(void *arg), void *arg,
               size_t stack_size, void *stack, ...)
{
	pid_t *ptid, *ctid;
	void *tls;
	va_list arg_clone;

	va_start(arg_clone, stack);
	ptid = va_arg(arg_clone, pid_t *);
	tls = va_arg(arg_clone, void *);
	ctid = va_arg(arg_clone, pid_t *);
	va_end(arg_clone);

	return ltp_clone_(flags, fn, arg, stack_size, stack, ptid, tls, ctid);
}

/*
 * ltp_alloc_stack: allocate stack of size 'size', that is sufficiently
 * aligned for all arches. User is responsible for freeing allocated
 * memory.
 * Returns pointer to new stack. On error, returns NULL with errno set.
 */
void *ltp_alloc_stack(size_t size)
{
	void *ret = NULL;
	int err;

	err = posix_memalign(&ret, 64, size);
	if (err)
		errno = err;

	return ret;
}
