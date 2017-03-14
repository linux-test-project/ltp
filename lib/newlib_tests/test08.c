/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
