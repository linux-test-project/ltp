// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

#include "lapi/fcntl.h"
#include "tst_safe_pthread.h"
#include "tst_test.h"
#include "fcntl_common.h"

static int thread_cnt;
static const int max_thread_cnt = 32;
static const char fname[] = "tst_ofd_locks";
const int writes_num = 100;
const int write_size = 4096;

static void setup(void)
{
	thread_cnt = tst_ncpus_conf() * 3;
	if (thread_cnt > max_thread_cnt)
		thread_cnt = max_thread_cnt;
}

static void spawn_threads(pthread_t *id, void *(*thread_fn)(void *))
{
	intptr_t i;

	tst_res(TINFO, "spawning '%d' threads", thread_cnt);
	for (i = 0; i < thread_cnt; ++i)
		SAFE_PTHREAD_CREATE(id + i, NULL, thread_fn, (void *)i);
}

static void wait_threads(pthread_t *id)
{
	int i;

	tst_res(TINFO, "waiting for '%d' threads", thread_cnt);
	for (i = 0; i < thread_cnt; ++i)
		SAFE_PTHREAD_JOIN(id[i], NULL);
}

void *thread_fn_01(void *arg)
{
	int i;
	unsigned char buf[write_size];
	int fd = SAFE_OPEN(fname, O_RDWR);

	memset(buf, (intptr_t)arg, write_size);

	struct flock64 lck = {
		.l_whence = SEEK_SET,
		.l_start  = 0,
		.l_len    = 1,
	};

	for (i = 0; i < writes_num; ++i) {
		lck.l_type = F_WRLCK;
		my_fcntl(fd, F_OFD_SETLKW, &lck);

		SAFE_LSEEK(fd, 0, SEEK_END);
		SAFE_WRITE(1, fd, buf, write_size);

		lck.l_type = F_UNLCK;
		my_fcntl(fd, F_OFD_SETLKW, &lck);

		sched_yield();
	}

	SAFE_CLOSE(fd);

	return NULL;
}

static void test01(void)
{
	intptr_t i;
	int k;
	pthread_t id[thread_cnt];
	int res[thread_cnt];
	unsigned char buf[write_size];

	tst_res(TINFO, "write to a file inside threads with OFD locks");

	int fd = SAFE_OPEN(fname, O_CREAT | O_TRUNC | O_RDWR, 0600);

	memset(res, 0, sizeof(res));

	spawn_threads(id, thread_fn_01);
	wait_threads(id);

	tst_res(TINFO, "verifying file's data");
	SAFE_LSEEK(fd, 0, SEEK_SET);
	for (i = 0; i < writes_num * thread_cnt; ++i) {
		SAFE_READ(1, fd, buf, write_size);

		if (buf[0] >= thread_cnt) {
			tst_res(TFAIL, "unexpected data read");
			return;
		}

		++res[buf[0]];

		for (k = 1; k < write_size; ++k) {
			if (buf[0] != buf[k]) {
				tst_res(TFAIL, "unexpected data read");
				return;
			}
		}
	}

	for (i = 0; i < thread_cnt; ++i) {
		if (res[i] != writes_num) {
			tst_res(TFAIL, "corrupted data found");
			return;
		}
	}
	SAFE_CLOSE(fd);

	tst_res(TPASS, "OFD locks synchronized access between threads");
}

static struct tst_test test = {
	.min_kver = "3.15.0",
	.needs_tmpdir = 1,
	.test_all = test01,
	.setup = setup
};
