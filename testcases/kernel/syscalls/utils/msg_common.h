/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef LTP_MSG_COMMON
#define LTP_MSG_COMMON

/* The #ifndef code below is for 2.5 64-bit kernels, where     */
/* the MSG_CMSG_COMPAT flag must be 0 in order for the syscall */
/* and this test to function correctly.                        */
#ifndef MSG_CMSG_COMPAT

#if defined(__powerpc64__) || defined(__mips64) || defined(__x86_64__) || \
	defined(__sparc64__) || defined(__ia64__) || defined(__s390x__)
#define MSG_CMSG_COMPAT 0x80000000
#else
#define MSG_CMSG_COMPAT 0
#endif

#endif


#endif /* LTP_MSG_COMMON */
