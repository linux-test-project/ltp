#ifndef FCNTL_COMMON_H__
#define FCNTL_COMMON_H__

#include <inttypes.h>

#include "tst_test.h"
#include "tst_kernel.h"

#include "lapi/syscalls.h"
#include "lapi/abisize.h"
#include "lapi/fcntl.h"

#if defined(TST_ABI64)
#define FCNTL_COMPAT(fd, cmd, flock) \
	SAFE_FCNTL(fd, cmd, flock)

#else
struct my_flock64 {
	short l_type;
	short l_whence;
	off64_t l_start;
	off64_t l_len;
	pid_t l_pid;
#if defined(__sparc__)
	short padding;
#endif
};

/*
 * F_OFD_* commands always require flock64 struct. Older GLibc (pre 2.29) would
 * pass the flock sturct directly to the kernel even if it had 32-bit
 * offsets.
 *
 * If we are on 32-bit abi we need to use the fcntl64 compat syscall.
 *
 * See:
 * glibc: 06ab719d30 Fix Linux fcntl OFD locks for non-LFS architectures (BZ#20251)
 * kernel: fs/fcntl.c
 */
static inline int fcntl_compat(const char *file, const int line, const char *cmd_name,
			       int fd, int cmd, struct flock *lck)
{
	struct my_flock64 l64 = {
		.l_type = lck->l_type,
		.l_whence = lck->l_whence,
		.l_start = lck->l_start,
		.l_len = lck->l_len,
		.l_pid = lck->l_pid,
	};

	const int ret = tst_syscall(__NR_fcntl64, fd, cmd, &l64);

	lck->l_type = l64.l_type;
	lck->l_whence = l64.l_whence;
	lck->l_start = l64.l_start;
	lck->l_len = l64.l_len;
	lck->l_pid = l64.l_pid;

	if (ret != -1)
		return ret;

	tst_brk_(file, line, TBROK | TERRNO,
		 "fcntl64(%d, %s, { %d, %d, %"PRId64", %"PRId64", %d })",
		 fd,
		 cmd_name,
		 l64.l_type, l64.l_whence, l64.l_start, l64.l_len, l64.l_pid);

	return ret;
}

#define FCNTL_COMPAT(fd, cmd, flock) \
	fcntl_compat(__FILE__, __LINE__, #cmd, fd, cmd, flock)
#endif

#endif /* FCNTL_COMMON_H__ */
