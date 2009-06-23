/*
 * Copyright (c) 2003 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/* Common platform specific defines for the clone system call tests */

//#define CHILD_STACK_SIZE 8192
#define CHILD_STACK_SIZE 16384

#if defined (__s390__) || (__s390x__)
#define clone __clone
extern int __clone(int(void*),void*,int,void*);
#elif defined(__ia64__)
#define clone2 __clone2
/* Prototype provided by David Mosberger				*/
/* int  __clone2(int (*fn) (void *arg), void *child_stack_base,		*/
/* 		size_t child_stack_size, int flags, void *arg,		*/
/*		pid_t *parent_tid, void *tls, pid_t *child_tid)		*/
extern int  __clone2(int (*fn) (void *arg), void *child_stack_base, 
		size_t child_stack_size, int flags, void *arg, 
		pid_t *parent_tid, void *tls, pid_t *child_tid); 
#endif
