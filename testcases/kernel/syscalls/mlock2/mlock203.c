// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */
/*
 * Description:
 * If one memory is already locked by mlock2() with MLOCK_ONFAULT and then
 * it is locked again by mlock()(or mlock2() without MLOCK_ONFAULT), the
 * VmLck field in /proc/pid/status should increase once instead of twice.
 *
 * This issue has been fixed in kernel:
 * 'b155b4fde5bd("mm: mlock: avoid increase mm->locked_vm on mlock() when already mlock2(,MLOCK_ONFAULT)")'
 */
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/mman.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/mlock2.h"

static unsigned long pgsz;
static char *addr;

static void verify_mlock2(void)
{
	unsigned long bsize, asize1, asize2;

	SAFE_FILE_LINES_SCANF("/proc/self/status", "VmLck: %lu", &bsize);

	TEST(tst_syscall(__NR_mlock2, addr, pgsz, MLOCK_ONFAULT));
	if (TST_RET != 0) {
		if (TST_ERR == EINVAL) {
			tst_res(TCONF,
				"mlock2() didn't support MLOCK_ONFAULT");
		} else {
			tst_res(TFAIL | TTERRNO,
				"mlock2(MLOCK_ONFAULT) failed");
		}
		return;
	}

	SAFE_FILE_LINES_SCANF("/proc/self/status", "VmLck: %lu", &asize1);

	if ((asize1 - bsize) * 1024 != pgsz) {
		tst_res(TFAIL,
			"mlock2(MLOCK_ONFAULT) locked %lu size, expected %lu",
			(asize1 - bsize) * 1024, pgsz);
		goto end;
	}

	TEST(tst_syscall(__NR_mlock2, addr, pgsz, 0));
	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "mlock2() failed");
		goto end;
	}

	SAFE_FILE_LINES_SCANF("/proc/self/status", "VmLck: %lu", &asize2);

	if (asize1 != asize2) {
		tst_res(TFAIL, "Locking one memory again increased VmLck");
	} else {
		tst_res(TPASS,
			"Locking one memory again didn't increased VmLck");
	}

end:
	SAFE_MUNLOCK(addr, pgsz);
}

static void setup(void)
{
	pgsz = getpagesize();
	addr = SAFE_MMAP(NULL, pgsz, PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

static void cleanup(void)
{
	if (addr)
		SAFE_MUNMAP(addr, pgsz);
}

static struct tst_test test = {
	.test_all = verify_mlock2,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.min_kver = "2.6.9",
};
