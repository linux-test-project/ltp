// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Xiao Yang <ice_yangxiao@163.com>
 *
 * Description:
 * Check various errnos for futex(FUTEX_CMP_REQUEUE).
 * 1) futex(FUTEX_CMP_REQUEUE) with invalid val returns EINVAL.
 * 2) futex(FUTEX_CMP_REQUEUE) with invalid val2 returns EINVAL.
 * 3) futex(FUTEX_CMP_REQUEUE) with mismatched val3 returns EAGAIN.
 *
 * It's also a regression test for CVE-2018-6927:
 * fbe0e839d1e2 ("futex: Prevent overflow by strengthen input validation")
 */

#include <errno.h>
#include <linux/futex.h>
#include <sys/time.h>

#include "tst_test.h"
#include "futextest.h"

static futex_t *futexes;

static struct tcase {
	int set_wakes;
	int set_requeues;
	int exp_val;
	int exp_errno;
} tcases[] = {
	{1, -1, FUTEX_INITIALIZER, EINVAL},
	{-1, 1, FUTEX_INITIALIZER, EINVAL},
	{1, 1, FUTEX_INITIALIZER + 1, EAGAIN},
};

static void verify_futex_cmp_requeue(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(futex_cmp_requeue(&futexes[0], tc->exp_val, &futexes[1],
	     tc->set_wakes, tc->set_requeues, 0));
	if (TST_RET != -1) {
		tst_res(TFAIL, "futex_cmp_requeue() succeeded unexpectedly");
		return;
	}

	if (TST_ERR != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"futex_cmp_requeue() failed unexpectedly, expected %s",
			tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "futex_cmp_requeue() failed as expected");
}

static void setup(void)
{
	futexes = SAFE_MMAP(NULL, sizeof(futex_t) * 2, PROT_READ | PROT_WRITE,
			    MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	futexes[0] = FUTEX_INITIALIZER;
	futexes[1] = FUTEX_INITIALIZER + 1;
}

static void cleanup(void)
{
	if (futexes)
		SAFE_MUNMAP((void *)futexes, sizeof(futex_t) * 2);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_futex_cmp_requeue,
	.tcnt = ARRAY_SIZE(tcases),
	.tags = (const struct tst_tag[]) {
		{"CVE", "2018-6927"},
		{"linux-git", "fbe0e839d1e2"},
		{}
	}
};
