// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015  Yi Zhang <wetpzy@gmail.com>
 *                     Li Wang <liwang@redhat.com>
 *
 * DESCRIPTION:
 *
 *   It is a regression test for commit:
 *   http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/
 *   commit/?id=13d60f4
 *
 *   The implementation of futex doesn't produce unique keys for futexes
 *   in shared huge pages, so threads waiting on different futexes may
 *   end up on the same wait list. This results in incorrect threads being
 *   woken by FUTEX_WAKE.
 *
 *   Needs to be run as root unless there are already enough huge pages available.
 *   In the fail case, which happens in the CentOS-6.6 kernel (2.6.32-504.8.1),
 *   the tests hangs until it times out after a 30-second wait.
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>

#include "futextest.h"
#include "futex_utils.h"
#include "lapi/mmap.h"
#include "tst_safe_stdio.h"
#include "tst_safe_pthread.h"

static futex_t *futex1, *futex2;

static struct tst_ts to;

static struct futex_test_variants variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .tstype = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .tstype = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	to = tst_ts_from_ns(tv->tstype, 30 * NSEC_PER_SEC);
}

static void *wait_thread1(void *arg LTP_ATTRIBUTE_UNUSED)
{
	struct futex_test_variants *tv = &variants[tst_variant];

	futex_wait(tv->fntype, futex1, *futex1, &to, 0);

	return NULL;
}

static void *wait_thread2(void *arg LTP_ATTRIBUTE_UNUSED)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	int res;

	errno = 0;
	res = futex_wait(tv->fntype, futex2, *futex2, &to, 0);
	if (!res)
		tst_res(TPASS, "Hi hydra, thread2 awake!");
	else
		tst_res(TFAIL | TERRNO, "Bug: wait_thread2 did not wake after 30 secs.");

	return NULL;
}

static void wakeup_thread2(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	void *addr;
	int hpsz, pgsz;
	pthread_t th1, th2;

	hpsz = tst_get_hugepage_size();
	tst_res(TINFO, "Hugepagesize %i", hpsz);

	/*allocate some shared memory*/
	addr = mmap(NULL, hpsz, PROT_WRITE | PROT_READ,
	            MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

	if (addr == MAP_FAILED) {
		if (errno == ENOMEM)
			tst_brk(TCONF, "Cannot allocate hugepage, memory too fragmented?");

		tst_brk(TBROK | TERRNO, "Cannot allocate hugepage");
	}

	pgsz = getpagesize();

	/*apply the first subpage to futex1*/
	futex1 = addr;
	*futex1 = 0;
	/*apply the second subpage to futex2*/
	futex2 = (futex_t *)((char *)addr + pgsz);
	*futex2 = 0;

	/*thread1 block on futex1 first,then thread2 block on futex2*/
	SAFE_PTHREAD_CREATE(&th1, NULL, wait_thread1, NULL);
	SAFE_PTHREAD_CREATE(&th2, NULL, wait_thread2, NULL);

	while (wait_for_threads(2))
		usleep(1000);

	futex_wake(tv->fntype, futex2, 1, 0);
	SAFE_PTHREAD_JOIN(th2, NULL);
	futex_wake(tv->fntype, futex1, 1, 0);
	SAFE_PTHREAD_JOIN(th1, NULL);

	SAFE_MUNMAP(addr, hpsz);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = wakeup_thread2,
	.test_variants = ARRAY_SIZE(variants),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.hugepages = {1, TST_NEEDS},
};
