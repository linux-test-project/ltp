/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef _PROCESS_VM_H_
#define _PROCESS_VM_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>
#include "lapi/semun.h"

static inline ssize_t test_process_vm_readv(pid_t pid,
		const struct iovec *lvec, unsigned long liovcnt,
		const struct iovec *rvec, unsigned long riovcnt,
		unsigned long flags)
{
#if defined(__NR_process_vm_readv)
	return syscall(__NR_process_vm_readv, pid, lvec, liovcnt,
			rvec, riovcnt, flags);
#else
	return syscall(-1);
#endif
}

static inline ssize_t test_process_vm_writev(pid_t pid,
		const struct iovec *lvec, unsigned long liovcnt,
		const struct iovec *rvec, unsigned long riovcnt,
		unsigned long flags)
{
#if defined(__NR_process_vm_writev)
	return syscall(__NR_process_vm_writev, pid, lvec, liovcnt,
			rvec, riovcnt, flags);
#else
	return syscall(-1);
#endif
}

#endif /* _PROCESS_VM_H_ */
