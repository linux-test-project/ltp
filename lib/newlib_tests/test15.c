// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 */

/*
 * A basic regression test for tst_atomic_{load,store}. Also provides a
 * limited check that atomic stores and loads order non-atomic memory
 * accesses. That is, we are checking that they implement memory fences or
 * barriers.
 *
 * Many architectures/machines will still pass the test even if you remove the
 * atomic functions. X86 in particular has strong memory ordering by default
 * so that should always pass (if you use volatile). However Aarch64
 * (Raspberry Pi 3 Model B) has been observed to fail without the atomic
 * functions.
 *
 * A failure can occur if an update to seq_n is not made globally visible by
 * the time the next thread needs to use it.
 */

#include <stdint.h>
#include <pthread.h>
#include "tst_test.h"
#include "tst_atomic.h"

#define THREADS 64
#define FILLER (1 << 20)

/* Uncomment these to see what happens without atomics. To prevent the compiler
 * from removing/reording atomic and seq_n, mark them as volatile.
 */
/* #define tst_atomic_load(v) (*(v)) */
/* #define tst_atomic_store(i, v) *(v) = (i) */

struct block {
	int seq_n;
	intptr_t id;
	intptr_t filler[FILLER];
};

static int atomic;
/* Instead of storing seq_n on the stack (probably next to the atomic variable
 * above), we store it in the middle of some anonymous mapped memory and keep
 * a pointer to it. This should decrease the probability that the value of
 * seq_n will be synchronised between processors as a byproduct of the atomic
 * variable being updated.
 */
static int *seq_n;
static struct block *m;

static void *worker_load_store(void *aid)
{
	int id = (intptr_t)aid, i;

	for (i = tst_atomic_load(&atomic);
	     i != id;
	     i = tst_atomic_load(&atomic))
		;

	(m + (*seq_n))->id = id;
	*seq_n += 1;
	tst_atomic_store(i + 1, &atomic);

	return NULL;
}

/* Attempt to stress the memory transport so that memory operations are
 * contended and less predictable. This should increase the likelyhood of a
 * failure if a memory fence is missing.
 */
static void *mem_spam(void *vp LTP_ATTRIBUTE_UNUSED)
{
	intptr_t i = 0, j;
	struct block *cur = m;

	tst_res(TINFO, "Memory spammer started");
	while (tst_atomic_load(&atomic) > 0) {
		for (j = 0; j < FILLER; j++)
			cur->filler[j] = j;

		if (i < THREADS - 1) {
			cur = m + (++i);
		} else {
			i = 0;
			cur = m;
		}
	}

	return NULL;
}

static void do_test(void)
{
	intptr_t i, id;
	pthread_t threads[THREADS + 1];

	atomic = 0;
	m = SAFE_MMAP(NULL, sizeof(*m) * THREADS,
		      PROT_READ | PROT_WRITE,
		      MAP_PRIVATE | MAP_ANONYMOUS,
		      -1, 0);
	seq_n = &((m + THREADS / 2)->seq_n);

	pthread_create(&threads[THREADS], NULL, mem_spam, NULL);
	for (i = THREADS - 1; i >= 0; i--)
		pthread_create(&threads[i], NULL, worker_load_store, (void *)i);

	for (i = 0; i < THREADS; i++) {
		tst_res(TINFO, "Joining thread %li", i);
		pthread_join(threads[i], NULL);
	}
	tst_atomic_store(-1, &atomic);
	pthread_join(threads[THREADS], NULL);

	tst_res(TINFO, "Expected\tFound");
	for (i = 0; i < THREADS; i++) {
		id = (m + i)->id;
		if (id != i)
			tst_res(TFAIL, "%d\t\t%d", (int)i, (int)id);
		else
			tst_res(TPASS, "%d\t\t%d", (int)i, (int)id);
	}

	SAFE_MUNMAP(m, sizeof(*m) * THREADS);
}

static struct tst_test test = {
	.test_all = do_test,
};
