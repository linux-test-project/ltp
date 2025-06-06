// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Check for potential issues in writev() if the first iovec entry is NULL
 * and the next one is not present in RAM. This can result in a brief window
 * where writev() first writes uninitialized data into the file (possibly
 * exposing internal kernel structures) and then overwrites it with the real
 * iovec contents later.
 */

/*
 * Bugs fixed in:
 *  commit d4690f1e1cdabb4d61207b6787b1605a0dc0aeab
 *  Author: Al Viro <viro@ZenIV.linux.org.uk>
 *  Date:   Fri Sep 16 00:11:45 2016 +0100
 *
 *  fix iov_iter_fault_in_readable()
 */

#include <sys/uio.h>
#include "tst_test.h"
#include "tst_atomic.h"
#include "tst_fuzzy_sync.h"

#define CHUNK_SIZE 256
#define BUF_SIZE (2 * CHUNK_SIZE)
#define MNTPOINT "mntpoint"
#define TEMPFILE MNTPOINT "/test_file"
#define MAPFILE MNTPOINT "/map_file"

static unsigned char buf[BUF_SIZE], *map_ptr;
static int mapfd = -1, writefd = -1, readfd = -1;
static tst_atomic_t written;
static struct tst_fzsync_pair fzsync_pair;
struct iovec iov[5];

static void setup(void)
{
	int i;

	for (i = 0; i < BUF_SIZE; i++)
		buf[i] = i & 0xff;

	mapfd = SAFE_OPEN(MAPFILE, O_CREAT|O_RDWR|O_TRUNC, 0644);
	SAFE_WRITE(SAFE_WRITE_ALL, mapfd, buf, BUF_SIZE);

	tst_fzsync_pair_init(&fzsync_pair);
}

static void *thread_run(void *arg)
{
	while (tst_fzsync_run_b(&fzsync_pair)) {
		writefd = SAFE_OPEN(TEMPFILE, O_CREAT|O_WRONLY|O_TRUNC, 0644);
		written = BUF_SIZE;
		tst_fzsync_wait_b(&fzsync_pair);

		/*
		 * Do *NOT* preload the data using MAP_POPULATE or touching
		 * the mapped range. We're testing whether writev() handles
		 * fault-in correctly.
		 */
		map_ptr = SAFE_MMAP(NULL, BUF_SIZE, PROT_READ, MAP_SHARED,
			mapfd, 0);
		iov[1].iov_base = map_ptr;
		iov[1].iov_len = CHUNK_SIZE;
		iov[3].iov_base = map_ptr + CHUNK_SIZE;
		iov[3].iov_len = CHUNK_SIZE;

		tst_fzsync_start_race_b(&fzsync_pair);
		tst_atomic_store(writev(writefd, iov, ARRAY_SIZE(iov)),
			&written);
		tst_fzsync_end_race_b(&fzsync_pair);

		SAFE_MUNMAP(map_ptr, BUF_SIZE);
		map_ptr = NULL;
		SAFE_CLOSE(writefd);
	}

	return arg;
}

static void run(void)
{
	int total_read;
	unsigned char readbuf[BUF_SIZE + 1];

	tst_fzsync_pair_reset(&fzsync_pair, thread_run);

	while (tst_fzsync_run_a(&fzsync_pair)) {
		tst_fzsync_wait_a(&fzsync_pair);
		readfd = SAFE_OPEN(TEMPFILE, O_RDONLY);
		tst_fzsync_start_race_a(&fzsync_pair);

		for (total_read = 0; total_read < tst_atomic_load(&written);) {
			total_read += SAFE_READ(0, readfd, readbuf+total_read,
				BUF_SIZE + 1 - total_read);
		}

		tst_fzsync_end_race_a(&fzsync_pair);
		SAFE_CLOSE(readfd);

		if (total_read > BUF_SIZE)
			tst_brk(TBROK, "writev() wrote too much data");

		if (total_read <= 0)
			continue;

		if (memcmp(readbuf, buf, total_read)) {
			tst_res(TFAIL, "writev() wrote invalid data");
			return;
		}
	}

	tst_res(TPASS, "writev() handles page fault-in correctly");
}

static void cleanup(void)
{
	if (map_ptr && map_ptr != MAP_FAILED)
		SAFE_MUNMAP(map_ptr, BUF_SIZE);

	if (mapfd >= 0)
		SAFE_CLOSE(mapfd);

	if (readfd >= 0)
		SAFE_CLOSE(readfd);

	if (writefd >= 0)
		SAFE_CLOSE(writefd);

	tst_fzsync_pair_cleanup(&fzsync_pair);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.min_cpus = 2,
	.setup = setup,
	.cleanup = cleanup,
	.runtime = 75,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "d4690f1e1cda"},
		{}
	}
};
