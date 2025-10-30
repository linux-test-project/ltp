// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2007 Davide Libenzi <davidel@xmailserver.org>
 * Copyright (C) 2015,2022 Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2025
 *
 * Mostly copied/adapted from <linux/userfaultfd.h>
 */

#ifndef LAPI_USERFAULTFD_H__
#define LAPI_USERFAULTFD_H__

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include "lapi/syscalls.h"

#ifdef HAVE_LINUX_USERFAULTFD_H
#include <linux/userfaultfd.h>
#endif

/* userfaultfd support was added in v4.1 */
#ifndef UFFD_API
#define UFFD_API ((__u64)0xAA)

/*
 * Valid ioctl command number range with this API is from 0x00 to
 * 0x3F.  UFFDIO_API is the fixed number, everything else can be
 * changed by implementing a different UFFD_API. If sticking to the
 * same UFFD_API more ioctl can be added and userland will be aware of
 * which ioctl the running kernel implements through the ioctl command
 * bitmask written by the UFFDIO_API.
 */
#define _UFFDIO_REGISTER		(0x00)
#define _UFFDIO_UNREGISTER		(0x01)
#define _UFFDIO_WAKE			(0x02)
#define _UFFDIO_COPY			(0x03)
#define _UFFDIO_ZEROPAGE		(0x04)
#define _UFFDIO_API			(0x3F)

/* userfaultfd ioctl ids */
#define UFFDIO 0xAA
#define UFFDIO_API		_IOWR(UFFDIO, _UFFDIO_API,	\
				      struct uffdio_api)
#define UFFDIO_REGISTER		_IOWR(UFFDIO, _UFFDIO_REGISTER, \
				      struct uffdio_register)
#define UFFDIO_UNREGISTER	_IOR(UFFDIO, _UFFDIO_UNREGISTER,	\
				     struct uffdio_range)
#define UFFDIO_WAKE		_IOR(UFFDIO, _UFFDIO_WAKE,	\
				     struct uffdio_range)
#define UFFDIO_COPY		_IOWR(UFFDIO, _UFFDIO_COPY,	\
				      struct uffdio_copy)
#define UFFDIO_ZEROPAGE		_IOWR(UFFDIO, _UFFDIO_ZEROPAGE,	\
				      struct uffdio_zeropage)

/* read() structure */
struct uffd_msg {
	__u8	event;

	__u8	reserved1;
	__u16	reserved2;
	__u32	reserved3;

	union {
		struct {
			__u64	flags;
			__u64	address;
		} pagefault;

		struct {
			/* unused reserved fields */
			__u64	reserved1;
			__u64	reserved2;
			__u64	reserved3;
		} reserved;
	} arg;
} __packed;

/*
 * Start at 0x12 and not at 0 to be more strict against bugs.
 */
#define UFFD_EVENT_PAGEFAULT	0x12

/* flags for UFFD_EVENT_PAGEFAULT */
#define UFFD_PAGEFAULT_FLAG_WRITE	(1<<0)	/* If this was a write fault */
#define UFFD_PAGEFAULT_FLAG_WP		(1<<1)	/* If reason is VM_UFFD_WP */

struct uffdio_api {
	/* userland asks for an API number and the features to enable */
	__u64 api;
	/*
	 * Kernel answers below with the all available features for
	 * the API, this notifies userland of which events and/or
	 * which flags for each event are enabled in the current
	 * kernel.
	 *
	 * Note: UFFD_EVENT_PAGEFAULT and UFFD_PAGEFAULT_FLAG_WRITE
	 * are to be considered implicitly always enabled in all kernels as
	 * long as the uffdio_api.api requested matches UFFD_API.
	 */
	__u64 features;

	__u64 ioctls;
};

struct uffdio_range {
	__u64 start;
	__u64 len;
};

struct uffdio_register {
	struct uffdio_range range;
#define UFFDIO_REGISTER_MODE_MISSING	((__u64)1<<0)
#define UFFDIO_REGISTER_MODE_WP		((__u64)1<<1)
	__u64 mode;

	/*
	 * kernel answers which ioctl commands are available for the
	 * range, keep at the end as the last 8 bytes aren't read.
	 */
	__u64 ioctls;
};

struct uffdio_copy {
	__u64 dst;
	__u64 src;
	__u64 len;
	/*
	 * There will be a wrprotection flag later that allows to map
	 * pages wrprotected on the fly. And such a flag will be
	 * available if the wrprotection ioctl are implemented for the
	 * range according to the uffdio_register.ioctls.
	 */
#define UFFDIO_COPY_MODE_DONTWAKE		((__u64)1<<0)
	__u64 mode;

	/*
	 * "copy" is written by the ioctl and must be at the end: the
	 * copy_from_user will not read the last 8 bytes.
	 */
	__s64 copy;
};

struct uffdio_zeropage {
	struct uffdio_range range;
#define UFFDIO_ZEROPAGE_MODE_DONTWAKE		((__u64)1<<0)
	__u64 mode;

	/*
	 * "zeropage" is written by the ioctl and must be at the end:
	 * the copy_from_user will not read the last 8 bytes.
	 */
	__s64 zeropage;
};
#endif /* UFFD_API */


/* UFFD_USER_MODE_ONLY was added in v5.11 */
#ifndef UFFD_USER_MODE_ONLY
#define UFFD_USER_MODE_ONLY 1
#endif /* UFFD_USER_MODE_ONLY */


/* UFFD_PAGEFAULT_FLAG_MINOR and UFFDIO_CONTINUE were added in v5.13 */
#ifndef UFFD_PAGEFAULT_FLAG_MINOR
#define UFFD_FEATURE_MINOR_HUGETLBFS		(1<<9)
#define UFFDIO_REGISTER_MODE_MINOR	((__u64)1<<2)

#define _UFFDIO_CONTINUE		(0x07)
#define UFFDIO_CONTINUE		_IOWR(UFFDIO, _UFFDIO_CONTINUE,	\
				      struct uffdio_continue)

struct uffdio_continue {
	struct uffdio_range range;
#define UFFDIO_CONTINUE_MODE_DONTWAKE		((__u64)1<<0)
	__u64 mode;

	/*
	 * Fields below here are written by the ioctl and must be at the end:
	 * the copy_from_user will not read past here.
	 */
	__s64 mapped;
};
#endif /* UFFD_PAGEFAULT_FLAG_MINOR */


/* UFFD_FEATURE_MINOR_SHMEM was added in v5.14 */
#ifndef UFFD_FEATURE_MINOR_SHMEM
#define UFFD_FEATURE_MINOR_SHMEM		(1<<10)
#endif /* UFFD_FEATURE_MINOR_SHMEM */

#define SAFE_USERFAULTFD(flags, retry) \
	safe_userfaultfd(__FILE__, __LINE__, (flags), (retry))

static inline int safe_userfaultfd(const char *file, const int lineno, int
				   flags, bool retry)
{
	int ret;

retry:
	ret = tst_syscall(__NR_userfaultfd, flags);
	if (ret == -1) {
		if (errno == EPERM) {
			if (retry && !(flags & UFFD_USER_MODE_ONLY)) {
				flags |= UFFD_USER_MODE_ONLY;
				goto retry;
			}
			tst_res_(file, lineno, TINFO,
				 "Hint: check /proc/sys/vm/unprivileged_userfaultfd");
			tst_brk_(file, lineno, TCONF | TERRNO,
				"userfaultfd() requires CAP_SYS_PTRACE on this system");
		}
		tst_brk_(file, lineno, TBROK | TERRNO,
				 "syscall(__NR_userfaultfd, %d) failed", flags);
	} else if (ret < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "Invalid syscall(__NR_userfaultfd, %d) return value %d", flags, ret);
	}

	return ret;
}

#endif /* LAPI_USERFAULTFD_H__ */
