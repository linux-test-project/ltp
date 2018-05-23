/*
 * Copyright (c) 2018 SUSE Linux.  All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Started by Jan Kara <jack@suse.cz>
 * Chnaged to use fzsync library by Cyril Hrubis <chrubis@suse.cz>
 *
 * DESCRIPTION
 * Test for inotify mark connector destruction race.
 *
 * Kernels prior to 4.17 have a race when the last fsnotify mark on the inode
 * is being deleted while another process reports event happening on that
 * inode. When the race is hit, the kernel crashes or loops.
 *
 * The problem has been fixed by commit:
 * d90a10e2444b "fsnotify: Fix fsnotify_mark_connector race"
 */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "tst_test.h"
#include "tst_safe_pthread.h"
#include "tst_fuzzy_sync.h"
#include "inotify.h"

#define FNAME "stress_fname"
#define TEARDOWNS 200000

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

static struct tst_fzsync_pair fzsync_pair = TST_FZSYNC_PAIR_INIT;
static pthread_t pt_write_seek;
static int fd;

static void *write_seek(void *unused)
{
	char buf[64];

	while (tst_fzsync_wait_update_b(&fzsync_pair)) {
		tst_fzsync_delay_b(&fzsync_pair);
		SAFE_WRITE(0, fd, buf, sizeof(buf));
		tst_fzsync_time_b(&fzsync_pair);
		SAFE_LSEEK(fd, 0, SEEK_SET);
		if (!tst_fzsync_wait_b(&fzsync_pair))
			break;
	}
	return unused;
}

static void setup(void)
{
	fd = SAFE_OPEN(FNAME, O_CREAT | O_RDWR, 0600);
	fzsync_pair.info_gap = 0x7fff;
	SAFE_PTHREAD_CREATE(&pt_write_seek, 0, write_seek, NULL);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (pt_write_seek) {
		tst_fzsync_pair_exit(&fzsync_pair);
		SAFE_PTHREAD_JOIN(pt_write_seek, NULL);
	}
}

static void verify_inotify(void)
{
	int inotify_fd;
	int wd;
	int tests;

	inotify_fd = myinotify_init1(0);
	if (inotify_fd < 0)
		tst_brk(TBROK | TERRNO, "inotify_init failed");
	for (tests = 0; tests < TEARDOWNS; tests++) {
		wd = myinotify_add_watch(inotify_fd, FNAME, IN_MODIFY);
		if (wd < 0)
			tst_brk(TBROK | TERRNO, "inotify_add_watch() failed.");

		tst_fzsync_wait_update_a(&fzsync_pair);
		tst_fzsync_delay_a(&fzsync_pair);

		wd = myinotify_rm_watch(inotify_fd, wd);
		if (wd < 0)
			tst_brk(TBROK | TERRNO, "inotify_rm_watch() failed.");

		tst_fzsync_time_a(&fzsync_pair);
		tst_fzsync_wait_a(&fzsync_pair);
	}
	SAFE_CLOSE(inotify_fd);
	/* We survived for given time - test succeeded */
	tst_res(TPASS, "kernel survived inotify beating");
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_inotify,
};

#else
	TST_TEST_TCONF("system doesn't have required inotify support");
#endif
