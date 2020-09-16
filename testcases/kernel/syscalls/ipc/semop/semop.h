// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SEMOP_VAR__
#define SEMOP_VAR__

#include <sys/sem.h>
#include "time64_variants.h"
#include "tst_timer.h"

static inline int sys_semtimedop(int semid, struct sembuf *sops, size_t nsops,
		void *timeout)
{
	return tst_syscall(__NR_semtimedop, semid, sops, nsops, timeout);
}

static inline int sys_semtimedop_time64(int semid, struct sembuf *sops,
					size_t nsops, void *timeout)
{
	return tst_syscall(__NR_semtimedop_time64, semid, sops, nsops, timeout);
}

static struct time64_variants variants[] = {
	{ .semop = semop, .ts_type = TST_LIBC_TIMESPEC, .desc = "semop: syscall"},

#if (__NR_semtimedop != __LTP__NR_INVALID_SYSCALL)
	{ .semtimedop = sys_semtimedop, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "semtimedop: syscall with old kernel spec"},
#endif

#if (__NR_semtimedop_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .semtimedop = sys_semtimedop_time64, .ts_type = TST_KERN_TIMESPEC, .desc = "semtimedop: syscall time64 with kernel spec"},
#endif
};

static inline int call_semop(struct time64_variants *tv, int semid,
		struct sembuf *sops, size_t nsops, void *timeout)
{
	if (tv->semop)
		return tv->semop(semid, sops, nsops);

	return tv->semtimedop(semid, sops, nsops, timeout);
}

static inline void semop_supported_by_kernel(struct time64_variants *tv)
{
       /* Check if the syscall is implemented on the platform */
       TEST(call_semop(tv, 0, NULL, 0, NULL));
       if (TST_RET == -1 && TST_ERR == ENOSYS)
               tst_brk(TCONF, "Test not supported on kernel/platform");
}

#endif /* SEMOP_VAR__ */
