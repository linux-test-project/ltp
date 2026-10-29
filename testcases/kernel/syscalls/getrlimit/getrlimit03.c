// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 *
 * Architectures may provide up to three syscalls that have been used to
 * implement getrlimit(2) in different libc implementations.  These syscalls
 * differ in the size and signedness of rlim_t:
 *
 * - __NR_getrlimit uses long or unsigned long, depending on the
 *   architecture
 *
 * - __NR_ugetrlimit uses unsigned long, and only exists on
 *   architectures where __NR_getrlimit is signed
 *
 * - __NR_prlimit64 uses uint64_t
 *
 * This test compares the results returned by all three syscalls, confirming
 * that they either match or were appropriately capped at the respective
 * RLIM_INFINITY constant.
 */

#include <inttypes.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/abisize.h"

/**
 * Linux provides an "old" getrlimit syscall handler that uses signed long,
 * and a "new" getrlimit syscall handler that uses unsigned long.
 *
 * The underlying syscall names vary across architectures, depending on whether
 * the architecture predates the "new" handler.  For clarity, this test
 * will call them getrlimit_long and getlimit_ulong internally.
 *
 * __NR_getrlimit has been deprecated from arm EABI and moved to OABI_COMPAT,
 * so the syscall on arm may or may not be available even if __NR_ugetrlimit
 * exists.
 */
#if __NR_ugetrlimit != __LTP__NR_INVALID_SYSCALL
# if !defined(__arm__) || __NR_getrlimit != __LTP__NR_INVALID_SYSCALL
#  define SIGNED_GETRLIMIT
# endif
# define __NR_getrlimit_ulong		__NR_ugetrlimit
# define __NR_getrlimit_ulong_str	"__NR_ugetrlimit"
#else
# define __NR_getrlimit_ulong		__NR_getrlimit
# define __NR_getrlimit_ulong_str	"__NR_getrlimit"
#endif

#ifndef HAVE_STRUCT_RLIMIT64
struct rlimit64 {
	uint64_t rlim_cur;
	uint64_t rlim_max;
};
#endif
const uint64_t RLIM_INFINITY_U64 = UINT64_MAX;

static int getrlimit_u64(int resource, struct rlimit64 *rlim)
{
	return tst_syscall(__NR_prlimit64, 0, resource, NULL, rlim);
}

struct rlimit_ulong {
	unsigned long rlim_cur;
	unsigned long rlim_max;
};

#if defined(__mips__) && defined(TST_ABI32)
	const unsigned long RLIM_INFINITY_UL = 0x7fffffffUL;
#else
	const unsigned long RLIM_INFINITY_UL = ULONG_MAX;
#endif

static int getrlimit_ulong(int resource, struct rlimit_ulong *rlim)
{
	return syscall(__NR_getrlimit_ulong, resource, rlim);
}

const long RLIM_INFINITY_L = LONG_MAX;

#ifdef SIGNED_GETRLIMIT
struct rlimit_long {
	long rlim_cur;
	long rlim_max;
};

static int getrlimit_long(int resource, struct rlimit_long *rlim)
{
	return syscall(__NR_getrlimit, resource, rlim);
}
#endif

static int compare_retval(int resource, int ret_u64, int errno_u64,
			  int ret_other, int errno_other,
			  const char *other_syscall)
{
	if (ret_u64 != ret_other || errno_u64 != errno_other) {
		tst_res(TFAIL, "__NR_prlimit64(%d) returned %d (%s) but %s(%d) returned %d (%s)",
			resource, ret_u64, tst_strerrno(errno_u64),
			other_syscall, resource, ret_other,
			tst_strerrno(errno_other));
		return -1;
	}

	return 0;
}

static int compare_u64_ulong(int resource, uint64_t val_u64,
			     unsigned long val_ul, const char *kind)
{
	if ((val_u64 > RLIM_INFINITY_UL && val_ul != RLIM_INFINITY_UL) ||
	    (val_u64 <= RLIM_INFINITY_UL && val_ul != val_u64)) {
		tst_res(TFAIL, "__NR_prlimit64(%d) had %s = %" PRIx64 " but " __NR_getrlimit_ulong_str "(%d) had %s = %lx",
			resource, kind, val_u64,
			resource, kind, val_ul);
		return -1;
	}

	return 0;
}

#ifdef SIGNED_GETRLIMIT
static int compare_u64_long(int resource, uint64_t val_u64, long val_l,
			    const char *kind)
{
	if ((val_u64 > (uint64_t)RLIM_INFINITY_L && val_l != RLIM_INFINITY_L) ||
	    (val_u64 <= (uint64_t)RLIM_INFINITY_L && val_l != (long)val_u64)) {
		tst_res(TFAIL, "__NR_prlimit64(%d) had %s = %" PRIx64 " but __NR_getrlimit(%d) had %s = %lx",
			resource, kind, val_u64,
			resource, kind, val_l);
		return -1;
	}

	return 0;
}
#endif

static void run(unsigned int resource)
{
	struct rlimit64 rlim_u64;
	int ret_u64;
	int errno_u64;

	struct rlimit_ulong rlim_ul;
	int ret_ul;
	int errno_ul;

#ifdef SIGNED_GETRLIMIT
	struct rlimit_long rlim_l;
	int ret_l;
	int errno_l;
#endif

	errno = 0;
	ret_u64 = getrlimit_u64(resource, &rlim_u64);
	errno_u64 = errno;

	errno = 0;
	ret_ul = getrlimit_ulong(resource, &rlim_ul);
	errno_ul = errno;
	if (errno_ul == ENOSYS) {
		tst_res(TCONF | TERRNO,
			"%s not implemented", __NR_getrlimit_ulong_str);
		test.tcnt = 1;
		return;
	}

	if (compare_retval(resource, ret_u64, errno_u64, ret_ul, errno_ul,
			   __NR_getrlimit_ulong_str) ||
	    compare_u64_ulong(resource, rlim_u64.rlim_cur, rlim_ul.rlim_cur,
			      "rlim_cur") ||
	    compare_u64_ulong(resource, rlim_u64.rlim_max, rlim_ul.rlim_max,
			      "rlim_max"))
		return;

	tst_res(TPASS, "__NR_prlimit64(%d) and %s(%d) gave consistent results",
		resource, __NR_getrlimit_ulong_str, resource);

#ifdef SIGNED_GETRLIMIT
	errno = 0;
	ret_l = getrlimit_long(resource, &rlim_l);
	errno_l = errno;
	if (errno_l == ENOSYS) {
		tst_res(TCONF | TERRNO,
			"__NR_getrlimit(%d) not implemented", __NR_getrlimit);
		return;
	}

	if (compare_retval(resource, ret_u64, errno_u64, ret_l, errno_l,
			   "__NR_getrlimit") ||
	    compare_u64_long(resource, rlim_u64.rlim_cur, rlim_l.rlim_cur,
			     "rlim_cur") ||
	    compare_u64_long(resource, rlim_u64.rlim_max, rlim_l.rlim_max,
			     "rlim_max"))
		return;

	tst_res(TPASS, "__NR_prlimit64(%d) and __NR_getrlimit(%d) gave "
		"consistent results", resource, resource);
#endif
}

static struct tst_test test = {
	.tcnt = RLIM_NLIMITS,
	.test = run,
};
