/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * getdents.h - common definitions for the getdents() tests.
 */

#ifndef __GETDENTS_H
#define __GETDENTS_H	1
#include <sys/syscall.h>

#ifdef __i386__
	#define GETDENTS_ASM() ({ int __rval;				\
				__asm__ __volatile__("			\
					movl	%4, %%edx \n		\
					movl	%3, %%ecx \n		\
					movl	%2, %%ebx \n		\
					movl	%1, %%eax \n		\
					int	$0x80 \n		\
					movl	%%eax, %0"		\
				: "=a" (__rval)				\
				: "a" (cnum), "b" (fd), "c" (dirp), "d" (count)\
				: "memory"				\
				);					\
				__rval;					\
		    	})
#else
	#define GETDENTS_ASM() 0
#endif /* __i386__ */

#endif /* getdents.h */
