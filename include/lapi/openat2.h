// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_OPENAT2_H__
#define LAPI_OPENAT2_H__

#include <sys/syscall.h>
#include <linux/types.h>

#include "lapi/syscalls.h"

#include "config.h"

#ifndef HAVE_OPENAT2
/*
 * Arguments for how openat2(2) should open the target path. If only @flags and
 * @mode are non-zero, then openat2(2) operates very similarly to openat(2).
 *
 * However, unlike openat(2), unknown or invalid bits in @flags result in
 * -EINVAL rather than being silently ignored. @mode must be zero unless one of
 * {O_CREAT, O_TMPFILE} are set.
 *
 * @flags: O_* flags.
 * @mode: O_CREAT/O_TMPFILE file mode.
 * @resolve: RESOLVE_* flags.
 */
struct open_how {
	uint64_t flags;
	uint64_t mode;
	uint64_t resolve;
};

/* how->resolve flags for openat2(2). */
#define RESOLVE_NO_XDEV		0x01 /* Block mount-point crossings
					(includes bind-mounts). */
#define RESOLVE_NO_MAGICLINKS	0x02 /* Block traversal through procfs-style
					"magic-links". */
#define RESOLVE_NO_SYMLINKS	0x04 /* Block traversal through all symlinks
					(implies OEXT_NO_MAGICLINKS) */
#define RESOLVE_BENEATH		0x08 /* Block "lexical" trickery like
					"..", symlinks, and absolute
					paths which escape the dirfd. */
#define RESOLVE_IN_ROOT		0x10 /* Make all jumps to "/" and ".."
					be scoped inside the dirfd
					(similar to chroot(2)). */

static inline int openat2(int dfd, const char *pathname,
                          struct open_how *how, size_t size)
{
	return tst_syscall(__NR_openat2, dfd, pathname, how, size);
}
#endif

struct open_how_pad {
	/* how should be kept as the first entry here */
	struct open_how how;
	uint64_t pad;
};

static inline void openat2_supported_by_kernel(void)
{
	long ret;

	if ((tst_kvercmp(5, 6, 0)) < 0) {
		/* Check if the syscall is backported on an older kernel */
		ret = syscall(__NR_openat2, -1, NULL, NULL, 0);
		if (ret == -1 && errno == ENOSYS)
			tst_brk(TCONF, "Test not supported on kernel version < v5.6");
	}
}

#endif /* LAPI_OPENAT2_H__ */
