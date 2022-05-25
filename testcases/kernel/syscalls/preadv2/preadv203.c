// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * This is a basic functional test for RWF_NOWAIT flag, we are attempting to
 * force preadv2() either to return a short read or EAGAIN with three
 * concurently running threads:
 *
 *  nowait_reader: reads from a random offset from a random file with
 *                 RWF_NOWAIT flag and expects to get EAGAIN and short
 *                 read sooner or later
 *
 *  writer_thread: rewrites random file in order to keep the underlying device
 *                 busy so that pages evicted from cache cannot be faulted
 *                 immediately
 *
 *  cache_dropper: attempts to evict pages from a cache in order for reader to
 *                 hit evicted page sooner or later
 */

/*
 * If test fails with EOPNOTSUPP you have likely hit a glibc bug:
 *
 * https://sourceware.org/bugzilla/show_bug.cgi?id=23579
 *
 * Which can be worked around by calling preadv2() directly by syscall() such as:
 *
 * static ssize_t sys_preadv2(int fd, const struct iovec *iov, int iovcnt,
 *                            off_t offset, int flags)
 * {
 *	return syscall(SYS_preadv2, fd, iov, iovcnt, offset, offset>>32, flags);
 * }
 *
 */

#define _GNU_SOURCE
#include <string.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#include "tst_test.h"
#include "tst_safe_pthread.h"
#include "lapi/preadv2.h"

#define CHUNK_SZ 4123
#define CHUNKS 60
#define MNTPOINT "mntpoint"
#define FILES 500

static int fds[FILES];

static volatile int stop;

static void drop_caches(void)
{
	SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "3");
}

/*
 * All files are divided in chunks each filled with the same bytes starting with
 * '0' at offset 0 and with increasing value on each next chunk.
 *
 * 000....000111....111.......AAA......AAA...
 * | chunk0 || chunk1 |  ...  |  chunk17 |
 */
static int verify_short_read(struct iovec *iov, size_t iov_cnt,
		             off_t off, size_t size)
{
	unsigned int i;
	size_t j, checked = 0;

	for (i = 0; i < iov_cnt; i++) {
		char *buf = iov[i].iov_base;
		for (j = 0; j < iov[i].iov_len; j++) {
			char exp_val = '0' + (off + checked)/CHUNK_SZ;

			if (exp_val != buf[j]) {
				tst_res(TFAIL,
				        "Wrong value read pos %zu size %zu %c (%i) %c (%i)!",
				        checked, size, exp_val, exp_val,
					isprint(buf[j]) ? buf[j] : ' ', buf[j]);
				return 1;
			}

			if (++checked >= size)
				return 0;
		}
	}

	return 0;
}

static void *nowait_reader(void *unused LTP_ATTRIBUTE_UNUSED)
{
	char buf1[CHUNK_SZ/2];
	char buf2[CHUNK_SZ];
	unsigned int full_read_cnt = 0, eagain_cnt = 0;
	unsigned int short_read_cnt = 0, zero_read_cnt = 0;

	struct iovec rd_iovec[] = {
		{buf1, sizeof(buf1)},
		{buf2, sizeof(buf2)},
	};

	while (!stop) {
		if (eagain_cnt >= 100 && short_read_cnt >= 10)
			stop = 1;

		/* Ensure short reads doesn't happen because of tripping on EOF */
		off_t off = random() % ((CHUNKS - 2) * CHUNK_SZ);
		int fd = fds[random() % FILES];

		TEST(preadv2(fd, rd_iovec, 2, off, RWF_NOWAIT));

		if (TST_RET < 0) {
			if (TST_ERR != EAGAIN)
				tst_brk(TBROK | TTERRNO, "preadv2() failed");

			eagain_cnt++;
			continue;
		}


		if (TST_RET == 0) {
			zero_read_cnt++;
			continue;
		}

		if (TST_RET != CHUNK_SZ + CHUNK_SZ/2) {
			verify_short_read(rd_iovec, 2, off, TST_RET);
			short_read_cnt++;
			continue;
		}

		full_read_cnt++;
	}

	tst_res(TINFO,
	        "Number of full_reads %u, short reads %u, zero len reads %u, EAGAIN(s) %u",
		full_read_cnt, short_read_cnt, zero_read_cnt, eagain_cnt);

	return (void*)(long)eagain_cnt;
}

static void *writer_thread(void *unused)
{
	char buf[CHUNK_SZ];
	unsigned int j, write_cnt = 0;

	struct iovec wr_iovec[] = {
		{buf, sizeof(buf)},
	};

	while (!stop) {
		int fd = fds[random() % FILES];

		for (j = 0; j < CHUNKS; j++) {
			memset(buf, '0' + j, sizeof(buf));

			off_t off = CHUNK_SZ * j;

			if (pwritev(fd, wr_iovec, 1, off) < 0) {
				if (errno == EBADF) {
					tst_res(TINFO | TERRNO, "FDs closed, exiting...");
					return unused;
				}

				tst_brk(TBROK | TERRNO, "pwritev()");
			}

			write_cnt++;
		}
	}

	tst_res(TINFO, "Number of writes %u", write_cnt);

	return unused;
}

static void *cache_dropper(void *unused)
{
	unsigned int drop_cnt = 0;

	while (!stop) {
		drop_caches();
		drop_cnt++;
	}

	tst_res(TINFO, "Cache dropped %u times", drop_cnt);

	return unused;
}

static void verify_preadv2(void)
{
	pthread_t reader, dropper, writer;
	void *eagains;

	stop = 0;

	drop_caches();

	SAFE_PTHREAD_CREATE(&dropper, NULL, cache_dropper, NULL);
	SAFE_PTHREAD_CREATE(&reader, NULL, nowait_reader, NULL);
	SAFE_PTHREAD_CREATE(&writer, NULL, writer_thread, NULL);

	while (!stop && tst_remaining_runtime())
		usleep(100000);

	stop = 1;

	SAFE_PTHREAD_JOIN(reader, &eagains);
	SAFE_PTHREAD_JOIN(dropper, NULL);
	SAFE_PTHREAD_JOIN(writer, NULL);

	if (eagains)
		tst_res(TPASS, "Got some EAGAIN");
	else
		tst_res(TFAIL, "Haven't got EAGAIN");
}

static void check_preadv2_nowait(int fd)
{
	char buf[1];
	struct iovec iovec[] = {
		{buf, sizeof(buf)},
	};

	TEST(preadv2(fd, iovec, 1, 0, RWF_NOWAIT));

	if (TST_ERR == EOPNOTSUPP)
		tst_brk(TCONF | TTERRNO, "preadv2()");
}

static void setup(void)
{
	char path[1024];
	char buf[CHUNK_SZ];
	unsigned int i;
	char j;

	for (i = 0; i < FILES; i++) {
		snprintf(path, sizeof(path), MNTPOINT"/file_%i", i);

		fds[i] = SAFE_OPEN(path, O_RDWR | O_CREAT, 0644);

		if (i == 0)
			check_preadv2_nowait(fds[i]);

		for (j = 0; j < CHUNKS; j++) {
			memset(buf, '0' + j, sizeof(buf));
			SAFE_WRITE(1, fds[i], buf, sizeof(buf));
		}
	}
}

static void do_cleanup(void)
{
	unsigned int i;

	for (i = 0; i < FILES; i++) {
		if (fds[i] > 0)
			SAFE_CLOSE(fds[i]);
	}
}

TST_DECLARE_ONCE_FN(cleanup, do_cleanup);

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_preadv2,
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.max_runtime = 60,
	.needs_root = 1,
};
