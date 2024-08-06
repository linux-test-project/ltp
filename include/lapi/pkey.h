// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019-2024 Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2019-2024
 */

#ifndef PKEYS_H__
#define PKEYS_H__

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/mmap.h"

#ifndef PKEY_DISABLE_ACCESS
# define PKEY_DISABLE_ACCESS 0x1
# define PKEY_DISABLE_WRITE  0x2
#endif

#ifndef HAVE_PKEY_MPROTECT
inline int pkey_mprotect(void *addr, size_t len, int prot, int pkey)
{
	return tst_syscall(__NR_pkey_mprotect, addr, len, prot, pkey);
}

inline int pkey_alloc(unsigned int flags, unsigned int access_rights)
{
	return tst_syscall(__NR_pkey_alloc, flags, access_rights);
}

inline int pkey_free(int pkey)
{
	return tst_syscall(__NR_pkey_free, pkey);
}
#endif /* HAVE_PKEY_MPROTECT */

static inline void check_pkey_support(void)
{
	int pkey = tst_syscall(__NR_pkey_alloc, 0, 0);

	if (pkey == -1) {
		if (errno == ENOSYS)
			tst_brk(TCONF, "pkey_alloc is not implemented");
		if (errno == EINVAL)
			tst_brk(TCONF, "pku is not supported on this CPU");
		if (errno == ENOSPC)
			tst_brk(TCONF, "pkeys are not available for test");
	}

	tst_syscall(__NR_pkey_free, pkey);
}

#endif /* PKEYS_H__ */
