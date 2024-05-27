// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Red Hat Inc. All Rights Reserved.
 * Author: Xiong Zhou <xzhou@redhat.com>
 *
 * This is testing OFD locks racing with POSIX locks:
 *
 *	OFD read  lock   vs   OFD   write lock
 *	OFD read  lock   vs   POSIX write lock
 *	OFD write lock   vs   POSIX write lock
 *	OFD write lock   vs   POSIX read  lock
 *	OFD write lock   vs   OFD   write lock
 *
 *	OFD   r/w locks vs POSIX write locks
 *	OFD   r/w locks vs POSIX read locks
 *
 * For example:
 *
 * Init an file with preset values.
 *
 * Threads acquire OFD READ  locks to read  a 4k section start from 0;
 * checking data read back, there should not be any surprise
 * values and data should be consistent in a 1k block.
 *
 * Threads acquire OFD WRITE locks to write a 4k section start from 1k,
 * writing different values in different threads.
 *
 * Check file data after racing, there should not be any surprise values
 * and data should be consistent in a 1k block.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>

#include "lapi/fcntl.h"
#include "tst_safe_pthread.h"
#include "tst_test.h"
#include "fcntl_common.h"

static int thread_cnt;
static int fail_flag = 0;
static volatile int loop_flag = 1;
static const int max_thread_cnt = 32;
static const char fname[] = "tst_ofd_posix_locks";
static const long write_size = 4096;
static pthread_barrier_t barrier;

struct param {
	long offset;
	long length;
	long cnt;
};

static void setup(void)
{
	thread_cnt = tst_ncpus_conf() * 3;
	if (thread_cnt > max_thread_cnt)
		thread_cnt = max_thread_cnt;
}

/* OFD write lock writing data*/
static void *fn_ofd_w(void *arg)
{
	struct param *pa = arg;
	unsigned char buf[pa->length];
	int fd = SAFE_OPEN(fname, O_RDWR);
	long wt = pa->cnt;

	struct flock lck = {
		.l_whence = SEEK_SET,
		.l_start  = pa->offset,
		.l_len    = pa->length,
		.l_pid    = 0,
	};

	do {

		memset(buf, wt, pa->length);

		lck.l_type = F_WRLCK;
		FCNTL_COMPAT(fd, F_OFD_SETLKW, &lck);

		SAFE_LSEEK(fd, pa->offset, SEEK_SET);
		SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, pa->length);

		lck.l_type = F_UNLCK;
		FCNTL_COMPAT(fd, F_OFD_SETLKW, &lck);

		wt++;
		if (wt >= 255)
			wt = pa->cnt;

		sched_yield();
	} while (loop_flag);

	pthread_barrier_wait(&barrier);
	SAFE_CLOSE(fd);
	return NULL;
}

/* POSIX write lock writing data*/
static void *fn_posix_w(void *arg)
{
	struct param *pa = arg;
	unsigned char buf[pa->length];
	int fd = SAFE_OPEN(fname, O_RDWR);
	long wt = pa->cnt;

	struct flock lck = {
		.l_whence = SEEK_SET,
		.l_start  = pa->offset,
		.l_len    = pa->length,
	};

	do {

		memset(buf, wt, pa->length);

		lck.l_type = F_WRLCK;
		SAFE_FCNTL(fd, F_SETLKW, &lck);

		SAFE_LSEEK(fd, pa->offset, SEEK_SET);
		SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, pa->length);

		lck.l_type = F_UNLCK;
		SAFE_FCNTL(fd, F_SETLKW, &lck);

		wt++;
		if (wt >= 255)
			wt = pa->cnt;

		sched_yield();
	} while (loop_flag);

	pthread_barrier_wait(&barrier);
	SAFE_CLOSE(fd);
	return NULL;
}

/* OFD read lock reading data*/
static void *fn_ofd_r(void *arg)
{
	struct param *pa = arg;
	unsigned char buf[pa->length];
	int i;
	int fd = SAFE_OPEN(fname, O_RDWR);

	struct flock lck = {
		.l_whence = SEEK_SET,
		.l_start  = pa->offset,
		.l_len    = pa->length,
		.l_pid    = 0,
	};

	while (loop_flag) {

		memset(buf, 0, pa->length);

		lck.l_type = F_RDLCK;
		FCNTL_COMPAT(fd, F_OFD_SETLKW, &lck);

		/* rlock acquired */
		SAFE_LSEEK(fd, pa->offset, SEEK_SET);
		SAFE_READ(1, fd, buf, pa->length);

		/* Verifying data read */
		for (i = 0; i < pa->length; i++) {

			if (buf[i] < 1 || buf[i] > 254) {

				tst_res(TFAIL, "Unexpected data "
					"offset %ld value %d",
					pa->offset + i, buf[i]);
				fail_flag = 1;
				break;
			}

			int j = (i / (pa->length/4)) * pa->length/4;

			if (buf[i] != buf[j]) {

				tst_res(TFAIL, "Unexpected data "
					"offset %ld value %d",
					pa->offset + i, buf[i]);
				fail_flag = 1;
				break;
			}
		}

		lck.l_type = F_UNLCK;
		FCNTL_COMPAT(fd, F_OFD_SETLK, &lck);

		sched_yield();
	}

	pthread_barrier_wait(&barrier);
	SAFE_CLOSE(fd);
	return NULL;
}

/* POSIX read lock reading data */
static void *fn_posix_r(void *arg)
{
	struct param *pa = arg;
	unsigned char buf[pa->length];
	int i;
	int fd = SAFE_OPEN(fname, O_RDWR);

	struct flock lck = {
		.l_whence = SEEK_SET,
		.l_start  = pa->offset,
		.l_len    = pa->length,
	};

	while (loop_flag) {

		memset(buf, 0, pa->length);

		lck.l_type = F_RDLCK;
		SAFE_FCNTL(fd, F_SETLKW, &lck);

		/* rlock acquired */
		SAFE_LSEEK(fd, pa->offset, SEEK_SET);
		SAFE_READ(1, fd, buf, pa->length);

		/* Verifying data read */
		for (i = 0; i < pa->length; i++) {

			if (buf[i] < 1 || buf[i] > 254) {

				tst_res(TFAIL, "Unexpected data "
					"offset %ld value %d",
					pa->offset + i, buf[i]);
				fail_flag = 1;
				break;
			}

			int j = (i / (pa->length/4)) * pa->length/4;

			if (buf[i] != buf[j]) {

				tst_res(TFAIL, "Unexpected data "
					"offset %ld value %d",
					pa->offset + i, buf[i]);
				fail_flag = 1;
				break;
			}
		}

		lck.l_type = F_UNLCK;
		SAFE_FCNTL(fd, F_SETLK, &lck);

		sched_yield();
	}

	pthread_barrier_wait(&barrier);
	SAFE_CLOSE(fd);
	return NULL;
}

static void *fn_dummy(void *arg)
{
	arg = NULL;

	pthread_barrier_wait(&barrier);
	return arg;
}

/* Test different functions and verify data */
static void test_fn(void *f0(void *), void *f1(void *),
		    void *f2(void *), const char *msg)
{
	int i, k, fd;
	pthread_t id0[thread_cnt];
	pthread_t id1[thread_cnt];
	pthread_t id2[thread_cnt];
	struct param p0[thread_cnt];
	struct param p1[thread_cnt];
	struct param p2[thread_cnt];
	unsigned char buf[write_size];

	tst_res(TINFO, "%s", msg);

	if (tst_fill_file(fname, 1, write_size, thread_cnt + 1))
		tst_brk(TBROK, "Failed to create tst file");

	if (pthread_barrier_init(&barrier, NULL, thread_cnt*3) != 0)
		tst_brk(TBROK, "Failed to init pthread barrier");

	for (i = 0; i < thread_cnt; i++) {

		p0[i].offset = i * write_size;
		p0[i].length = write_size;
		p0[i].cnt = i + 2;

		p1[i].offset = i * write_size + write_size / 4;
		p1[i].length = write_size;
		p1[i].cnt = i + 2;

		p2[i].offset = i * write_size + write_size / 2;
		p2[i].length = write_size;
		p2[i].cnt = i + 2;
	}

	fail_flag = 0;
	loop_flag = 1;

	for (i = 0; i < thread_cnt; i++) {

		SAFE_PTHREAD_CREATE(id0 + i, NULL, f0, (void *)&p0[i]);
		SAFE_PTHREAD_CREATE(id1 + i, NULL, f1, (void *)&p1[i]);
		SAFE_PTHREAD_CREATE(id2 + i, NULL, f2, (void *)&p2[i]);
	}

	sleep(1);
	loop_flag = 0;

	for (i = 0; i < thread_cnt; i++) {

		SAFE_PTHREAD_JOIN(id0[i], NULL);
		SAFE_PTHREAD_JOIN(id1[i], NULL);
		SAFE_PTHREAD_JOIN(id2[i], NULL);
	}

	fd = SAFE_OPEN(fname, O_RDONLY);

	for (i = 0; i < thread_cnt * 4; i++) {

		SAFE_READ(1, fd, buf, write_size/4);

		for (k = 0; k < write_size/4; k++) {

			if (buf[k] < 2 || buf[k] > 254) {

				if (i < 3 && buf[k] == 1)
					continue;
				tst_res(TFAIL, "Unexpected data "
					"offset %ld value %d",
					i * write_size / 4 + k, buf[k]);
				SAFE_CLOSE(fd);
				return;
			}
		}

		for (k = 1; k < write_size/4; k++) {

			if (buf[k] != buf[0]) {
				tst_res(TFAIL, "Unexpected block read");
				SAFE_CLOSE(fd);
				return;
			}
		}
	}

	if (pthread_barrier_destroy(&barrier) != 0)
		tst_brk(TBROK, "Failed to destroy pthread barrier");

	SAFE_CLOSE(fd);
	if (fail_flag == 0)
		tst_res(TPASS, "Access between threads synchronized");
}

static struct tcase {
	void *(*fn0)(void *);
	void *(*fn1)(void *);
	void *(*fn2)(void *);
	const char *desc;
} tcases[] = {
	{fn_ofd_r, fn_ofd_w, fn_dummy, "OFD read lock vs OFD write lock"},
	{fn_ofd_w, fn_posix_w, fn_dummy, "OFD write lock vs POSIX write lock"},
	{fn_ofd_r, fn_posix_w, fn_dummy, "OFD read lock vs POSIX write lock"},
	{fn_ofd_w, fn_posix_r, fn_dummy, "OFD write lock vs POSIX read lock"},
	{fn_ofd_w, fn_ofd_w, fn_dummy, "OFD write lock vs OFD write lock"},
	{fn_ofd_r, fn_ofd_w, fn_posix_w, "OFD r/w lock vs POSIX write lock"},
	{fn_ofd_r, fn_ofd_w, fn_posix_r, "OFD r/w lock vs POSIX read lock"},
};

static void tests(unsigned int i)
{
	test_fn(tcases[i].fn0, tcases[i].fn1, tcases[i].fn2, tcases[i].desc);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test = tests,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup
};
