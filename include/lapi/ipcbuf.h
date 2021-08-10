// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_IPCBUF_H__
#define LAPI_IPCBUF_H__

#include "config.h"
#include "lapi/posix_types.h"

#ifndef HAVE_IPC64_PERM

#if defined(__hppa__)
#define HAVE_IPC64_PERM
/*
 * The ipc64_perm structure for PA-RISC is almost identical to
 * kern_ipc_perm as we have always had 32-bit UIDs and GIDs in the kernel.
 * 'seq' has been changed from long to int so that it's the same size
 * on 64-bit kernels as on 32-bit ones.
 */

struct ipc64_perm
{
	__kernel_key_t		key;
	__kernel_uid_t		uid;
	__kernel_gid_t		gid;
	__kernel_uid_t		cuid;
	__kernel_gid_t		cgid;
#if __BITS_PER_LONG != 64
	unsigned short int	__pad1;
#endif
	__kernel_mode_t		mode;
	unsigned short int	__pad2;
	unsigned short int	seq;
	unsigned int		__pad3;
	unsigned long long int __unused1;
	unsigned long long int __unused2;
};
#endif /* __hppa__ */

#if defined(__powerpc__) || defined(__powerpc64__)
#define HAVE_IPC64_PERM
/*
 * The ipc64_perm structure for the powerpc is identical to
 * kern_ipc_perm as we have always had 32-bit UIDs and GIDs in the
 * kernel.  Note extra padding because this structure is passed back
 * and forth between kernel and user space.  Pad space is left for:
 *	- 1 32-bit value to fill up for 8-byte alignment
 *	- 2 miscellaneous 64-bit values
 */

struct ipc64_perm
{
	__kernel_key_t	key;
	__kernel_uid_t	uid;
	__kernel_gid_t	gid;
	__kernel_uid_t	cuid;
	__kernel_gid_t	cgid;
	__kernel_mode_t	mode;
	unsigned int	seq;
	unsigned int	__pad1;
	unsigned long long __unused1;
	unsigned long long __unused2;
};

#endif /* defined(__powerpc__) || defined(__powerpc64__) */

#if defined(__s390__)
#define HAVE_IPC64_PERM
/*
 * The user_ipc_perm structure for S/390 architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 32-bit mode_t and seq
 * - 2 miscellaneous 32-bit values
 */

struct ipc64_perm
{
	__kernel_key_t		key;
	__kernel_uid32_t	uid;
	__kernel_gid32_t	gid;
	__kernel_uid32_t	cuid;
	__kernel_gid32_t	cgid;
	__kernel_mode_t		mode;
	unsigned short		__pad1;
	unsigned short		seq;
#ifndef __s390x__
	unsigned short		__pad2;
#endif /* ! __s390x__ */
	unsigned long		__unused1;
	unsigned long		__unused2;
};

#endif /* defined(__powerpc__) || defined(__powerpc64__) */

#if defined(__sparc__)
#define HAVE_IPC64_PERM
/*
 * The ipc64_perm structure for sparc/sparc64 architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 32-bit seq
 * - on sparc for 32 bit mode (it is 32 bit on sparc64)
 * - 2 miscellaneous 64-bit values
 */

struct ipc64_perm
{
	__kernel_key_t		key;
	__kernel_uid32_t	uid;
	__kernel_gid32_t	gid;
	__kernel_uid32_t	cuid;
	__kernel_gid32_t	cgid;
#ifndef __arch64__
	unsigned short		__pad0;
#endif
	__kernel_mode_t		mode;
	unsigned short		__pad1;
	unsigned short		seq;
	unsigned long long	__unused1;
	unsigned long long	__unused2;
};

#endif /* __sparc__ */

#if defined(__xtensa__)
#define HAVE_IPC64_PERM
/*
 * Pad space is left for:
 * - 32-bit mode_t and seq
 * - 2 miscellaneous 32-bit values
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file "COPYING" in the main directory of
 * this archive for more details.
 */

struct ipc64_perm
{
	__kernel_key_t		key;
	__kernel_uid32_t	uid;
	__kernel_gid32_t	gid;
	__kernel_uid32_t	cuid;
	__kernel_gid32_t	cgid;
	__kernel_mode_t		mode;
	unsigned long		seq;
	unsigned long		__unused1;
	unsigned long		__unused2;
};

#endif /* __xtensa__ */

#ifndef HAVE_IPC64_PERM
/*
 * The generic ipc64_perm structure:
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * ipc64_perm was originally meant to be architecture specific, but
 * everyone just ended up making identical copies without specific
 * optimizations, so we may just as well all use the same one.
 *
 * Pad space is left for:
 * - 32-bit mode_t on architectures that only had 16 bit
 * - 32-bit seq
 * - 2 miscellaneous 32-bit values
 */

struct ipc64_perm {
	__kernel_key_t		key;
	__kernel_uid32_t	uid;
	__kernel_gid32_t	gid;
	__kernel_uid32_t	cuid;
	__kernel_gid32_t	cgid;
	__kernel_mode_t		mode;
				/* pad if mode_t is u16: */
	unsigned char		__pad1[4 - sizeof(__kernel_mode_t)];
	unsigned short		seq;
	unsigned short		__pad2;
	__kernel_ulong_t	__unused1;
	__kernel_ulong_t	__unused2;
};

#endif /* ipc64_perm */

#endif /* HAVE_IPC64_PERM */

#endif /* LAPI_IPCBUF_H__ */
