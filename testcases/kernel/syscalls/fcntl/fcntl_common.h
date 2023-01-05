#ifndef FCNTL_COMMON_H__
#define FCNTL_COMMON_H__

#include <inttypes.h>

#include "tst_test.h"
#include "tst_kernel.h"

#include "lapi/syscalls.h"
#include "lapi/abisize.h"
#include "lapi/fcntl.h"

struct my_flock64 {
	int16_t l_type;
	int16_t l_whence;
	int64_t l_start;
	int64_t l_len;
	int32_t l_pid;
#if defined(__sparc__)
	int16_t padding;
#endif
};

/*
 * F_OFD_* commands always require flock64 struct. Older GLibc (pre 2.29) would
 * pass the flock sturct directly to the kernel even if it had 32-bit
 * offsets.
 *
 * Also, if and only if, we are on 32-bit kernel we need to use the
 * fcntl64 compat syscall.
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
	const int sysno = tst_kernel_bits() > 32 ? __NR_fcntl : __NR_fcntl64;
	const int ret = tst_syscall(sysno, fd, cmd, &l64);

	lck->l_type = l64.l_type;
	lck->l_whence = l64.l_whence;
	lck->l_start = l64.l_start;
	lck->l_len = l64.l_len;
	lck->l_pid = l64.l_pid;

	if (ret != -1)
		return ret;

	tst_brk_(file, line, TBROK | TERRNO,
		 "%s(%d, %s, { %d, %d, %"PRId64", %"PRId64", %d })",
		 tst_kernel_bits() > 32 ? "fcntl" : "fcntl64",
		 fd,
		 cmd_name,
		 l64.l_type, l64.l_whence, l64.l_start, l64.l_len, l64.l_pid);

	return ret;
}

#define FCNTL_COMPAT(fd, cmd, flock) \
	fcntl_compat(__FILE__, __LINE__, #cmd, fd, cmd, flock)

#endif /* FCNTL_COMMON_H__ */
