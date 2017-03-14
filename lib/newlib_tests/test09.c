/*
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
