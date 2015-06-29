/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Licensed under the GNU GPLv2 or later.
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
 /*
  * Block several threads on a private mutex, then wake them up.
  */

#include <errno.h>
#include <pthread.h>

#include "test.h"
#include "safe_macros.h"
#include "futextest.h"
#include "futex_utils.h"

const char *TCID="futex_wake02";
const int TST_TOTAL=11;

static futex_t futex = FUTEX_INITIALIZER;

static volatile int threads_flags[55];

static int threads_awake(void)
{
	int ret = 0;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(threads_flags); i++) {
		if (threads_flags[i])
			ret++;
	}

	return ret;
}

static void clear_threads_awake(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(threads_flags); i++)
		threads_flags[i] = 0;
}

static void *threaded(void *arg)
{
	long i = (long)arg;

	futex_wait(&futex, futex, NULL, FUTEX_PRIVATE_FLAG);

	threads_flags[i] = 1;

	return NULL;
}

static void do_child(void)
{
	int res, i, j, awake;
	pthread_t t[55];

	for (i = 0; i < (int)ARRAY_SIZE(t); i++) {
		res = pthread_create(&t[i], NULL, threaded, (void*)((long)i));
		if (res) {
			tst_brkm(TBROK, NULL, "pthread_create(): %s",
			         tst_strerrno(res));
		}
	}

	while (wait_for_threads(ARRAY_SIZE(t)))
		usleep(100);

	for (i = 1; i <= 10; i++) {
		clear_threads_awake();
		res = futex_wake(&futex, i, FUTEX_PRIVATE_FLAG);
		if (i != res) {
			tst_resm(TFAIL,
			         "futex_wake() woken up %i threads, expected %i",
			         res, i);
		}

		for (j = 0; j < 100000; j++) {
			awake = threads_awake();
			if (awake == i)
				break;

			usleep(100);
		}

		if (awake == i) {
			tst_resm(TPASS, "futex_wake() woken up %i threads", i);
		} else {
			tst_resm(TFAIL, "Woken up %i threads, expected %i",
			         awake, i);
		}
	}

	res = futex_wake(&futex, 1, FUTEX_PRIVATE_FLAG);

	if (res) {
		tst_resm(TFAIL, "futex_wake() woken up %i, none were waiting",
		         res);
	} else {
		tst_resm(TPASS, "futex_wake() woken up 0 threads");
	}

	for (i = 0; i < (int)ARRAY_SIZE(t); i++)
		pthread_join(t[i], NULL);

	tst_exit();
}

/*
 * We do the real test in a child because with the test -i parameter the loop
 * that checks that all threads are sleeping may fail with ENOENT. That is
 * because some of the threads from previous run may still be there.
 *
 * Which is because the userspace part of pthread_join() sleeps in a futex on a
 * pthread tid which is woken up at the end of the exit_mm(tsk) which is before
 * the process is removed from the parent thread_group list. So there is a
 * small race window where the readdir() returns the process tid as a directory
 * under /proc/$PID/tasks/, but the subsequent open() fails with ENOENT because
 * the thread was removed meanwhile.
 */
static void verify_futex_wake(void)
{
	int pid;

	pid = tst_fork();

	switch (pid) {
	case 0:
		do_child();
	case -1:
		tst_brkm(TBROK | TERRNO, NULL, "fork() failed");
	default:
		tst_record_childstatus(NULL, pid);
	}
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify_futex_wake();

	tst_exit();
}
