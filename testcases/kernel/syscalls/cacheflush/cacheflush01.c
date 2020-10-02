// SPDX-License-Identifier: GPL-2.0-or-later

#include "tst_test.h"
#include "lapi/syscalls.h"

#if __NR_cacheflush != __LTP__NR_INVALID_SYSCALL

#include <asm/cachectl.h>

/*
 * m68k does not have these constants
 */

#ifndef   ICACHE
# define   ICACHE   (1<<0)
#endif

#ifndef   DCACHE
# define   DCACHE   (1<<1)
#endif

#ifndef   BCACHE
# define   BCACHE   (ICACHE|DCACHE)
#endif

#define CACHE_DESC(x) .cache = x, .desc = #x

static struct test_case_t {
	int cache;
	const char *desc;
} test_cases[] = {
	{ CACHE_DESC(ICACHE) },
	{ CACHE_DESC(DCACHE) },
	{ CACHE_DESC(BCACHE) },
};

static char *addr;

static void setup(void)
{
	addr = SAFE_MALLOC(getpagesize());
}

static void test_cacheflush(unsigned int i)
{
	struct test_case_t *tc = &test_cases[i];

	TEST(tst_syscall(__NR_cacheflush, addr, getpagesize(), tc->cache));
	if (TST_RET == 0) {
		tst_res(TPASS, "%s passed", tc->desc);
	} else {
		tst_res(TFAIL | TTERRNO, "%s failed", tc->desc);
	}
}

static struct tst_test test = {
	.setup = setup,
	.test = test_cacheflush,
	.tcnt = ARRAY_SIZE(test_cases),
};

#else
	TST_TEST_TCONF("system doesn't support cacheflush()");
#endif
