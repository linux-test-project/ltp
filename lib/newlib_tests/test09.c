// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 */

/*
 * Test that tst_atomic_inc works as expected.
 */

#include <pthread.h>
#include "tst_test.h"

#define THREADS 64
#define ITERATIONS 100000

static int atomic;

static void *worker(void *id)
{
	int i;

	(void) id;
	for (i = 0; i < ITERATIONS; i++)
		tst_atomic_inc(&atomic);

	return NULL;
}

static void do_test(void)
{
	long i;
	pthread_t threads[THREADS];

	for (i = 0; i < THREADS; i++)
		pthread_create(threads+i, NULL, worker, (void *)i);

	for (i = 0; i < THREADS; i++) {
		tst_res(TINFO, "Joining thread %li", i);
		pthread_join(threads[i], NULL);
	}

	if (atomic == THREADS * ITERATIONS)
		tst_res(TPASS, "Atomic working as expected");
	else
		tst_res(TFAIL, "Atomic does not have expected value");
}

static struct tst_test test = {
	.test_all = do_test,
};
