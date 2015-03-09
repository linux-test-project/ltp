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
  * Block on a futex and wait for wakeup.
  *
  * This tests uses private mutexes with threads.
  */

#include <errno.h>
#include <pthread.h>

#include "test.h"
#include "futextest.h"

const char *TCID="futex_wait03";
const int TST_TOTAL=1;

static futex_t futex = FUTEX_INITIALIZER;

static void *threaded(void *arg LTP_ATTRIBUTE_UNUSED)
{
	long ret;

	tst_process_state_wait2(getpid(), 'S');

	ret = futex_wake(&futex, 1, FUTEX_PRIVATE_FLAG);

	return (void*)ret;
}

static void verify_futex_wait(void)
{
	long ret;
	int res;
	pthread_t t;

	res = pthread_create(&t, NULL, threaded, NULL);
	if (res) {
		tst_brkm(TBROK, NULL, "pthread_create(): %s",
	                 tst_strerrno(res));
	}

	res = futex_wait(&futex, futex, NULL, FUTEX_PRIVATE_FLAG);
	if (res) {
		tst_resm(TFAIL, "futex_wait() returned %i, errno %s",
	                 res, tst_strerrno(errno));
		pthread_join(t, NULL);
		return;
	}

	res = pthread_join(t, (void*)&ret);
	if (res)
		tst_brkm(TBROK, NULL, "pthread_join(): %s", tst_strerrno(res));

	if (ret != 1)
		tst_resm(TFAIL, "futex_wake() returned %li", ret);
	else
		tst_resm(TPASS, "futex_wait() woken up");
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify_futex_wait();

	tst_exit();
}
