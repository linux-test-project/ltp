// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 *
 * Test Description:
 * This case is designed to test whether pipe can wakeup all readers
 * when last writer closes.
 *
 * This is also a regression test for commit 6551d5c56eb0
 * ("pipe: make sure to wake up everybody when the last reader/writer closes").
 * This bug was introduced by commit 0ddad21d3e99 ("pipe: use exclusive
 * waits when reading or writing").
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "tst_test.h"

static unsigned int tcases[] = {
	2,
	10,
	27,
	100
};

static int fds[2];

static void do_child(unsigned int i)
{
	char buf;

	SAFE_CLOSE(fds[1]);
	TST_CHECKPOINT_WAKE(i);
	int ret = SAFE_READ(0, fds[0], &buf, 1);
	if (ret != 0)
		tst_res(TFAIL, "Wrong return from read %i", ret);
	exit(0);
}

static void verify_pipe(unsigned int n)
{
	int ret;
	unsigned int i, cnt = 0, sleep_us = 1, fail = 0;
	unsigned int child_num = tcases[n];
	int pid[child_num];

	SAFE_PIPE(fds);
	tst_res(TINFO, "Creating %d child processes", child_num);

	for (i = 0; i < child_num; i++) {
		pid[i] = SAFE_FORK();
		if (pid[i] == 0)
			do_child(i);
		TST_CHECKPOINT_WAIT(i);
		TST_PROCESS_STATE_WAIT(pid[i], 'S', 0);
	}

	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);

	while (cnt < child_num && sleep_us < 100000) {
		ret = waitpid(-1, NULL, WNOHANG);
		if (ret < 0)
			tst_brk(TBROK | TERRNO, "waitpid()");
		if (ret > 0) {
			cnt++;
			for (i = 0; i < child_num; i++) {
				if (pid[i] == ret)
					pid[i] = 0;
			}
			continue;
		}
		usleep(sleep_us);
		sleep_us *= 2;
	}

	for (i = 0; i < child_num; i++) {
		if (pid[i]) {
			tst_res(TINFO, "pid %i still sleeps", pid[i]);
			fail = 1;
			SAFE_KILL(pid[i], SIGKILL);
			SAFE_WAIT(NULL);
		}
	}

	if (fail)
		tst_res(TFAIL, "Closed pipe didn't wake up everyone");
	else
		tst_res(TPASS, "Closed pipe waked up everyone");
}

static struct tst_test test = {
	.test = verify_pipe,
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.needs_checkpoints = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "6551d5c56eb"},
		{}
	}
};
