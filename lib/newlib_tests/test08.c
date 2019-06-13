// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test for callback thread safety.
 */
#include <pthread.h>
#include "tst_test.h"

#define THREADS 10

static pthread_barrier_t barrier;

static void setup(void)
{
	pthread_barrier_init(&barrier, NULL, THREADS);

	tst_res(TINFO, "setup() executed");
}

static void cleanup(void)
{
	static int flag;

	/* Avoid subsequent threads to enter the cleanup */
	if (tst_atomic_inc(&flag) != 1)
		pthread_exit(NULL);

	tst_res(TINFO, "cleanup() started");
	usleep(10000);
	tst_res(TINFO, "cleanup() finished");
}

static void *worker(void *id)
{
	tst_res(TINFO, "Thread %ld waiting...", (long)id);
	pthread_barrier_wait(&barrier);
	tst_brk(TBROK, "Failure %ld", (long)id);

	return NULL;
}

static void do_test(void)
{
	long i;
	pthread_t threads[THREADS];

	for (i = 0; i < THREADS; i++)
		pthread_create(threads+i, NULL, worker, (void*)i);

	for (i = 0; i < THREADS; i++) {
		tst_res(TINFO, "Joining thread %li", i);
		pthread_join(threads[i], NULL);
	}
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
};
